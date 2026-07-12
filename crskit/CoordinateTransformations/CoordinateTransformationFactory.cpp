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
#include "Algorithms/Afine.h"
#include "Algorithms/Geographic2D2Geocentric.h"
#include "Algorithms/Geographic2D2Geographic3D.h"
#include "Algorithms/PositionVectorTransformation.h"
#include "../CoordinateSystems/HorizontalDatum.h"
#include "../ExtensionManager.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;
using namespace Algorithms;
using namespace std;

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

	auto StaticCoordinateTransformationFactory::CreateProjectionTransforms(std::shared_ptr<ProjectedCoordinateSystem> const& pcs) -> std::shared_ptr<IMathTransform>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		// The following does not work: sourceCS->GetAxis(0).GetOrientation() == AxisOrientationEnum::North && sourceCS->GetAxis(1).GetOrientation() == AxisOrientationEnum::East
		// because test 5110_part1 would fail since the coordinate system in that case is X:North Y:X, i.e. the axes are not swapped (the coordinate is still
		// formed as X,Y)
		if (pcs->GetGeographicCoordinateSystem()->GetAxis(0).GetName() == "Lat" && (pcs->GetGeographicCoordinateSystem()->GetAxis(1).GetName() == "Long" || pcs->GetGeographicCoordinateSystem()->GetAxis(1).GetName() == "Lon"))
			transformations.push_back(Affine::AffineSwapAxesTransform(2));

		if (fabs(pcs->GetGeographicCoordinateSystem()->GetAngularUnit().GetRadiansPerUnit() - M_PI / 180.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(pcs->GetGeographicCoordinateSystem()->GetAngularUnit().GetRadiansPerUnit() * 180 / M_PI, 2));

		transformations.push_back(ExtensionManager::CreateMathTransform(pcs->GetProjection(), false));

		if (fabs(pcs->GetLinearUnit().GetMetersPerUnit() - 1.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(1.0 / pcs->GetLinearUnit().GetMetersPerUnit(), 2));

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

		if (geo->GetAxis(0).GetName() == "Lat" && (geo->GetAxis(1).GetName() == "Long" || geo->GetAxis(1).GetName() == "Lon"))
			if (2 == pointDimensions)
				transformations.push_back(Affine::AffineSwapAxesTransform(2));
			else if (3 == pointDimensions)
				transformations.push_back(Affine::TransformXyzToYxz());

		if (fabs(geo->GetAngularUnit().GetRadiansPerUnit() - M_PI / 180.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(geo->GetAngularUnit().GetRadiansPerUnit() * 180 / M_PI, 2));

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(std::shared_ptr<GeographicCoordinateSystem> const& geo) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (fabs(geo->GetAngularUnit().GetRadiansPerUnit() - M_PI / 180.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(M_PI / geo->GetAngularUnit().GetRadiansPerUnit() * 180, 2));

		if (geo->GetAxis(0).GetName() == "Lat" && (geo->GetAxis(1).GetName() == "Long" || geo->GetAxis(1).GetName() == "Lon"))
			transformations.push_back(Affine::AffineSwapAxesTransform(2));

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(std::shared_ptr<GeocentricCoordinateSystem> const& system) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (fabs(system->GetLinearUnit().GetMetersPerUnit() - 1.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(1.0 / system->GetLinearUnit().GetMetersPerUnit(), 2));

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(std::shared_ptr<GeocentricCoordinateSystem> const& system) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		if (fabs(system->GetLinearUnit().GetMetersPerUnit() - 1.0) > 1E-15)
			transformations.push_back(Affine::AffineMainDiagonalTransform(system->GetLinearUnit().GetMetersPerUnit(), 2));

		return transformations;
	}


	auto StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::string const& transformationName, std::string const& description, std::string const& areaName, std::shared_ptr<GeographicCoordinateSystem> const& source, std::shared_ptr<GeographicCoordinateSystem> const& target, std::shared_ptr<IMathTransform> const& mathTransform, bool inverse, int authorityCode) -> std::shared_ptr<CoordinateTransformation>
	{
		auto transformations = CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(source, 2);

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

	auto StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		if ((nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(sourceCS) || nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(targetCS)) &&
			(nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(sourceCS) || nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(targetCS)))
			return make_shared<CoordinateTransformation>("", "", "", sourceCS, targetCS, ResolveUnknownCrs(sourceCS, targetCS, options));

		// Geographic <-> geocentric conversion (same geodetic network). It is not an operation of
		// datum, so it is built here (axis reordering + Ellipsoid_To_Geocentric) in
		// instead of delegating to the authority factory, which does not handle geocentric systems.
		{
			auto const sourceGeographic = dynamic_pointer_cast<GeographicCoordinateSystem>(sourceCS);
			auto const sourceGeocentric = dynamic_pointer_cast<GeocentricCoordinateSystem>(sourceCS);
			auto const targetGeographic = dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS);
			auto const targetGeocentric = dynamic_pointer_cast<GeocentricCoordinateSystem>(targetCS);
			if ((sourceGeographic && targetGeocentric) || (sourceGeocentric && targetGeographic))
			{
				auto const geographic = sourceGeographic ? sourceGeographic : targetGeographic;
				auto const geocentric = sourceGeocentric ? sourceGeocentric : targetGeocentric;
				auto const ellipsoid = geographic->GetHorizontalDatum()->GetEllipsoid();

				auto transformations = CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(geographic, 3);
				transformations.push_back(make_shared<GeographicToGeocentric>(ellipsoid.GetSemiMajorAxis(), ellipsoid.GetSemiMinorAxis(), false));
				for (auto const& t : CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(geocentric))
					transformations.push_back(t);

				auto const geographicToGeocentric = make_shared<ConcatenatedTransform>(transformations);
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
		// so a 3D point comes back as a 3D point with its height converted.
		if (3 == sourceCS->GetDimension() && 1 == targetCS->GetDimension())
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
		if (1 == sourceCS->GetDimension() && 3 == targetCS->GetDimension())
		{
			auto const vertical = dynamic_pointer_cast<VerticalCoordinateSystem>(sourceCS);
			auto const geographic = dynamic_pointer_cast<GeographicCoordinateSystem>(targetCS);
			if (vertical && geographic)
				return CreateFromVerticalCoordinateSystems(vertical, geographic, options);
		}

		if (sourceCS->GetAuthority() == "EPSG" && targetCS->GetAuthority() == "EPSG")
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

		// A datum whose relationship to WGS 84 is the identity: WGS 84 itself.
		auto IsWgs84Datum(std::shared_ptr<HorizontalDatum> const& datum) -> bool
		{
			if (kWgs84DatumCode == datum->GetAuthorityCode())
				return true;

			auto const name = datum->GetName();
			for (auto const candidate : { "WGS_1984", "WGS 84", "WGS84", "D_WGS_1984", "World Geodetic System 1984" })
				if (0 == compareNoCase(name.c_str(), candidate))
					return true;
			return false;
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

			// Same datum on both sides (same ellipsoid and same Bursa-Wolf parameters): the geographic
			// step would be the identity, so there is nothing to shift. Return null and let the caller
			// fall through to its plain same-datum (un)projection handling.
			{
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
					&& sameTowgs84(sourceToWgs84, targetToWgs84))
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
			chain.push_back(std::make_shared<Geographic2D2Geographic3D>());
			chain.push_back(std::make_shared<GeographicToGeocentric>(sourceEllipsoid.GetSemiMajorAxis(), sourceEllipsoid.GetSemiMinorAxis(), false));

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

			chain.push_back(std::make_shared<GeographicToGeocentric>(targetEllipsoid.GetSemiMajorAxis(), targetEllipsoid.GetSemiMinorAxis(), true));
			chain.push_back(std::make_shared<Geographic2D2Geographic3D>(true));

			return std::make_shared<ConcatenatedTransform>(chain);
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

		if (nullptr != sourceAsGeographic && nullptr != targetAsProjected && 0 == targetAsProjected->GetGeographicCoordinateSystem()->CompareTo(sourceAsGeographic))
			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				sourceAsGeographic,
				targetAsProjected,
				CreateProjectionTransforms(targetAsProjected));

		if (nullptr != sourceAsProjected && nullptr != targetAsGeographic && 0 == sourceAsProjected->GetGeographicCoordinateSystem()->CompareTo(targetAsGeographic))
			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				sourceAsProjected,
				targetAsGeographic,
				CreateProjectionTransforms(sourceAsProjected)->GetInverse());

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

			if (1 == verticalTransformation->GetTargetDimension())
			{
				// The transformation receives XYZ and returns Z
				// but what we must return is XYZ, so I do the following:
				// XYZ -> XYXYZ
				// PASSTHROUGH(2, transformation)
				transformations.clear();
				transformations.push_back(Affine::TransformXyzToXyXyz());
				transformations.push_back(make_shared<PassThrough>(2, verticalTransformation));

				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					sourceCS,
					targetCS,
					make_shared<ConcatenatedTransform>(transformations));
			}

			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				sourceCS,
				targetCS,
				verticalTransformation);
		}

		throw runtime_error(std::format("The coordinate reference system {} is neither 1 nor 3 dimensional.", sourceCS->GetName().c_str()));
	}

	// Vertical -> geographic 3D: undoing a geoid. LocateFromVerticalCoordinateSystems already knows how
	// to build the inverse chain with the axes and the angular unit normalized (it is what a compound
	// source has always used); all that is left is to carry the horizontal coordinates through, so the
	// point comes back with its latitude and longitude untouched and its height now ellipsoidal.
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

		// The chain receives XYZ and returns just the height, so the horizontal coordinates are
		// duplicated (XYZ -> XYXYZ) and passed around it, exactly as the direct path does.
		if (3 == verticalTransformation->GetSourceDimension() && 1 == verticalTransformation->GetTargetDimension())
		{
			vector<shared_ptr<IMathTransform>> chain;
			chain.push_back(Affine::TransformXyzToXyXyz());
			chain.push_back(make_shared<PassThrough>(2, verticalTransformation));

			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				sourceCS,
				targetCS,
				make_shared<ConcatenatedTransform>(chain));
		}

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

			if (1 == verticalTransformation->GetTargetDimension())
			{
				// The transformation receives XYZ and returns Z
				// but what we must return is XYZ, so I do the following:
				// XYZ -> XYXYZ
				// PASSTHROUGH(2, transformation)
				transformations.clear();
				transformations.push_back(Affine::TransformXyzToXyXyz());
				transformations.push_back(make_shared<PassThrough>(2, verticalTransformation));

				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					sourceCS,
					targetCS,
					make_shared<ConcatenatedTransform>(transformations));
			}
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
			auto transformation = GetCoordinateTransformationAuthorityFactory()->CreateCoordinateTransformBetweenGeographicVerticalSystems(
				sourceCS,
				targetCS,
				false,
				options)->GetMathTransform();
			if (inverse)
				transformation = transformation->GetInverse();

			if (3 == transformation->GetSourceDimension())
			{
				// Three coordinates are received. Let us make sure they come in the proper format (Longitude, Latitude), sexagesimal...
				for (auto const& t : CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(sourceCS, 3))
					transformations.push_back(t);
			}

			transformations.push_back(transformation);

			for (auto const& t : CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(targetCS))
				transformations.push_back(t);
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

		if (sourceCS->GetAuthority() == "EPSG" && targetCS->GetAuthority() == "EPSG")
		{
			try
			{
				for (auto const& transformation : CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(sourceCS))
					transformations.push_back(transformation);

				transformations.push_back(GetCoordinateTransformationAuthorityFactory()->CreateFromCoordinateSystems(
					sourceCS,
					targetCS,
					options)->GetMathTransform());

				for (auto const& transformation : CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(targetCS))
					transformations.push_back(transformation);
			}
			catch (...)
			{
			}
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

		// If the target vertical is unknown/local we resolve it per the options
		// (ResolveUnknownCrs throws on Reject; on Identity the height passes through unchanged).
		auto const verticalTargetLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(verticalTarget);
		if (verticalTargetLocal)
			static_cast<void>(ResolveUnknownCrs(geographic3D, verticalTarget, options));

		if (nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget))
		{
			auto verticalTransformations = LocateFromVerticalCoordinateSystems(
				geographic3D,
				dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget),
				options,
				false);

			std::shared_ptr<IMathTransform> verticalTransformation;
			if (1 == verticalTransformations.size())
				verticalTransformation = verticalTransformations[0];
			else
				verticalTransformation = make_shared<ConcatenatedTransform>(verticalTransformations);

			if (3 == verticalTransformation->GetSourceDimension())
				transformations.push_back(Affine::TransformXyzToXyXyz());

			transformations.push_back(make_shared<PassThrough>(2, verticalTransformation));
		}

		if (!verticalTargetLocal && 0 == transformations.size())
		{
			std::shared_ptr<IMathTransform> userSpecifiedVerticalTransformation = nullptr;

			if (nullptr != options.resolveTransform)
				userSpecifiedVerticalTransformation = options.resolveTransform(geographic3D, verticalTarget);

			if (nullptr == userSpecifiedVerticalTransformation)
				throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", geographic3D->GetName().c_str(), verticalTarget->GetName().c_str()));

			if (3 == userSpecifiedVerticalTransformation->GetSourceDimension())
				transformations.push_back(Affine::TransformXyzToXyXyz());

			transformations.push_back(make_shared<PassThrough>(2, userSpecifiedVerticalTransformation));
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

		// If the source vertical is unknown/local we resolve it per the options
		// (ResolveUnknownCrs throws on Reject; on Identity the height passes through unchanged).
		auto const verticalSourceLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(verticalSource);
		if (verticalSourceLocal)
			static_cast<void>(ResolveUnknownCrs(verticalSource, target, options));

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


		std::shared_ptr<IMathTransform> verticalTransformation;
		if (1 == verticalTransformations.size())
			verticalTransformation = verticalTransformations[0];
		else
			verticalTransformation = make_shared<ConcatenatedTransform>(verticalTransformations);

		// Some Geo-to-Vertical transformations return three coordinates and others only one coordinate

		if (3 == verticalTransformation->GetSourceDimension() && 1 == verticalTransformation->GetTargetDimension())
		{
			// The transformation receives XYZ and returns Z
			// but what we must return is XYZ, so I do the following:
			// XYZ -> XYXYZ
			// PASSTHROUGH(2, transformation)
			transformations.push_back(Affine::TransformXyzToXyXyz());
			transformations.push_back(make_shared<PassThrough>(2, verticalTransformation));
		}
		else if (1 == verticalTransformation->GetSourceDimension() && 1 == verticalTransformation->GetTargetDimension())
		{
			transformations.push_back(make_shared<PassThrough>(2, verticalTransformation));
		}
		else
		{
			for (auto i = verticalTransformations.size() - 1; i >= 0; i--)
				transformations.push_back(verticalTransformations[i]);
		}

		return transformations;
	}

	auto StaticCoordinateTransformationFactory::FindTransformsCompoundToCompound(std::shared_ptr<CoordinateSystem> const& horizontalSource, std::shared_ptr<CoordinateSystem> const& verticalSource, std::shared_ptr<CoordinateSystem> const& horizontalTarget, std::shared_ptr<CoordinateSystem> const& verticalTarget, CoordinateTransformationOptions const& options) -> vector<shared_ptr<IMathTransform>>
	{
		vector<shared_ptr<IMathTransform>> transformations;

		// This is the typical case of WGS 84 / UTM Zone 30N + EGM2008 -> ED50 / UTM 30N + EGM96

		// If exactly one vertical is unknown/local we resolve it per the options
		// (ResolveUnknownCrs throws on Reject; on Identity the height passes through unchanged).
		auto const verticalSourceLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(verticalSource);
		auto const verticalTargetLocal = nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(verticalTarget);
		if (verticalSourceLocal != verticalTargetLocal)
			static_cast<void>(ResolveUnknownCrs(verticalSource, verticalTarget, options));

		if (!verticalSourceLocal && !verticalTargetLocal)
		{
			// As we have two vertical transformations, we transform first
			if (nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(verticalSource) && nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget))
			{
				transformations = LocateFromVerticalCoordinateSystems(
					dynamic_pointer_cast<VerticalCoordinateSystem>(verticalSource),
					dynamic_pointer_cast<VerticalCoordinateSystem>(verticalTarget),
					options);
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

				if (3 == userSpecifiedVerticalTransformation->GetSourceDimension())
					transformations.push_back(Affine::TransformXyzToXyXyz());

				transformations.push_back(make_shared<PassThrough>(2, userSpecifiedVerticalTransformation));
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
				// Both are geographic 3D. 

				// If we run the following query:
				// 
				//SELECT        COORD_OP_CODE, COORD_OP_NAME, COORD_OP_TYPE, SOURCE_CRS_CODE, TARGET_CRS_CODE, COORD_TFM_VERSION, COORD_OP_VARIANT, AREA_OF_USE_CODE, COORD_OP_SCOPE, 
				//	COORD_OP_ACCURACY, COORD_OP_METHOD_CODE, UOM_CODE_SOURCE_COORD_DIFF, UOM_CODE_TARGET_COORD_DIFF, REMARKS, INFORMATION_SOURCE, DATA_SOURCE, REVISION_DATE, 
				//	CHANGE_ID, SHOW_OPERATION, DEPRECATED
				//	FROM            Coordinate_Operation
				//	WHERE        (SOURCE_CRS_CODE IN
				//	(SELECT        COORD_REF_SYS_CODE
				//	FROM            [Coordinate Reference System]
				//WHERE        (COORD_REF_SYS_KIND = 'geographic 3D'))) AND (TARGET_CRS_CODE IN
				//	(SELECT        COORD_REF_SYS_CODE
				//	FROM            [Coordinate Reference System] AS [Coordinate Reference System_1]
				//WHERE        (COORD_REF_SYS_KIND = 'geographic 3D')))
				//
				// Only two appear: "RGPF to WGS 84 (1)" and "RGPF to WGS 84 (2)", which I am not going to implement, so we do nothing.
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
