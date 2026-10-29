#include "pch.h"
using namespace CrsKit::Math;
#include "ICoordinateTransformationAuthorityFactory.h"
#include "CoordinateTransformationAuthorityFactory.h"
#include "CoordinateTransformation.h"
#include "../CoordinateSystems/BaseInfo.h"
#include "../CoordinateSystems/CoordinateSystem.h"
#include "CoordinateTransformationFactory.h"
#include "ConcatenatedTransform.h"
#include "Passthrough.h"
#include "ThreeCoordinateHeightTransform.h"
#include "Algorithms/Afine.h"
#include "Algorithms/Geographic2D2Geocentric.h"
#include "Algorithms/Geographic2D2Geographic3D.h"
#include "Algorithms/LongitudeRotation.h"
#include "Algorithms/PositionVectorTransformation.h"
#include "../CoordinateSystems/HorizontalDatum.h"
#include "../ExtensionManager.h"

#include <array>
#include <optional>
#include <utility>

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;
using namespace Algorithms;
using namespace std;

namespace
{
	// Directions of the two values the projection formula yields: (easting, northing) for most methods.
	auto FormulaAxes(std::string const& className) -> std::pair<AxisOrientationEnum, AxisOrientationEnum>
	{
		if (0 == CrsKit::compareNoCase(className.c_str(), "Transverse_Mercator_South_Orientated") || 0 == CrsKit::compareNoCase(className.c_str(), "Bonne_South_Orientated"))
			return { AxisOrientationEnum::West, AxisOrientationEnum::South };
		if (0 == CrsKit::compareNoCase(className.c_str(), "Lambert_Conformal_Conic_West_Orientated"))
			return { AxisOrientationEnum::West, AxisOrientationEnum::North };
		if (0 == CrsKit::compareNoCase(className.c_str(), "Krovak") || 0 == CrsKit::compareNoCase(className.c_str(), "Krovak_Modified"))
			return { AxisOrientationEnum::South, AxisOrientationEnum::West };
		return { AxisOrientationEnum::East, AxisOrientationEnum::North };
	}

	auto Opposite(AxisOrientationEnum orientation) -> AxisOrientationEnum
	{
		switch (orientation)
		{
		case AxisOrientationEnum::East: return AxisOrientationEnum::West;
		case AxisOrientationEnum::West: return AxisOrientationEnum::East;
		case AxisOrientationEnum::North: return AxisOrientationEnum::South;
		case AxisOrientationEnum::South: return AxisOrientationEnum::North;
		default: return orientation;
		}
	}

	// When the CRS or the formula has a west or south axis, the matrix that takes the formula output to the CRS axes
	// (EPSG 8044, Cassini-Soldner with axes X south, Y west, is (-N, -E); EPSG 5514, Krovak with axes east and north, is
	// (-W, -S)). Empty otherwise.
	auto AxesFromFormula(ProjectedCoordinateSystem const& pcs) -> std::optional<std::array<double, 4>>
	{
		auto const a0 = pcs.GetAxis(0).GetOrientation();
		auto const a1 = pcs.GetAxis(1).GetOrientation();
		auto const westOrSouth = [](AxisOrientationEnum o) { return o == AxisOrientationEnum::West || o == AxisOrientationEnum::South; };
		auto const [f0, f1] = FormulaAxes(pcs.GetProjection()->GetClassName());
		if (!westOrSouth(a0) && !westOrSouth(a1) && !westOrSouth(f0) && !westOrSouth(f1))
			return std::nullopt;
		auto const row = [&](AxisOrientationEnum axis) -> std::optional<std::pair<double, double>>
		{
			if (axis == f0) return std::pair{ 1.0, 0.0 };
			if (axis == Opposite(f0)) return std::pair{ -1.0, 0.0 };
			if (axis == f1) return std::pair{ 0.0, 1.0 };
			if (axis == Opposite(f1)) return std::pair{ 0.0, -1.0 };
			return std::nullopt;
		};
		auto const r0 = row(a0);
		auto const r1 = row(a1);
		if (!r0 || !r1)
			return std::nullopt;
		return std::array{ r0->first, r0->second, r1->first, r1->second };
	}

	// Appends the steps that apply 'vertical' (1->1, 3->1 or 3->3) to the height of an XYZ point.
	auto AppendVerticalStep(std::vector<std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform>>& chain, std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform> const& vertical) -> void
	{
		using namespace CrsKit::CoordinateTransformations;
		auto const source = vertical->GetSourceDimension();
		auto const target = vertical->GetTargetDimension();
		if (1 == source && 1 == target)
		{
			chain.push_back(std::make_shared<PassThrough>(2, vertical));
			return;
		}
		if (3 == source && 1 == target)
		{
			chain.push_back(std::make_shared<ThreeCoordinateHeightTransform>(nullptr, vertical));
			return;
		}
		if (3 == source && 3 == target)
		{
			chain.push_back(vertical);
			return;
		}
		throw std::runtime_error(std::format("A vertical transformation from {} to {} dimensions cannot be applied to the height of a 3D point.", source, target));
	}

	// False under UnknownCrsPolicy::Identity without resolveTransform: an unknown vertical then keeps the height as it is.
	auto ResolvesUnknownVertical(CrsKit::CoordinateTransformationOptions const& options) -> bool
	{
		return nullptr != options.resolveTransform || CrsKit::UnknownCrsPolicy::Identity != options.unknownCrsPolicy;
	}

	// The located vertical steps as one transform: the step itself when there is one.
	auto ConcatenateVertical(std::vector<std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform>> const& steps) -> std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform>
	{
		if (1 == steps.size())
			return steps[0];
		return std::make_shared<CrsKit::CoordinateTransformations::ConcatenatedTransform>(steps);
	}
}
namespace CrsKit::CoordinateTransformations
{
	auto CoordinateTransformationFactory::CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS) const->std::shared_ptr<ICoordinateTransformation>
	{
		return StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(sourceCS, targetCS);
	}
	auto CoordinateTransformationFactory::CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation>
	{
		return StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(sourceCS, targetCS, options);
	}
	auto CoordinateTransformationFactory::CreateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation>
	{
		return StaticCoordinateTransformationFactory::CreateFromVerticalCoordinateSystems(sourceCS, targetCS, options);
	}
	auto CoordinateTransformationFactory::CreateFrom3DCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation>
	{
		return StaticCoordinateTransformationFactory::CreateFrom3DCoordinateSystems(sourceCS, targetCS, options);
	}
	auto CoordinateTransformationFactory::CreateFromHorizontalCoordinateSystems(std::shared_ptr<CoordinateSystem> const& geographic3D, std::shared_ptr<CoordinateSystem> const& target, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation>
	{
		return StaticCoordinateTransformationFactory::CreateFromHorizontalCoordinateSystems(geographic3D, target, options);
	}

	auto StaticCoordinateTransformationFactory::HasLatitudeFirst(ICoordinateSystem const& geographic) -> bool
	{
		// Reads the axis orientations and not the axis names: a Well-Known Text version 2 (WKT2) written by PROJ names the
		// axes "geodetic latitude (Lat)" and "geodetic longitude (Lon)", while the catalogue names them "Lat" and "Lon".
		auto const firstAxisOrientation = geographic.GetAxis(0).GetOrientation();
		auto const secondAxisOrientation = geographic.GetAxis(1).GetOrientation();
		return (AxisOrientationEnum::North == firstAxisOrientation || AxisOrientationEnum::South == firstAxisOrientation)
			&& (AxisOrientationEnum::East == secondAxisOrientation || AxisOrientationEnum::West == secondAxisOrientation);
	}

	auto StaticCoordinateTransformationFactory::CreateProjectionTransforms(std::shared_ptr<ProjectedCoordinateSystem> const& pcs) -> std::shared_ptr<IMathTransform>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (HasLatitudeFirst(*pcs->GetGeographicCoordinateSystem()))
			transformations.push_back(Affine::AffineSwapAxesTransform(2));

		if (fabs(pcs->GetGeographicCoordinateSystem()->GetAngularUnit().GetRadiansPerUnit() - M_PI / 180.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(pcs->GetGeographicCoordinateSystem()->GetAngularUnit().GetRadiansPerUnit() * 180 / M_PI, 2));

		transformations.push_back(ExtensionManager::CreateMathTransform(pcs->GetProjection(), false));

		if (fabs(pcs->GetLinearUnit().GetMetersPerUnit() - 1.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(1.0 / pcs->GetLinearUnit().GetMetersPerUnit(), 2));

		if (auto const m = AxesFromFormula(*pcs))
		{
			if ((*m)[0] != 1.0 || (*m)[3] != 1.0)
				transformations.push_back(Affine::Affine2D((*m)[0], (*m)[1], (*m)[2], (*m)[3]));
			return make_shared<ConcatenatedTransform>(transformations);
		}

		//  Coordinate swap depending on the axes:
		//
		//	In a projected coordinate system it is normal to find X (oriented east), Y (oriented north), but there are projected coordinate systems such as
		//	or 3035 whose axes are: Y (North), X (East); therefore in these systems X and Y would need to be swapped when calling the transformation algorithms
		//	(which by definition always expect to receive X,Y) by means of an affine transformation that swaps axes.
		//
		//	There is another coordinate system (22175, used in test 5101_part_4) whose axes are X (north), Y (east). In this case they must not be swapped.
		//
		//	So this might suggest that the criterion for swapping axes is to look at the orientation of the first axis, and if it is North and the second East, swap them,
		//  but that is not the case, since there are coordinate reference systems, like 23700, whose first axis is Y (east), X (north) as in test 5105_part_2, in which
		//	the axes must not be swapped, they simply have a different name.
		//
		//	There are projected systems whose axes are not named X,Y,E,N, such as system 3388 (Pulkovo 1942 / Caspian Sea Mercator) used in the test
		//	5112_Mercator_(variantB), whose names are "none" and where the coordinate axis must indeed be swapped.
		//	
		//	Summary:
		//		X | East  | Y | North | <-- NO
		//		Y | North | X | East  | <-- YES
		//		X | North | Y | East  | <-- NO
		//		Y | East  | X | North | <-- NO
		//	    * * | North | * | East  | <-- YES
		//
		//	So first I check the names. If they are Y,X or N,E I swap them when their orientations are North,South.
		//	If the names are not X,Y,E,N I swap them when their orientations are North, East.
		if ((pcs->GetAxis(0).GetName() == "Y" || pcs->GetAxis(0).GetName() == "N") && (pcs->GetAxis(1).GetName() == "X" || pcs->GetAxis(1).GetName() == "E") ||
			(pcs->GetAxis(0).GetName() != "X" && pcs->GetAxis(0).GetName() != "Y" && pcs->GetAxis(0).GetName() != "E" && pcs->GetAxis(0).GetName() != "N" &&
				pcs->GetAxis(1).GetName() != "X" && pcs->GetAxis(1).GetName() != "Y" && pcs->GetAxis(1).GetName() != "E" && pcs->GetAxis(1).GetName() != "N"))
		{
			if (pcs->GetAxis(0).GetOrientation() == AxisOrientationEnum::North && pcs->GetAxis(1).GetOrientation() == AxisOrientationEnum::East)
				transformations.push_back(Affine::AffineSwapAxesTransform(2));
		}

		return make_shared<ConcatenatedTransform>(transformations);
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(std::shared_ptr<VerticalCoordinateSystem> const& vertical) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (fabs(vertical->GetVerticalUnit().GetMetersPerUnit() - 1.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(vertical->GetVerticalUnit().GetMetersPerUnit(), 1));

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(std::shared_ptr<VerticalCoordinateSystem> const& vertical) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (fabs(vertical->GetVerticalUnit().GetMetersPerUnit() - 1.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(1.0 / vertical->GetVerticalUnit().GetMetersPerUnit(), 1));

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::string const& transformationName, std::string const& description, std::string const& areaName, std::shared_ptr<VerticalCoordinateSystem> const& source, std::shared_ptr<VerticalCoordinateSystem> const& target, std::shared_ptr<IMathTransform> const& mathTransform, bool inverse, int authorityCode) -> std::shared_ptr<CoordinateTransformation>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (source->GetAxis(0).GetOrientation() != target->GetAxis(0).GetOrientation())
			transformations.push_back(Affine::AffineMainDiagonalTransform(-1.0, 1));

		for (auto const& transformation : CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(source))
			transformations.push_back(transformation);

		if (nullptr != mathTransform)
			transformations.push_back(mathTransform);

		for (auto const& transformation : CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(target))
			transformations.push_back(transformation);

		auto const concatenatedTransformation = make_shared<ConcatenatedTransform>(transformations);

		if (inverse)
			return make_shared<CoordinateTransformation>(
				transformationName,
				description,
				areaName,
				target,
				source,
				concatenatedTransformation->GetInverse(),
				authorityCode);

		return make_shared<CoordinateTransformation>(
			transformationName,
			description,
			areaName,
			source,
			target,
			concatenatedTransformation,
			authorityCode);
	}

	auto StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::string const& transformationName, std::string const& description, std::string const& areaName, std::shared_ptr<GeocentricCoordinateSystem> const& source, std::shared_ptr<GeocentricCoordinateSystem> const& target, std::shared_ptr<IMathTransform> const& mathTransform, bool inverse, int authorityCode) -> std::shared_ptr<CoordinateTransformation>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		// Note: geocentric axis-orientation sign handling is not done here (no such systems encountered).
		//if( source->GetAxis(0).GetOrientation() != target->GetAxis(0).GetOrientation() )
		//	transformations.push_back(Affine::AffineMainDiagonalTransform(-1.0, 1));

		for (auto const& transformation : CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(source))
			transformations.push_back(transformation);

		if (nullptr != mathTransform)
		{
			auto const concatenated = dynamic_pointer_cast<ConcatenatedTransform>(mathTransform);
			if (nullptr != concatenated)
			{
				for (auto const& transformation : concatenated->GetTransformations())
					transformations.push_back(transformation);
			}
			else
			{
				transformations.push_back(mathTransform);
			}
		}

		for (auto const& transformation : CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(target))
			transformations.push_back(transformation);

		auto const concatenatedTransformation = make_shared<ConcatenatedTransform>(transformations);

		if (inverse)
			return make_shared<CoordinateTransformation>(
				transformationName,
				description,
				areaName,
				target,
				source,
				concatenatedTransformation->GetInverse(),
				authorityCode);

		return make_shared<CoordinateTransformation>(
			transformationName,
			description,
			areaName,
			source,
			target,
			concatenatedTransformation,
			authorityCode);
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(std::shared_ptr<GeographicCoordinateSystem> const& geo, int pointDimensions) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (HasLatitudeFirst(*geo))
			if (2 == pointDimensions)
				transformations.push_back(Affine::AffineSwapAxesTransform(2));
			else if (3 == pointDimensions)
				transformations.push_back(Affine::TransformXyzToYxz());

		if (fabs(geo->GetAngularUnit().GetRadiansPerUnit() - M_PI / 180.0) > 1E-15)
		{
			auto const degreesPerUnit = geo->GetAngularUnit().GetRadiansPerUnit() * 180 / M_PI;
			if (2 == pointDimensions)
				transformations.push_back(Affine::AffineMainDiagonalTransform(degreesPerUnit, 2));
			else if (3 == pointDimensions)
				transformations.push_back(Affine::AffineMainDiagonalTransform({ degreesPerUnit, degreesPerUnit, 1.0 }));
		}

		if (3 == pointDimensions)
			if (auto const heightConversion = CreateHeightConversionToMetres(*geo))
				transformations.push_back(heightConversion);

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateHeightConversionToMetres(GeographicCoordinateSystem const& geo) -> shared_ptr<IMathTransform>
	{
		if (3 != geo.GetDimension() || geo.GetUnits().size() < 3)
			return nullptr;

		auto const* heightUnit = std::get_if<LinearUnit>(&geo.GetUnits()[2]);
		if (nullptr == heightUnit || fabs(heightUnit->GetMetersPerUnit() - 1.0) <= 1E-15)
			return nullptr;

		return Affine::AffineMainDiagonalTransform({ 1.0, 1.0, heightUnit->GetMetersPerUnit() });
	}

	auto StaticCoordinateTransformationFactory::CreatePrimeMeridianToGreenwichLongitudeConversion(GeographicCoordinateSystem const& geographicSystem, int pointDimensionCount) -> shared_ptr<IMathTransform>
	{
		auto const primeMeridian = geographicSystem.GetPrimeMeridian();
		auto const primeMeridianLongitudeInDegrees = primeMeridian.GetLongitude() * primeMeridian.GetAngularUnit().GetRadiansPerUnit() * 180.0 / M_PI;
		if (0.0 == primeMeridianLongitudeInDegrees)
			return nullptr;

		return make_shared<LongitudeRotation>(pointDimensionCount, primeMeridianLongitudeInDegrees, false);
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(std::shared_ptr<GeographicCoordinateSystem> const& geo) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (fabs(geo->GetAngularUnit().GetRadiansPerUnit() - M_PI / 180.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(M_PI / 180.0 / geo->GetAngularUnit().GetRadiansPerUnit(), 2));

		if (HasLatitudeFirst(*geo))
			transformations.push_back(Affine::AffineSwapAxesTransform(2));

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(std::shared_ptr<GeocentricCoordinateSystem> const& system) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		// The affine transformation converts X, Y and Z from the linear unit of system to metres.
		if (fabs(system->GetLinearUnit().GetMetersPerUnit() - 1.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(system->GetLinearUnit().GetMetersPerUnit(), 3));

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(std::shared_ptr<GeocentricCoordinateSystem> const& system) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		// The affine transformation converts X, Y and Z from metres to the linear unit of system.
		if (fabs(system->GetLinearUnit().GetMetersPerUnit() - 1.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(1.0 / system->GetLinearUnit().GetMetersPerUnit(), 3));

		return transformations;
	}


	auto StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::string const& transformationName, std::string const& description, std::string const& areaName, std::shared_ptr<GeographicCoordinateSystem> const& source, std::shared_ptr<GeographicCoordinateSystem> const& target, std::shared_ptr<IMathTransform> const& mathTransform, bool inverse, int authorityCode) -> std::shared_ptr<CoordinateTransformation>
	{
		// When source and target are three-dimensional, mathTransform receives and returns longitude and latitude in degrees
		// and the ellipsoidal height in metres. Otherwise the transformation receives and returns the two horizontal
		// coordinates, also for a three-dimensional source or target: FindTransformsGeographic3DToCompound and
		// FindTransformsCompoundToGeographic3D apply it to the horizontal coordinates through PassThrough.
		auto const requiresThreeCoordinates = 3 == source->GetDimension() && 3 == target->GetDimension();

		auto transformations = CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(source, requiresThreeCoordinates ? 3 : 2);

		if (nullptr != mathTransform)
		{
			auto const concatenated = dynamic_pointer_cast<ConcatenatedTransform>(mathTransform);
			if (nullptr != concatenated)
			{
				for (auto const& transformation : concatenated->GetTransformations())
					transformations.push_back(transformation);
			}
			else
			{
				transformations.push_back(mathTransform);
			}
		}

		if (requiresThreeCoordinates)
		{
			for (auto const& transformation : CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(target, 3) | std::views::reverse)
				transformations.push_back(transformation->GetInverse());
		}
		else
		{
			for (auto const& transformation : CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(target))
				transformations.push_back(transformation);
		}

		if (0 == transformations.size())
			return make_shared<CoordinateTransformation>(transformationName, description, areaName, source, target, Affine::AffineMainDiagonalTransform(1.0, std::max(source->GetDimension(), target->GetDimension())), authorityCode);
		if (1 == transformations.size())
			return make_shared<CoordinateTransformation>(transformationName, description, areaName, source, target, inverse ? transformations[0]->GetInverse() : transformations[0], authorityCode);

		auto concatenatedTransformation = make_shared<ConcatenatedTransform>(transformations);

		if (inverse)
			return make_shared<CoordinateTransformation>(
				transformationName,
				description,
				areaName,
				target,
				source,
				concatenatedTransformation->GetInverse(),
				authorityCode);
		else
			return make_shared<CoordinateTransformation>(
				transformationName,
				description,
				areaName,
				source,
				target,
				concatenatedTransformation,
				authorityCode);
	}

	auto StaticCoordinateTransformationFactory::CreateAssumeWorkingFrameTransform(std::shared_ptr<CoordinateSystem> const& a, std::shared_ptr<CoordinateSystem> const& b) -> std::shared_ptr<IMathTransform>
	{
		auto const dimension = std::max(a->GetDimension(), b->GetDimension());
		if (dimension >= 2
			&& (a->GetAxis(0).GetOrientation() != b->GetAxis(0).GetOrientation()
				|| a->GetAxis(1).GetOrientation() != b->GetAxis(1).GetOrientation()))
			return Affine::AffineSwapAxesTransform(2);

		return Affine::AffineMainDiagonalTransform(1.0, dimension);
	}

	auto StaticCoordinateTransformationFactory::ResolveUnknownCrs(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, CoordinateTransformationOptions const& options) -> std::shared_ptr<IMathTransform>
	{
		if (options.resolveTransform)
		{
			auto const transform = options.resolveTransform(source, target);
			if (transform)
				return transform;
			throw runtime_error("The transformation between an unknown/local coordinate system and a known one was rejected.");
		}

		if (options.unknownCrsPolicy == UnknownCrsPolicy::Identity)
			return CreateAssumeWorkingFrameTransform(source, target);

		throw runtime_error("It is not possible to transform between two reference coordinate systems if one of them is unknown or local.");
	}

	namespace
	{
		auto AreSameDatum(std::shared_ptr<IHorizontalDatum> const& firstDatum, std::shared_ptr<IHorizontalDatum> const& secondDatum) -> bool;
		auto CreateTransformBetweenGeographicAndGeocentricSystemsOfDifferentDatums(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, std::shared_ptr<GeographicCoordinateSystem> const& geographic, std::shared_ptr<GeocentricCoordinateSystem> const& geocentric, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>;

		// Returns the conversion of three coordinates from geographic to geocentric with the ellipsoid of geographic; it
		// does not shift the datum.
		auto CreateGeographicToGeocentricConversion(std::shared_ptr<GeographicCoordinateSystem> const& geographic, std::shared_ptr<GeocentricCoordinateSystem> const& geocentric) -> std::shared_ptr<IMathTransform>
		{
			auto transformations = StaticCoordinateTransformationFactory::CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(geographic, 3);
			if (auto const primeMeridianToGreenwichLongitudeConversion = StaticCoordinateTransformationFactory::CreatePrimeMeridianToGreenwichLongitudeConversion(*geographic, 3))
				transformations.push_back(primeMeridianToGreenwichLongitudeConversion);
			transformations.push_back(make_shared<GeographicToGeocentric>(geographic->GetHorizontalDatum()->GetEllipsoid(), false));
			for (auto const& geocentricSystemTransform : StaticCoordinateTransformationFactory::CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(geocentric))
				transformations.push_back(geocentricSystemTransform);

			return make_shared<ConcatenatedTransform>(transformations);
		}
	}

	auto StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		if ((nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(sourceCS) || nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(targetCS)) &&
			(nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(sourceCS) || nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(targetCS)))
			return make_shared<CoordinateTransformation>("", "", "", sourceCS, targetCS, ResolveUnknownCrs(sourceCS, targetCS, options));

		// CreateFromCoordinateSystems builds the transformation between geographic and geocentric coordinate reference systems
		// (CRS) because the authority factory does not handle this pair. With the same datum, CreateFromCoordinateSystems uses
		// the conversion returned by CreateGeographicToGeocentricConversion for a geographic source and the inverse conversion
		// for a geocentric source; with different datums, the transformation also shifts the datum between the geographic CRS
		// and the geographic CRS of the geocentric datum.
		{
			auto const sourceGeographic = dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS);
			auto const sourceGeocentric = dynamic_pointer_cast<GeocentricCoordinateSystem>(sourceCS);
			auto const targetGeographic = dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS);
			auto const targetGeocentric = dynamic_pointer_cast<GeocentricCoordinateSystem>(targetCS);
			if ((sourceGeographic && targetGeocentric) || (sourceGeocentric && targetGeographic))
			{
				auto const geographic = sourceGeographic ? sourceGeographic : targetGeographic;
				auto const geocentric = sourceGeocentric ? sourceGeocentric : targetGeocentric;
				// If CreateFromCoordinateSystems throws TransformationNotFoundException for the datum shift and
				// HaveSameEllipsoidAndPrimeMeridian returns true, CreateTransformBetweenGeographicAndGeocentricSystemsOfDifferentDatums
				// returns nullptr, and CreateFromCoordinateSystems converts with the ellipsoid of the geographic coordinate reference
				// system (CRS).
				if (!AreSameDatum(geographic->GetHorizontalDatum(), geocentric->GetHorizontalDatum()))
					if (auto const transformation = CreateTransformBetweenGeographicAndGeocentricSystemsOfDifferentDatums(sourceCS, targetCS, geographic, geocentric, options))
						return transformation;

				auto const geographicToGeocentric = CreateGeographicToGeocentricConversion(geographic, geocentric);
				std::shared_ptr<IMathTransform> mt = sourceGeographic
					? std::static_pointer_cast<IMathTransform>(geographicToGeocentric)
					: geographicToGeocentric->GetInverse();
				return make_shared<CoordinateTransformation>("", "", "", sourceCS, targetCS, mt);
			}
		}

		// Geographic 3D -> vertical (a geoid model) is routed here BEFORE the EPSG shortcut below,
		// because the authority factory hands back the geoid operation raw: it interpolates the grid at
		// (longitude, latitude), but a geographic CRS may well declare its axes the other way round --
		// EPSG 4979 is (Lat, Lon, h) -- and nothing in that path swaps them. Feeding it a point as the
		// CRS declares it silently sampled the geoid at the transposed location (over Madrid, an
		// undulation of -32 m instead of +52 m: an 84 m error in the height). CreateFromVerticalCoordinateSystems
		// normalizes the axes and the angular unit first, and carries the horizontal coordinates through,
		// so the transformation returns three coordinates with the converted height. For a geographic 2D CRS, the
		// transformation created by CreateFromVerticalCoordinateSystems receives the ellipsoidal height in metres as third
		// coordinate.
		if ((2 == sourceCS->GetDimension() || 3 == sourceCS->GetDimension()) && 1 == targetCS->GetDimension())
		{
			auto const geographic = dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS);
			auto const vertical = dynamic_pointer_cast<VerticalCoordinateSystem>(targetCS);
			if (geographic && vertical)
				return CreateFromVerticalCoordinateSystems(geographic, vertical, options);
		}

		// ... and so is the way back, for the very same reason: undoing a geoid means interpolating the
		// same grid at the same point, so a raw operation transposes it just as badly (Madrid came back
		// 84 m below where it went in). The height travels with the horizontal coordinates the geoid
		// needs to interpolate, so this pair is 3D in and 3D out too.
		if (1 == sourceCS->GetDimension() && (2 == targetCS->GetDimension() || 3 == targetCS->GetDimension()))
		{
			auto const vertical = dynamic_pointer_cast<VerticalCoordinateSystem>(sourceCS);
			auto const geographic = dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS);
			if (vertical && geographic)
				return CreateFromVerticalCoordinateSystems(vertical, geographic, options);
		}

		// A compound CRS is resolved component by component below, even when it carries an EPSG code.
		auto const isCompound = [](std::shared_ptr<CoordinateSystem> const& cs) { return nullptr != dynamic_pointer_cast<CompoundCoordinateSystem>(cs); };
		if (sourceCS->GetAuthority() == "EPSG" && targetCS->GetAuthority() == "EPSG" && !isCompound(sourceCS) && !isCompound(targetCS))
			return GetCoordinateTransformationAuthorityFactory()->CreateFromCoordinateSystems(sourceCS, targetCS, options);

		if (3 == sourceCS->GetDimension() && 3 == targetCS->GetDimension())
			return CreateFrom3DCoordinateSystems(sourceCS, targetCS, options);

		if (3 == sourceCS->GetDimension() && 2 == targetCS->GetDimension() || 2 == sourceCS->GetDimension() && 2 == targetCS->GetDimension())
			return CreateFromHorizontalCoordinateSystems(sourceCS, targetCS, options);

		if (3 == sourceCS->GetDimension() && 1 == targetCS->GetDimension() || 1 == sourceCS->GetDimension() && 1 == targetCS->GetDimension()) {
			auto const geographic = dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS);
			auto const vertical = dynamic_pointer_cast<VerticalCoordinateSystem>(targetCS);
			if( geographic && vertical )
				return CreateFromVerticalCoordinateSystems(geographic, vertical, options);
			return CreateFromVerticalCoordinateSystems(sourceCS, targetCS, options);
		}

		if (sourceCS->GetDimension() != targetCS->GetDimension())
			throw runtime_error("You cannot create a transformation between reference coordinate systems of different dimensions. It is not possible to know whether a horizontal or vertical transformation is desired.");

		throw TransformationNotFoundException(std::format("No transformation could be located to transform between the coordinate systems {} and {} as they are neither 3D and horizontal nor vertical.",
																	sourceCS->GetName().c_str(),
																	targetCS->GetName().c_str()));
	}

	namespace
	{
		// EPSG code of the World Geodetic System 1984 datum.
		constexpr int kWgs84DatumCode = 6326;

		// Returns true when the authority code of datum is 6326 or its name, ignoring case, is one of the names of WGS 84 below.
		auto IsWgs84Datum(std::shared_ptr<HorizontalDatum> const& datum) -> bool
		{
			if (kWgs84DatumCode == datum->GetAuthorityCode())
				return true;

			// For an ENSEMBLE named "World Geodetic System 1984 ensemble" without ID, the Well-known Text version 2 (WKT2)
			// parser creates a datum with that name and code 0.
			auto const name = datum->GetName();
			for (auto const candidate : { "WGS_1984", "WGS 84", "WGS84", "D_WGS_1984", "World Geodetic System 1984", "World Geodetic System 1984 ensemble" })
				if (0 == compareNoCase(name.c_str(), candidate))
					return true;
			return false;
		}

		// Returns true when IsWgs84Datum returns true for both datums or HorizontalDatum::CompareTo returns 0. CompareTo
		// compares a datum without an authority code with the other datum by name, and the catalogue names the WGS 84 datum
		// "World Geodetic System 1984 ensemble", whereas Well-Known Text without a code names it WGS_1984, D_WGS_1984 or World
		// Geodetic System 1984. Returns false when a datum is not a HorizontalDatum.
		auto AreSameDatum(std::shared_ptr<IHorizontalDatum> const& firstDatum, std::shared_ptr<IHorizontalDatum> const& secondDatum) -> bool
		{
			auto const firstHorizontalDatum = std::dynamic_pointer_cast<HorizontalDatum>(firstDatum);
			auto const secondHorizontalDatum = std::dynamic_pointer_cast<HorizontalDatum>(secondDatum);
			if (!firstHorizontalDatum || !secondHorizontalDatum)
				return false;
			return (IsWgs84Datum(firstHorizontalDatum) && IsWgs84Datum(secondHorizontalDatum)) || 0 == firstHorizontalDatum->CompareTo(secondHorizontalDatum);
		}

		// Returns true when the corresponding semi-axes of the ellipsoids of the two geographic coordinate reference systems
		// differ by at most maximumSemiAxisDifferenceInMetres and the longitudes of their prime meridians from Greenwich differ
		// by less than 1e-12 radians.
		auto HaveSameEllipsoidAndPrimeMeridian(GeographicCoordinateSystem const& firstGeographicCoordinateSystem, GeographicCoordinateSystem const& secondGeographicCoordinateSystem, double maximumSemiAxisDifferenceInMetres) -> bool
		{
			// An ellipsoid can be defined in a linear unit other than the metre.
			auto const calculateSemiAxesInMetres = [](Ellipsoid const& ellipsoid)
			{
				auto const metresPerUnit = ellipsoid.GetLinearUnit().GetMetersPerUnit();
				return std::pair{ ellipsoid.GetSemiMajorAxis() * metresPerUnit, ellipsoid.GetSemiMinorAxis() * metresPerUnit };
			};
			auto const [firstSemiMajorAxis, firstSemiMinorAxis] = calculateSemiAxesInMetres(firstGeographicCoordinateSystem.GetHorizontalDatum()->GetEllipsoid());
			auto const [secondSemiMajorAxis, secondSemiMinorAxis] = calculateSemiAxesInMetres(secondGeographicCoordinateSystem.GetHorizontalDatum()->GetEllipsoid());
			if (fabs(firstSemiMajorAxis - secondSemiMajorAxis) > maximumSemiAxisDifferenceInMetres || fabs(firstSemiMinorAxis - secondSemiMinorAxis) > maximumSemiAxisDifferenceInMetres)
				return false;

			auto const calculateLongitudeFromGreenwichInRadians = [](PrimeMeridian const& primeMeridian)
			{
				return primeMeridian.GetLongitude() * primeMeridian.GetAngularUnit().GetRadiansPerUnit();
			};
			return fabs(calculateLongitudeFromGreenwichInRadians(firstGeographicCoordinateSystem.GetPrimeMeridian())
				- calculateLongitudeFromGreenwichInRadians(secondGeographicCoordinateSystem.GetPrimeMeridian())) < 1e-12;
		}

		// Two geographic coordinate reference systems (CRSs) share a geodetic frame when AreSameDatum returns true for their
		// datums and HaveSameEllipsoidAndPrimeMeridian returns true with a maximum semi-axis difference of 1e-6 metres. The
		// axes and angular units of the two CRSs may differ.
		auto ShareGeodeticFrame(GeographicCoordinateSystem const& firstGeographicCoordinateSystem, GeographicCoordinateSystem const& secondGeographicCoordinateSystem) -> bool
		{
			return AreSameDatum(firstGeographicCoordinateSystem.GetHorizontalDatum(), secondGeographicCoordinateSystem.GetHorizontalDatum())
				&& HaveSameEllipsoidAndPrimeMeridian(firstGeographicCoordinateSystem, secondGeographicCoordinateSystem, 1e-6);
		}

		// Builds the datum shift between two geographic systems from their TOWGS84 (Bursa-Wolf)
		// parameters, going through WGS 84. By default the rotations are read in the Position Vector
		// convention (EPSG 9606), which is how WKT 1 TOWGS84 is interpreted by GDAL/PROJ; pass
		// CoordinateFrame for the ESRI convention (EPSG 9607), which uses the opposite rotation sign.
		// Returns the geographic-to-geographic core transform, or null when the shift cannot be resolved
		// safely: if a side neither carries TOWGS84 nor is WGS 84, its relation to WGS 84 is unknown and
		// we must NOT assume the identity (silent error).
		auto TryCreateTowgs84Core(std::shared_ptr<GeographicCoordinateSystem> const& source, std::shared_ptr<GeographicCoordinateSystem> const& target, Towgs84RotationConvention rotationConvention) -> std::shared_ptr<IMathTransform>
		{
			auto const sourceDatum = std::dynamic_pointer_cast<HorizontalDatum>(source->GetHorizontalDatum());
			auto const targetDatum = std::dynamic_pointer_cast<HorizontalDatum>(target->GetHorizontalDatum());
			if (!sourceDatum || !targetDatum)
				return nullptr;

			auto const sourceToWgs84 = sourceDatum->GetWgs84ConversionInfo();
			auto const targetToWgs84 = targetDatum->GetWgs84ConversionInfo();

			if (!(sourceToWgs84 || IsWgs84Datum(sourceDatum)) || !(targetToWgs84 || IsWgs84Datum(targetDatum)))
				return nullptr;

			// Both sides WGS 84 (no shift) is handled by the equivalent/same-datum path upstream.
			if (!sourceToWgs84 && !targetToWgs84)
				return nullptr;

			// When source and target are three-dimensional, the chain receives and returns longitude and latitude in degrees and
			// ellipsoidal height in metres; otherwise it receives and returns longitude and latitude in degrees.
			auto const requiresThreeCoordinates = 3 == source->GetDimension() && 3 == target->GetDimension();

			// The direct GeographicToGeocentric receives longitudes from Greenwich and its inverse returns longitudes from
			// Greenwich; the chain receives longitudes from the prime meridian of source and returns longitudes from the prime
			// meridian of target.
			auto const sourcePrimeMeridianToGreenwichLongitudeConversion = StaticCoordinateTransformationFactory::CreatePrimeMeridianToGreenwichLongitudeConversion(*source, requiresThreeCoordinates ? 3 : 2);
			auto const targetPrimeMeridianToGreenwichLongitudeConversion = StaticCoordinateTransformationFactory::CreatePrimeMeridianToGreenwichLongitudeConversion(*target, requiresThreeCoordinates ? 3 : 2);

			// Same datum on both sides (same ellipsoid, same Bursa-Wolf parameters and same prime meridian): the geographic
			// step would be the identity, so there is nothing to shift. Return null and let the caller
			// fall through to its plain same-datum (un)projection handling.
			{
				auto const calculatePrimeMeridianLongitudeInDegrees = [](GeographicCoordinateSystem const& geographic)
				{
					auto const primeMeridian = geographic.GetPrimeMeridian();
					return primeMeridian.GetLongitude() * primeMeridian.GetAngularUnit().GetRadiansPerUnit() * 180.0 / M_PI;
				};
				auto const sameTowgs84 = [](std::shared_ptr<Wgs84ConversionInfo> const& a, std::shared_ptr<Wgs84ConversionInfo> const& b)
				{
					if (static_cast<bool>(a) != static_cast<bool>(b))
						return false;
					if (!a)
						return true;
					return fabs(a->GetDx() - b->GetDx()) < 1e-9 && fabs(a->GetDy() - b->GetDy()) < 1e-9 && fabs(a->GetDz() - b->GetDz()) < 1e-9
						&& fabs(a->GetEx() - b->GetEx()) < 1e-12 && fabs(a->GetEy() - b->GetEy()) < 1e-12 && fabs(a->GetEz() - b->GetEz()) < 1e-12
						&& fabs(a->GetPpm() - b->GetPpm()) < 1e-12;
				};
				if (fabs(sourceDatum->GetEllipsoid().GetSemiMajorAxis() - targetDatum->GetEllipsoid().GetSemiMajorAxis()) < 1e-6
					&& fabs(sourceDatum->GetEllipsoid().GetSemiMinorAxis() - targetDatum->GetEllipsoid().GetSemiMinorAxis()) < 1e-6
					&& sameTowgs84(sourceToWgs84, targetToWgs84)
					&& fabs(calculatePrimeMeridianLongitudeInDegrees(*source) - calculatePrimeMeridianLongitudeInDegrees(*target)) < 1e-12)
					return nullptr;
			}

			auto const sourceEllipsoid = sourceDatum->GetEllipsoid();
			auto const targetEllipsoid = targetDatum->GetEllipsoid();

			// TOWGS84 rotations are in arc-seconds (OGC WKT 1 convention), but PositionVectorTransformation
			// expects them in decimal degrees, so we convert. The Coordinate Frame convention is the same
			// shift with the opposite rotation sign, so we fold that sign into the same factor and keep
			// using PositionVectorTransformation throughout.
			double const rotationFactor = (Towgs84RotationConvention::CoordinateFrame == rotationConvention ? -1.0 : 1.0) / 3600.0;

			std::vector<std::shared_ptr<IMathTransform>> chain;
			if (sourcePrimeMeridianToGreenwichLongitudeConversion)
				chain.push_back(sourcePrimeMeridianToGreenwichLongitudeConversion);
			if (!requiresThreeCoordinates)
				chain.push_back(std::make_shared<Geographic2D2Geographic3D>());
			chain.push_back(std::make_shared<GeographicToGeocentric>(sourceEllipsoid, false));

			// source geocentric -> WGS 84 geocentric
			if (sourceToWgs84)
				chain.push_back(std::make_shared<PositionVectorTransformation>(
					sourceToWgs84->GetDx(), sourceToWgs84->GetDy(), sourceToWgs84->GetDz(),
					sourceToWgs84->GetEx() * rotationFactor, sourceToWgs84->GetEy() * rotationFactor, sourceToWgs84->GetEz() * rotationFactor, sourceToWgs84->GetPpm()));

			// WGS 84 geocentric -> target geocentric (inverse of target -> WGS 84)
			if (targetToWgs84)
				chain.push_back(std::make_shared<PositionVectorTransformation>(
					targetToWgs84->GetDx(), targetToWgs84->GetDy(), targetToWgs84->GetDz(),
					targetToWgs84->GetEx() * rotationFactor, targetToWgs84->GetEy() * rotationFactor, targetToWgs84->GetEz() * rotationFactor, targetToWgs84->GetPpm())->GetInverse());

			chain.push_back(std::make_shared<GeographicToGeocentric>(targetEllipsoid, true));
			if (!requiresThreeCoordinates)
				chain.push_back(std::make_shared<Geographic2D2Geographic3D>(true));
			if (targetPrimeMeridianToGreenwichLongitudeConversion)
				chain.push_back(targetPrimeMeridianToGreenwichLongitudeConversion->GetInverse());

			return std::make_shared<ConcatenatedTransform>(chain);
		}

		// Returns a geographic coordinate reference system (CRS) with the datum of geocentric and dimensionCount axes. When
		// geocentric has a code of the European Petroleum Survey Group (EPSG) catalogue, CreateGeographicSystemOfGeocentricDatum
		// returns the geographic CRS of the catalogue for that datum and dimension, if the catalogue has one. Otherwise,
		// CreateGeographicSystemOfGeocentricDatum returns a CRS without authority code with the prime meridian of geocentric,
		// latitude and longitude in degrees and, for three axes, the ellipsoidal height in metres.
		auto CreateGeographicSystemOfGeocentricDatum(GeocentricCoordinateSystem const& geocentric, int dimensionCount) -> std::shared_ptr<GeographicCoordinateSystem>
		{
			if ("EPSG" == geocentric.GetAuthority() && 0 != geocentric.GetAuthorityCode())
			{
				auto const factory = CrsKit::GetCoordinateSystemAuthorityFactory();
				auto const geographic2DCode = factory->GetCodeOfGeographic2DSystemAssociatedWithDatum(factory->GetCodeOfDatumAssociatedWithCrs(geocentric.GetAuthorityCode()));
				if (0 != geographic2DCode)
				{
					auto const geographic2D = factory->CreateGeographicCoordinateSystem(geographic2DCode);
					if (2 == dimensionCount)
						return geographic2D;
					if (auto const geographic3D = factory->FindGeographicSystem3D(geographic2DCode, geographic2D->GetAxis(0), geographic2D->GetAxis(1)))
						return geographic3D;
				}
			}

			AngularUnit const degree{ "degree", M_PI / 180.0 };
			vector<AxisInfo> axes{ AxisInfo("Lat", AxisOrientationEnum::North), AxisInfo("Lon", AxisOrientationEnum::East) };
			vector<AnyUnit> units{ degree, degree };
			if (3 == dimensionCount)
			{
				axes.emplace_back("h", AxisOrientationEnum::Up);
				units.emplace_back(LinearUnit{ "metre", 1.0 });
			}
			return make_shared<GeographicCoordinateSystem>(geocentric.GetName(), "", 0, "", "", degree, geocentric.GetHorizontalDatum(), geocentric.GetPrimeMeridian(), axes, units);
		}

		// Returns the transformation between a geographic and a geocentric coordinate reference system (CRS) whose datums
		// differ; one of sourceCS and targetCS is geographic and the other is geocentric. The transformation receives and
		// returns three coordinates. The transformation shifts the datum between geographic and the geographic CRS that
		// CreateGeographicSystemOfGeocentricDatum returns, and converts between that CRS and geocentric. For a geographic 2D
		// CRS, the datum shift uses the geographic 3D CRS of the catalogue for the same datum when FindGeographicSystem3D
		// returns one with the same angular unit; otherwise the datum shift transforms latitude and longitude and returns the
		// third coordinate, the ellipsoidal height, unchanged. When CreateFromCoordinateSystems throws
		// TransformationNotFoundException for the datum shift, the function returns nullptr if the exception is not a
		// MultipleTransformationsFoundException, which reports several datum shifts without selectOperation, and
		// HaveSameEllipsoidAndPrimeMeridian returns true for geographic and the geographic CRS of the geocentric datum with a
		// maximum semi-axis difference of 4e-4 metres; otherwise the function propagates the exception.
		auto CreateTransformBetweenGeographicAndGeocentricSystemsOfDifferentDatums(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, std::shared_ptr<GeographicCoordinateSystem> const& geographic, std::shared_ptr<GeocentricCoordinateSystem> const& geocentric, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
		{
			auto geographicForDatumShift = geographic;
			if (2 == geographic->GetDimension())
				if (auto const geographic3D = CoordinateSystemFactory::FindGeographicSystem3D(geographic); geographic3D && fabs(geographic3D->GetAngularUnit().GetRadiansPerUnit() - geographic->GetAngularUnit().GetRadiansPerUnit()) <= 1E-15)
					geographicForDatumShift = geographic3D;

			auto const dimensionCount = geographicForDatumShift->GetDimension();
			auto const geographicOfGeocentricDatum = CreateGeographicSystemOfGeocentricDatum(*geocentric, dimensionCount);

			// CreateFromCoordinateSystems is called once, so selectOperation is called at most once; the inverse datum shift is
			// the inverse of the returned transformation.
			std::shared_ptr<ICoordinateTransformation> datumShiftCoordinateTransformation;
			try
			{
				datumShiftCoordinateTransformation = StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(geographicForDatumShift, geographicOfGeocentricDatum, options);
			}
			catch (MultipleTransformationsFoundException const&)
			{
				throw;
			}
			catch (TransformationNotFoundException const&)
			{
				// The maximum accepted difference between corresponding ellipsoid semi-axes is 0.4 millimetres. The semi-minor axes
				// of the Geodetic Reference System 1980 (GRS 1980) and World Geodetic System 1984 (WGS 84) ellipsoids differ by
				// 0.105 millimetres. For identical latitude, longitude and ellipsoidal height, the Euclidean distance between the
				// geocentric coordinates computed with the GRS 1980 and WGS 84 ellipsoids is at most 0.121 millimetres.
				if (!HaveSameEllipsoidAndPrimeMeridian(*geographic, *geographicOfGeocentricDatum, 4e-4))
					throw;
				return nullptr;
			}
			auto const datumShift = datumShiftCoordinateTransformation->GetMathTransform();
			auto const inverseDatumShift = datumShift->GetInverse();
			auto const adaptDatumShiftToThreeCoordinates = [dimensionCount](std::shared_ptr<IMathTransform> const& datumShiftTransform) -> std::shared_ptr<IMathTransform>
			{
				if (3 == dimensionCount)
					return datumShiftTransform;

				// The datum shift receives latitude and longitude; the third coordinate passes unchanged.
				return make_shared<ConcatenatedTransform>(vector<shared_ptr<IMathTransform>>{ Affine::TransformXyzToZxy(), make_shared<PassThrough>(1, datumShiftTransform), Affine::TransformZxyToXyz() });
			};
			auto const conversion = CreateGeographicToGeocentricConversion(geographicOfGeocentricDatum, geocentric);

			vector<shared_ptr<IMathTransform>> geographicToGeocentricTransforms{ adaptDatumShiftToThreeCoordinates(datumShift), conversion };
			vector<shared_ptr<IMathTransform>> geocentricToGeographicTransforms{ conversion->GetInverse(), adaptDatumShiftToThreeCoordinates(inverseDatumShift) };
			auto const sourceIsGeographic = nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS);
			auto const directTransformation = make_shared<ConcatenatedTransform>(sourceIsGeographic ? geographicToGeocentricTransforms : geocentricToGeographicTransforms);
			auto const inverseTransformation = make_shared<ConcatenatedTransform>(sourceIsGeographic ? geocentricToGeographicTransforms : geographicToGeocentricTransforms);
			directTransformation->SetInverseTransformation(inverseTransformation);
			inverseTransformation->SetInverseTransformation(directTransformation);

			auto const transformation = make_shared<CoordinateTransformation>("", "", "", sourceCS, targetCS, directTransformation);
			transformation->SetInverseMathTransform(inverseTransformation);
			return transformation;
		}
	}

	auto StaticCoordinateTransformationFactory::CreateFromHorizontalCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		auto [horizontalSource, verticalSource] = dynamic_pointer_cast<CoordinateSystem>(sourceCS)->ExtractHorizontalVertical();
		auto [horizontalTarget, verticalTarget] = dynamic_pointer_cast<CoordinateSystem>(targetCS)->ExtractHorizontalVertical();

		if (horizontalSource->GetAuthority() == "EPSG" && horizontalTarget->GetAuthority() == "EPSG")
		{
			return GetCoordinateTransformationAuthorityFactory()->CreateCoordinateTransformBetweenHorizontalSystems(
				dynamic_pointer_cast<HorizontalCoordinateSystem>(horizontalSource),
				dynamic_pointer_cast<HorizontalCoordinateSystem>(horizontalTarget),
				options);
		}

		// Equivalent coordinate systems (same object, same definition, or both local): the library builds
		// the axis-handling transform itself -- identity if the axes match, swap if they are reordered.
		// Axis orientation differences (e.g. a sign flip) fall through to the normal same-datum path below.
		if (sourceCS == targetCS || AreEquivalent(sourceCS, targetCS) || (nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(sourceCS) && nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(targetCS)))
		{
			auto const sourceAxis0 = sourceCS->GetAxis(0).GetOrientation();
			auto const sourceAxis1 = sourceCS->GetAxis(1).GetOrientation();
			auto const targetAxis0 = targetCS->GetAxis(0).GetOrientation();
			auto const targetAxis1 = targetCS->GetAxis(1).GetOrientation();

			if (sourceAxis0 == targetAxis0 && sourceAxis1 == targetAxis1)
				return make_shared<CoordinateTransformation>("", "", "", sourceCS, targetCS, Affine::AffineMainDiagonalTransform(1.0, sourceCS->GetDimension()));

			if (sourceAxis0 == targetAxis1 && sourceAxis1 == targetAxis0)
				return make_shared<CoordinateTransformation>("", "", "", sourceCS, targetCS, Affine::AffineSwapAxesTransform(2));

			// Axis orientations differ: fall through to the normal path (same-datum axis/sign handling).
		}

		if ((nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(sourceCS) || nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(targetCS)) &&
			(nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(sourceCS) || nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(targetCS)))
			return make_shared<CoordinateTransformation>("", "", "", sourceCS, targetCS, ResolveUnknownCrs(sourceCS, targetCS, options));

		auto sourceAsProjected = dynamic_pointer_cast<ProjectedCoordinateSystem>(sourceCS);
		auto const sourceAsGeographic = dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS);
		auto const sourceAsVertical = dynamic_pointer_cast<VerticalCoordinateSystem>(sourceCS);

		auto targetAsProjected = dynamic_pointer_cast<ProjectedCoordinateSystem>(targetCS);
		auto const targetAsGeographic = dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS);
		auto const targetAsVertical = dynamic_pointer_cast<VerticalCoordinateSystem>(targetCS);

		if (nullptr != sourceAsVertical || targetAsVertical != nullptr)
			throw runtime_error("This method only allows the transformation of horizontal reference coordinate systems");

		// Datum shift between two CRS (geographic and/or projected) that are not EPSG-identified, using
		// their embedded TOWGS84 (Bursa-Wolf) parameters via WGS 84. Reuses the axis/unit handling of the
		// EPSG path for the underlying geographic step, and wraps it with the projection (un-project on the
		// source side, project on the target side) for the projected combinations. This runs before the
		// plain (un)projection shortcuts below so a genuine datum shift is taken by the Bursa-Wolf path;
		// TryCreateTowgs84Core returns null when no shift is needed, leaving the same-datum shortcuts to
		// handle pure (un)projection.
		{
			auto const sourceGeo = sourceAsGeographic ? sourceAsGeographic
				: (sourceAsProjected ? dynamic_pointer_cast<GeographicCoordinateSystem>(sourceAsProjected->GetGeographicCoordinateSystem()) : nullptr);
			auto const targetGeo = targetAsGeographic ? targetAsGeographic
				: (targetAsProjected ? dynamic_pointer_cast<GeographicCoordinateSystem>(targetAsProjected->GetGeographicCoordinateSystem()) : nullptr);

			if (nullptr != sourceGeo && nullptr != targetGeo)
				if (auto const core = TryCreateTowgs84Core(sourceGeo, targetGeo, options.towgs84RotationConvention))
				{
					auto const geographicTransform = CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
						sourceGeo->GetName() + " to " + targetGeo->GetName() + " (Bursa-Wolf)", "", "",
						sourceGeo, targetGeo, core, false)->GetMathTransform();

					// Pure geographic <-> geographic: no projection wrapping needed.
					if (nullptr == sourceAsProjected && nullptr == targetAsProjected)
						return make_shared<CoordinateTransformation>(
							sourceCS->GetName() + " to " + targetCS->GetName() + " (Bursa-Wolf)", "", "",
							sourceCS, targetCS, geographicTransform);

					vector<shared_ptr<IMathTransform>> chain;
					if (nullptr != sourceAsProjected)
						chain.push_back(CreateProjectionTransforms(sourceAsProjected)->GetInverse());
					chain.push_back(geographicTransform);
					if (nullptr != targetAsProjected)
						chain.push_back(CreateProjectionTransforms(targetAsProjected));

					return make_shared<CoordinateTransformation>(
						sourceCS->GetName() + " to " + targetCS->GetName() + " (Bursa-Wolf)", "", "",
						sourceCS, targetCS, make_shared<ConcatenatedTransform>(chain));
				}
		}

		// When the source and target geographic CRSs share a geodetic frame, including a CRS defined in Well-Known Text (WKT)
		// without an authority code: remove the projection of the source if the source is projected, convert the axes and the
		// angular unit between the geographic CRSs, and apply the projection of the target if the target is projected.
		{
			auto const sourceGeographicCoordinateSystem = sourceAsGeographic ? sourceAsGeographic
				: (sourceAsProjected ? dynamic_pointer_cast<GeographicCoordinateSystem>(sourceAsProjected->GetGeographicCoordinateSystem()) : nullptr);
			auto const targetGeographicCoordinateSystem = targetAsGeographic ? targetAsGeographic
				: (targetAsProjected ? dynamic_pointer_cast<GeographicCoordinateSystem>(targetAsProjected->GetGeographicCoordinateSystem()) : nullptr);

			if (nullptr != sourceGeographicCoordinateSystem && nullptr != targetGeographicCoordinateSystem
				&& ShareGeodeticFrame(*sourceGeographicCoordinateSystem, *targetGeographicCoordinateSystem))
			{
				vector<shared_ptr<IMathTransform>> transforms;
				if (nullptr != sourceAsProjected)
					transforms.push_back(CreateProjectionTransforms(sourceAsProjected)->GetInverse());
				transforms.push_back(CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
					sourceGeographicCoordinateSystem->GetName() + " to " + targetGeographicCoordinateSystem->GetName(), "", "",
					sourceGeographicCoordinateSystem, targetGeographicCoordinateSystem, nullptr, false)->GetMathTransform());
				if (nullptr != targetAsProjected)
					transforms.push_back(CreateProjectionTransforms(targetAsProjected));

				return make_shared<CoordinateTransformation>("", "", "", sourceCS, targetCS, make_shared<ConcatenatedTransform>(transforms));
			}
		}
		throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", sourceCS->GetName().c_str(), targetCS->GetName().c_str()));
	}

	auto StaticCoordinateTransformationFactory::CreateFromVerticalCoordinateSystems(std::shared_ptr<GeographicCoordinateSystem> const& sourceCS, std::shared_ptr<VerticalCoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		auto transformations = LocateFromVerticalCoordinateSystems(sourceCS, targetCS, options, false);

		// If verticalTransformation has no value here, we ask the user
		if (0 == transformations.size())
		{
			if (nullptr != options.resolveTransform)
			{
				auto const transformation = options.resolveTransform(sourceCS, targetCS);
				if (nullptr != transformation)
					transformations.push_back(transformation);
			}
		}

		if (0 == transformations.size())
		{
			throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", sourceCS->GetName().c_str(), targetCS->GetName().c_str()));
		}

		std::shared_ptr<IMathTransform> verticalTransformation;
		if (1 == transformations.size())
			verticalTransformation = transformations[0];
		else
			verticalTransformation = make_shared<ConcatenatedTransform>(transformations);

		if (1 == sourceCS->GetDimension())
		{
			if (1 == verticalTransformation->GetSourceDimension())
			{
				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					sourceCS,
					targetCS,
					verticalTransformation);
			}

			throw runtime_error("The selected transformation does not allow to transform coordinates from a vertical coordinate system to a vertical one.");
		}

		// For a geographic 2D sourceCS, the transformation receives the ellipsoidal height in metres as third coordinate.
		if (2 == sourceCS->GetDimension() || 3 == sourceCS->GetDimension())
		{
			if (1 == verticalTransformation->GetSourceDimension())
			{
				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					sourceCS,
					targetCS,
					make_shared<PassThrough>(2, verticalTransformation));
			}

			// Some Geo-to-Vertical transformations return three coordinates and others only one coordinate

			// ThreeCoordinateHeightTransform keeps the first two coordinates and replaces the third with the result of
			// verticalTransformation.
			if (1 == verticalTransformation->GetTargetDimension())
				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					sourceCS,
					targetCS,
					make_shared<ThreeCoordinateHeightTransform>(nullptr, verticalTransformation));

			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				sourceCS,
				targetCS,
				verticalTransformation);
		}

		throw runtime_error(std::format("The coordinate reference system {} is neither 1, 2 nor 3 dimensional.", sourceCS->GetName().c_str()));
	}

	// From vertical to geographic 3D: LocateFromVerticalCoordinateSystems(targetCS, sourceCS, options, true) returns steps
	// that receive and return three coordinates in the axes of the geographic 3D coordinate reference system, keep the
	// latitude and the longitude and replace the gravity-related height with the ellipsoidal height.
	auto StaticCoordinateTransformationFactory::CreateFromVerticalCoordinateSystems(std::shared_ptr<VerticalCoordinateSystem> const& sourceCS, std::shared_ptr<GeographicCoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		auto transformations = LocateFromVerticalCoordinateSystems(targetCS, sourceCS, options, true);

		if (0 == transformations.size())
		{
			if (nullptr != options.resolveTransform)
			{
				auto const transformation = options.resolveTransform(sourceCS, targetCS);
				if (nullptr != transformation)
					transformations.push_back(transformation);
			}
		}

		if (0 == transformations.size())
			throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", sourceCS->GetName().c_str(), targetCS->GetName().c_str()));

		std::shared_ptr<IMathTransform> verticalTransformation;
		if (1 == transformations.size())
			verticalTransformation = transformations[0];
		else
			verticalTransformation = make_shared<ConcatenatedTransform>(transformations);

		// ThreeCoordinateHeightTransform keeps the first two coordinates and replaces the third with the result of
		// verticalTransformation.
		if (3 == verticalTransformation->GetSourceDimension() && 1 == verticalTransformation->GetTargetDimension())
			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				sourceCS,
				targetCS,
				make_shared<ThreeCoordinateHeightTransform>(nullptr, verticalTransformation));

		if (1 == verticalTransformation->GetSourceDimension() && 1 == verticalTransformation->GetTargetDimension())
		{
			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				sourceCS,
				targetCS,
				make_shared<PassThrough>(2, verticalTransformation));
		}

		return make_shared<CoordinateTransformation>(
			"",
			"",
			"",
			sourceCS,
			targetCS,
			verticalTransformation);
	}

	auto StaticCoordinateTransformationFactory::CreateFromVerticalCoordinateSystems(std::shared_ptr<VerticalCoordinateSystem> const& sourceCS, std::shared_ptr<VerticalCoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		auto transformations = LocateFromVerticalCoordinateSystems(sourceCS, targetCS, options);

		// If verticalTransformation has no value here, we ask the user
		if (0 == transformations.size())
		{
			if (nullptr != options.resolveTransform)
			{
				auto const transformation = options.resolveTransform(sourceCS, targetCS);
				if (nullptr != transformation)
					transformations.push_back(transformation);
			}
		}

		if (0 == transformations.size())
		{
			throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", sourceCS->GetName().c_str(), targetCS->GetName().c_str()));
		}

		std::shared_ptr<IMathTransform> verticalTransformation;
		if (1 == transformations.size())
			verticalTransformation = transformations[0];
		else
			verticalTransformation = make_shared<ConcatenatedTransform>(transformations);

		if (1 == sourceCS->GetDimension())
		{
			if (1 == verticalTransformation->GetSourceDimension())
			{
				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					sourceCS,
					targetCS,
					verticalTransformation);
			}
			throw runtime_error("The selected transformation does not allow to transform coordinates from a vertical coordinate system to a vertical one.");
		}

		if (3 == sourceCS->GetDimension())
		{
			if (1 == verticalTransformation->GetSourceDimension())
			{
				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					sourceCS,
					targetCS,
					make_shared<PassThrough>(2, verticalTransformation));
			}

			// Some Geo-to-Vertical transformations return three coordinates and others only one coordinate

			// ThreeCoordinateHeightTransform keeps the first two coordinates and replaces the third with the result of
			// verticalTransformation.
			if (1 == verticalTransformation->GetTargetDimension())
				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					sourceCS,
					targetCS,
					make_shared<ThreeCoordinateHeightTransform>(nullptr, verticalTransformation));
			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				sourceCS,
				targetCS,
				verticalTransformation);
		}

		throw runtime_error( std::format("The coordinate reference system {} is neither 1 nor 3 dimensional.", sourceCS->GetName().c_str()) );
	}

	auto StaticCoordinateTransformationFactory::CreateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		auto [horizontalSource, verticalSource] = dynamic_pointer_cast<CoordinateSystem>(sourceCS)->ExtractHorizontalVertical();
		auto [horizontalTarget, verticalTarget] = dynamic_pointer_cast<CoordinateSystem>(targetCS)->ExtractHorizontalVertical();
		return CreateFromVerticalCoordinateSystems(
			dynamic_pointer_cast<VerticalCoordinateSystem>(verticalSource),
			dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget),
			options);
	}

	auto StaticCoordinateTransformationFactory::LocateFromVerticalCoordinateSystems(std::shared_ptr<GeographicCoordinateSystem> const& sourceCS, std::shared_ptr<VerticalCoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options, bool inverse) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (sourceCS->GetAuthority() == "EPSG" && targetCS->GetAuthority() == "EPSG")
		{
			auto const transformation = GetCoordinateTransformationAuthorityFactory()->CreateCoordinateTransformBetweenGeographicVerticalSystems(
				sourceCS,
				targetCS,
				false,
				options)->GetMathTransform();

			// The steps receive and return three coordinates in the axes of sourceCS. If transformation receives three coordinates,
			// horizontalSteps convert the first two from the axis order, angular unit and prime meridian of sourceCS to longitude
			// from Greenwich and latitude in degrees; if it also returns three coordinates, the inverses of horizontalSteps
			// convert them back to the axis order, angular unit and prime meridian of sourceCS.
			auto horizontalSteps = CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(sourceCS, 3);

			// The transformation receives the ellipsoidal height in metres. When CreateHeightConversionToMetres returns a step, the
			// code removes the final step, the height conversion, from horizontalSteps and appends heightConversion to
			// transformations. The remaining horizontalSteps keep the third coordinate, as ThreeCoordinateHeightTransform requires.
			if (auto const heightConversion = CreateHeightConversionToMetres(*sourceCS))
			{
				horizontalSteps.pop_back();
				transformations.push_back(heightConversion);
			}

			// If transformation receives three coordinates, horizontalSteps convert the longitude from the prime meridian of
			// sourceCS to the longitude from Greenwich.
			if (auto const primeMeridianToGreenwichLongitudeConversion = CreatePrimeMeridianToGreenwichLongitudeConversion(*sourceCS, 3))
				horizontalSteps.push_back(primeMeridianToGreenwichLongitudeConversion);

			if (1 == transformation->GetSourceDimension())
				transformations.push_back(make_shared<PassThrough>(2, transformation));
			else if (1 == transformation->GetTargetDimension())
			{
				std::shared_ptr<IMathTransform> horizontalConversion;
				if (1 == horizontalSteps.size())
					horizontalConversion = horizontalSteps[0];
				else if (1 < horizontalSteps.size())
					horizontalConversion = make_shared<ConcatenatedTransform>(horizontalSteps);
				transformations.push_back(make_shared<ThreeCoordinateHeightTransform>(horizontalConversion, transformation));
			}
			else
			{
				std::ranges::copy(horizontalSteps, std::back_inserter(transformations));
				transformations.push_back(transformation);
				for (auto const& step : horizontalSteps | std::views::reverse)
					transformations.push_back(step->GetInverse());
			}

			for (auto const& step : CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(targetCS))
				transformations.push_back(make_shared<PassThrough>(2, step));

			// The inverse of the steps applies the inverse of the height unit conversion to the gravity-related height before the
			// inverse of the transformation.
			if (inverse)
			{
				auto const geographicToVerticalTransformation = make_shared<ConcatenatedTransform>(transformations);
				return { geographicToVerticalTransformation->GetInverse() };
			}
		}

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::LocateFromVerticalCoordinateSystems(std::shared_ptr<VerticalCoordinateSystem> const& sourceCS, std::shared_ptr<VerticalCoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> vector<shared_ptr<IMathTransform>>
	{
		assert(1 == sourceCS->GetDimension());

		vector<shared_ptr<IMathTransform>> transformations;

		// Same vertical datum: the conversion is only an axis change (Up/Down) and a
		// unit; it does not need a transformation operation between datums.
		if (std::dynamic_pointer_cast<BaseInfo>(sourceCS->GetVerticalDatum())->GetName() == std::dynamic_pointer_cast<BaseInfo>(targetCS->GetVerticalDatum())->GetName())
		{
			auto const signo = (sourceCS->GetAxis(0).GetOrientation() == targetCS->GetAxis(0).GetOrientation()) ? 1.0 : -1.0;
			auto const factor = signo * sourceCS->GetVerticalUnit().GetMetersPerUnit() / targetCS->GetVerticalUnit().GetMetersPerUnit();
			transformations.push_back(Affine::AffineMainDiagonalTransform(factor, 1));
			return transformations;
		}

		// An empty result (no usable EPSG operation) lets the caller ask options.resolveTransform. Any other
		// failure, such as a database error, propagates.
		if (sourceCS->GetAuthority() == "EPSG" && targetCS->GetAuthority() == "EPSG")
		{
			std::shared_ptr<IMathTransform> operation;
			try
			{
				operation = GetCoordinateTransformationAuthorityFactory()->CreateFromCoordinateSystems(sourceCS, targetCS, options)->GetMathTransform();
			}
			catch (TransformationNotFoundException const&)
			{
				return transformations;  // No operation, or several and no selectOperation.
			}
			catch (AuthorityCodeNotFoundException const&)
			{
				return transformations;
			}
			catch (UnsupportedFormatException const&)
			{
				return transformations;  // The operation method is not implemented.
			}

			for (auto const& transformation : CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(sourceCS))
				transformations.push_back(transformation);
			transformations.push_back(operation);
			for (auto const& transformation : CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(targetCS))
				transformations.push_back(transformation);
		}

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS) -> std::shared_ptr<ICoordinateTransformation>
	{
		return CreateFromCoordinateSystems(sourceCS, targetCS, {});
	}

	auto StaticCoordinateTransformationFactory::FindTransformsGeographic3DToCompound(std::shared_ptr<GeographicCoordinateSystem> const& geographic3D, std::shared_ptr<CoordinateSystem> const& horizontalTarget, std::shared_ptr<CoordinateSystem> const& verticalTarget, CoordinateTransformationOptions const& options) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		// An unknown/local target vertical: the transformation resolveTransform returns is the vertical step.
		// ResolveUnknownCrs throws on Reject; on Identity the height passes through unchanged.
		auto const verticalTargetLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(verticalTarget);
		if (verticalTargetLocal && ResolvesUnknownVertical(options))
			AppendVerticalStep(transformations, ResolveUnknownCrs(geographic3D, verticalTarget, options));

		if (nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget))
		{
			auto verticalTransformations = LocateFromVerticalCoordinateSystems(
				geographic3D,
				dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget),
				options,
				false);

			if (!verticalTransformations.empty())
				AppendVerticalStep(transformations, ConcatenateVertical(verticalTransformations));
		}

		if (!verticalTargetLocal && 0 == transformations.size())
		{
			std::shared_ptr<IMathTransform> userSpecifiedVerticalTransformation = nullptr;

			if (nullptr != options.resolveTransform)
				userSpecifiedVerticalTransformation = options.resolveTransform(geographic3D, verticalTarget);

			if (nullptr == userSpecifiedVerticalTransformation)
				throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", geographic3D->GetName().c_str(), verticalTarget->GetName().c_str()));

			AppendVerticalStep(transformations, userSpecifiedVerticalTransformation);
		}

		// Transformation horizontal
		transformations.push_back(Affine::TransformXyzToZxy());
		auto const horizontalTransformation = CreateFromHorizontalCoordinateSystems(geographic3D, horizontalTarget, options);
		transformations.push_back(make_shared<PassThrough>(1, horizontalTransformation->GetMathTransform()));
		transformations.push_back(Affine::TransformZxyToXyz());

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::FindTransformsCompoundToGeographic3D(std::shared_ptr<CoordinateSystem> const& horizontalSource, std::shared_ptr<CoordinateSystem> const& verticalSource, std::shared_ptr<GeographicCoordinateSystem> const& target, CoordinateTransformationOptions const& options) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;
		// This is the typical case of WGS 84 + EGM96 -> WGS84
		// The vertical transformation will be the inverse of WGS84 to EGM96, so it must come after the horizontal transformation has been performed

		// Transformation horizontal
		transformations.push_back(Affine::TransformXyzToZxy());
		auto const horizontalTransformation = CreateFromHorizontalCoordinateSystems(horizontalSource, target, options);
		transformations.push_back(make_shared<PassThrough>(1, horizontalTransformation->GetMathTransform()));
		transformations.push_back(Affine::TransformZxyToXyz());

		// We already have the XY coordinates in WGS84 3D with Z in EGM96. We locate a transformation from WGS84 to EGM96 (which is exactly the inverse of the one
		// we are looking for) and apply it.

		vector<shared_ptr<IMathTransform>> verticalTransformations;

		// An unknown/local source vertical: the transformation resolveTransform returns is the vertical step.
		// ResolveUnknownCrs throws on Reject; on Identity the height passes through unchanged.
		auto const verticalSourceLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(verticalSource);
		if (verticalSourceLocal && ResolvesUnknownVertical(options))
			verticalTransformations.push_back(ResolveUnknownCrs(verticalSource, target, options));

		if (nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(verticalSource))
		{
			verticalTransformations = LocateFromVerticalCoordinateSystems(
				target,
				dynamic_pointer_cast<VerticalCoordinateSystem>(verticalSource),
				options,
				true);
		}

		if (!verticalSourceLocal && 0 == verticalTransformations.size())
		{
			std::shared_ptr<IMathTransform> userSpecifiedVerticalTransformation = nullptr;

			if (nullptr != options.resolveTransform)
				userSpecifiedVerticalTransformation = options.resolveTransform(verticalSource, target);

			if (nullptr == userSpecifiedVerticalTransformation)
				throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", verticalSource->GetName().c_str(), target->GetName().c_str()));

			verticalTransformations.push_back(userSpecifiedVerticalTransformation);
		}

		// Some geographic-to-vertical transformations return the three coordinates and others only the height.
		if (!verticalTransformations.empty())
			AppendVerticalStep(transformations, ConcatenateVertical(verticalTransformations));

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::FindTransformsCompoundToCompound(std::shared_ptr<CoordinateSystem> const& horizontalSource, std::shared_ptr<CoordinateSystem> const& verticalSource, std::shared_ptr<CoordinateSystem> const& horizontalTarget, std::shared_ptr<CoordinateSystem> const& verticalTarget, CoordinateTransformationOptions const& options) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		// This is the typical case of WGS 84 / UTM Zone 30N + EGM2008 -> ED50 / UTM 30N + EGM96

		// Exactly one vertical unknown/local: the transformation resolveTransform returns is the vertical step.
		// ResolveUnknownCrs throws on Reject; on Identity the height passes through unchanged.
		auto const verticalSourceLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(verticalSource);
		auto const verticalTargetLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(verticalTarget);
		if (verticalSourceLocal != verticalTargetLocal && ResolvesUnknownVertical(options))
			AppendVerticalStep(transformations, ResolveUnknownCrs(verticalSource, verticalTarget, options));

		if (!verticalSourceLocal && !verticalTargetLocal)
		{
			// As we have two vertical transformations, we transform first
			if (nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(verticalSource) && nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget))
			{
				auto const verticalTransformations = LocateFromVerticalCoordinateSystems(
					dynamic_pointer_cast<VerticalCoordinateSystem>(verticalSource),
					dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget),
					options);
				if (!verticalTransformations.empty())
					AppendVerticalStep(transformations, ConcatenateVertical(verticalTransformations));
			}

			if (0 == transformations.size())
			{
				// We no longer look for the vertical from target to source, but from source to target so as not to confuse the user, so we no longer need to
				// the inverse path for Z.
				std::shared_ptr<IMathTransform> userSpecifiedVerticalTransformation = nullptr;

				if (nullptr != options.resolveTransform)
					userSpecifiedVerticalTransformation = options.resolveTransform(verticalSource, verticalTarget);

				if (nullptr == userSpecifiedVerticalTransformation)
					throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", verticalSource->GetName().c_str(), verticalTarget->GetName().c_str()));

				AppendVerticalStep(transformations, userSpecifiedVerticalTransformation);
			}
		}

		// Transformation horizontal
		transformations.push_back(Affine::TransformXyzToZxy());
		auto const horizontalTransformation = CreateFromHorizontalCoordinateSystems(horizontalSource, horizontalTarget, options);
		transformations.push_back(make_shared<PassThrough>(1, horizontalTransformation->GetMathTransform()));
		transformations.push_back(Affine::TransformZxyToXyz());

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateFrom3DCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		assert(3 == sourceCS->GetDimension());
		assert(3 == targetCS->GetDimension());

		// We also compute the inverse transformation since sometimes the inverse cannot be derived from the direct one because non-rectangular affine transforms may be generated for
		// transform a 3D point into a 2D point, for example.
		auto const directTransformations = FindTransformsToTransformBetween3DCrs(sourceCS, targetCS, options);
		auto const inverseTransformations = FindTransformsToTransformBetween3DCrs(targetCS, sourceCS, options);

		auto const directTransformation = make_shared<ConcatenatedTransform>(directTransformations);
		auto const inverseTransformation = make_shared<ConcatenatedTransform>(inverseTransformations);
		directTransformation->SetInverseTransformation(inverseTransformation);
		inverseTransformation->SetInverseTransformation(directTransformation);

		auto const transformation = make_shared<CoordinateTransformation>(
			"",
			"",
			"",
			sourceCS,
			targetCS,
			directTransformation);

		// The result owns both transforms (direct via _transformation, inverse here) so the weakly
		// cached cross-references resolve while it lives -- exact inverse both ways, no cycle, no leak.
		transformation->SetInverseMathTransform(inverseTransformation);

		return transformation;
	}

	auto StaticCoordinateTransformationFactory::FindTransformsToTransformBetween3DCrs(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		auto [horizontalSource, verticalSource] = dynamic_pointer_cast<CoordinateSystem>(sourceCS)->ExtractHorizontalVertical();
		auto [horizontalTarget, verticalTarget] = dynamic_pointer_cast<CoordinateSystem>(targetCS)->ExtractHorizontalVertical();

		// The Z transformation is complicated, since the following cases can occur:
		// a) Both systems are geographic 3D
		// b) Source is geographic 3D and Target is Local + Local
		// c) Source is geographic 3D and Target is Geographic 2D + Vertical
		// d) Source is geographic 3D and Target is Geographic 2D + Unknown
		// e) Source is geographic 3D and Target is Projected 2D + Vertical
		// f) Source is geographic 3D and Target is Projected 2D + Unknown
		// f) Source is geographic 2D and Target is Projected 2D + Unknown
		// + All the inverse cases.

		// I analyze here all the cases to optimize the transformations:


		// If exactly one horizontal system is unknown/local we ask the options how to resolve it
		// (resolveTransform delegate or unknownCrsPolicy); ResolveUnknownCrs throws on Reject.
		// With the vertical ones it is handled separately through the createVertical delegate.
		auto const horizontalSourceLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(horizontalSource);
		auto const horizontalTargetLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(horizontalTarget);
		if (horizontalSourceLocal != horizontalTargetLocal)
			static_cast<void>(ResolveUnknownCrs(horizontalSource, horizontalTarget, options));

		if (horizontalSourceLocal || horizontalTargetLocal)
		{
			// Unknown/local horizontal -> placed as identity (do nothing).
		}
		else if (nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS))
		{
			// Source is Geographic 3D

			if (nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS))
			{
				// The source and target coordinate reference systems (CRS) are geographic and three-dimensional.
				// CreateFromCoordinateSystems sends two CRS with codes of the European Petroleum Survey Group (EPSG) catalogue to
				// the authority factory, so at least one of these CRS has no EPSG code. The
				// transformation applies the TOWGS84 parameters of the datums or, when the two CRS share a geodetic frame,
				// converts the axes and the units.
				auto const sourceGeographic = dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS);
				auto const targetGeographic = dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS);
				if (auto const datumShift = TryCreateTowgs84Core(sourceGeographic, targetGeographic, options.towgs84RotationConvention))
					transformations.push_back(CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems("", "", "", sourceGeographic, targetGeographic, datumShift, false)->GetMathTransform());
				else if (ShareGeodeticFrame(*sourceGeographic, *targetGeographic))
					transformations.push_back(CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems("", "", "", sourceGeographic, targetGeographic, nullptr, false)->GetMathTransform());
				else
					throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", sourceCS->GetName().c_str(), targetCS->GetName().c_str()));
			}
			else
			{
				transformations = FindTransformsGeographic3DToCompound(
					dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS),
					horizontalTarget,
					verticalTarget,
					options);
			}
		}
		else if (nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(horizontalSource))
		{
			if (nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS))
			{
				transformations = FindTransformsCompoundToGeographic3D(
					horizontalSource,
					verticalSource,
					dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS),
					options);
			}
			else
			{
				transformations = FindTransformsCompoundToCompound(
					horizontalSource,
					verticalSource,
					horizontalTarget,
					verticalTarget,
					options);
			}
		}
		else if (nullptr != dynamic_pointer_cast<ProjectedCoordinateSystem>(horizontalSource))
		{
			if (nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS))
			{
				transformations = FindTransformsCompoundToGeographic3D(
					horizontalSource,
					verticalSource,
					dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS),
					options);
			}
			else
			{
				transformations = FindTransformsCompoundToCompound(
					horizontalSource,
					verticalSource,
					horizontalTarget,
					verticalTarget,
					options);
			}
		}
		return transformations;
	}
}
