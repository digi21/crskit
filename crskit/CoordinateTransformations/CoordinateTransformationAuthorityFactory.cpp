#include "pch.h"
#include "ICoordinateTransformationAuthorityFactory.h"
#include "CoordinateTransformationAuthorityFactory.h"
#include "CoordinateTransformation.h"
#include "../CoordinateSystems/BaseInfo.h"
#include "../CoordinateSystems/VerticalDatum.h"
#include "../CoordinateSystems/CoordinateSystemAuthorityFactory.h"
#include "../CoordinateSystems/CoordinateSystem.h"
#include "../CoordinateSystems/HorizontalCoordinateSystem.h"
#include "Algorithms/Afine.h"
#include "CoordinateTransformationFactory.h"
#include "MathTransformFactory.h"
#include "Algorithms/Geographic2D2Geographic3D.h"
#include "Algorithms/Geographic2D2Geocentric.h"
#include "Algorithms/GeocentricTranslations.h"
#include "Algorithms/PositionVectorTransformation.h"
#include "Algorithms/CoordinateFrameRotation.h"
#include "ConcatenatedTransform.h"
#include "CoordinateOperation.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;
using namespace CrsKit::CoordinateTransformations::Algorithms;
using namespace std;

namespace CrsKit::Epsg
{
	CoordinateTransformationAuthorityFactory::CoordinateTransformationAuthorityFactory(std::shared_ptr<Epsg::IAuthorityProvider> const& authorityProvider)
		: _provider{ authorityProvider }
	{
	}

	auto CoordinateTransformationAuthorityFactory::GetAuthority() const -> std::string
	{
		return "EPSG";
	}

	auto CoordinateTransformationAuthorityFactory::CreateFromTransformationCode(int code) -> std::shared_ptr<ICoordinateTransformation>
	{
		return StaticCreateFromTransformationCode(code);
	}

	auto CoordinateTransformationAuthorityFactory::CreateFromCoordinateSystemsCodes(int sourceCode, int targetCode) -> std::shared_ptr<ICoordinateTransformation>
	{
		return StaticCreateFromCoordinateSystemsCodes(sourceCode, targetCode);
	}

	auto CoordinateTransformationAuthorityFactory::CreateFromCoordinateSystemsCodes(int sourceCode, int targetCode, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>
	{
		return StaticCreateFromCoordinateSystemsCodes(sourceCode, targetCode, options);
	}

	auto CoordinateTransformationAuthorityFactory::CreateMathTransform(int code) const -> std::shared_ptr<IMathTransform>
	{
		auto const algorithmCode = _provider->GetAlgorithmCodeForOperation(code);
		auto const projection = GetCoordinateSystemAuthorityFactory()->CreateProjection(code, algorithmCode);

		try
		{
			return MathTransformFactory::CreateParameterizedTransform(projection->GetClassName(), projection->GetParameters(), false);
		}
		catch (GridFileNotFoundException const& e)
		{
			// Enrich the missing-grid error with EPSG operation metadata so the caller can locate the file.
			std::string operationName, areaOfUse, informationSource;
			try
			{
				operationName = _provider->GetCoordinateOperationName(code);
				areaOfUse = _provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(code));
				informationSource = _provider->GetCoordinateOperationInformationSource(code);
			}
			catch (...)
			{
				throw e; // metadata lookup failed: rethrow the basic exception
			}
			throw GridFileNotFoundException{ e.gridFileName, e.searchedPath, code, operationName, areaOfUse, informationSource };
		}
	}

	auto CoordinateTransformationAuthorityFactory::CreateFromTransformationCode(int code, std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, bool inverse) const -> std::shared_ptr<CoordinateTransformation>
	{
		auto const transformationName = _provider->GetCoordinateOperationName(code);
		if (transformationName.empty())
			throw AuthorityCodeNotFoundException(std::format("Could not locate the transformation with authority code: {}", code));

		auto const transformation = CreateMathTransform(code);

		if (nullptr == transformation)
			throw runtime_error(std::format("An error was found when instantiating the transformation with authority code: {}", code));

		try
		{
			return make_shared<CoordinateTransformation>(
				transformationName,
				_provider->GetCoordinateOperationDescription(code),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(code)),
				source,
				target,
				inverse ? transformation->GetInverse() : transformation,
				code);
		}
		catch (...)
		{
			throw AuthorityCodeNotFoundException(std::format("Could not locate the transformation with authority code: {}", code));
		}
	}

	auto CoordinateTransformationAuthorityFactory::StaticCreateFromTransformationCode(int code) const -> std::shared_ptr<ICoordinateTransformation>
	{
		return CreateFromTransformationCode(
			code,
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(_provider->GetCoordinateOperationSourceCrs(code)),
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(_provider->GetCoordinateOperationTargetCrs(code)),
			false);
	}

	auto CoordinateTransformationAuthorityFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::shared_ptr<GeographicCoordinateSystem> const& source, std::shared_ptr<GeographicCoordinateSystem> const& target, int operationCode, bool inverse) const -> std::shared_ptr<CoordinateTransformation>
	{
		auto const algorithmCode = _provider->GetAlgorithmCodeForOperation(operationCode);

		// The operation may be a geocentric operation, for which there is no class because 1: there are no standardized names for the parameters of the two
		// ellipsoids involved and 2: I cannot think of another cause.

		if (9603 == algorithmCode)
		{
			auto parameters = _provider->GetTransformationParameters(operationCode, algorithmCode);

			vector<shared_ptr<IMathTransform>> transformations;
			transformations.push_back(make_shared<Geographic2D2Geographic3D>());
			transformations.push_back(make_shared<GeographicToGeocentric>(source->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), source->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis(), false));
			transformations.push_back(make_shared<GeocentricTranslations>(
				get<double>(parameters[0]),
				get<double>(parameters[1]),
				get<double>(parameters[2])));
			transformations.push_back(make_shared<GeographicToGeocentric>(target->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), target->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis(), true));
			transformations.push_back(make_shared<Geographic2D2Geographic3D>(true));

			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				source,
				target,
				make_shared<ConcatenatedTransform>(transformations),
				inverse,
				operationCode);
		}

		if (9606 == algorithmCode)
		{
			auto parameters = _provider->GetTransformationParameters(operationCode, algorithmCode);

			vector<shared_ptr<IMathTransform>> transformations;
			transformations.push_back(make_shared<Geographic2D2Geographic3D>());
			transformations.push_back(make_shared<GeographicToGeocentric>(source->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), source->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis(), false));
			transformations.push_back(make_shared<PositionVectorTransformation>(
				get<double>(parameters[0]),
				get<double>(parameters[1]),
				get<double>(parameters[2]),
				get<double>(parameters[3]),
				get<double>(parameters[4]),
				get<double>(parameters[5]),
				get<double>(parameters[6])));
			transformations.push_back(make_shared<GeographicToGeocentric>(target->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), target->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis(), true));
			transformations.push_back(make_shared<Geographic2D2Geographic3D>(true));

			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				source,
				target,
				make_shared<ConcatenatedTransform>(transformations),
				inverse,
				operationCode);
		}

		if (9607 == algorithmCode)
		{
			auto parameters = _provider->GetTransformationParameters(operationCode, algorithmCode);

			vector<shared_ptr<IMathTransform>> transformations;
			transformations.push_back(make_shared<Geographic2D2Geographic3D>());
			transformations.push_back(make_shared<GeographicToGeocentric>(source->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), source->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis(), false));
			transformations.push_back(make_shared<CoordinateFrameRotation>(
				get<double>(parameters[0]),
				get<double>(parameters[1]),
				get<double>(parameters[2]),
				get<double>(parameters[3]),
				get<double>(parameters[4]),
				get<double>(parameters[5]),
				get<double>(parameters[6])));
			transformations.push_back(make_shared<GeographicToGeocentric>(target->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), target->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis(), true));
			transformations.push_back(make_shared<Geographic2D2Geographic3D>(true));

			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				source,
				target,
				make_shared<ConcatenatedTransform>(transformations),
				inverse,
				operationCode);
		}

		return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
			_provider->GetCoordinateOperationName(operationCode),
			_provider->GetCoordinateOperationDescription(operationCode),
			_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
			source,
			target,
			CreateMathTransform(operationCode),
			inverse,
			operationCode);
	}

	auto CoordinateTransformationAuthorityFactory::CreateCoordinateTransformBetweenVerticalSystems(std::shared_ptr<VerticalCoordinateSystem> const& source, std::shared_ptr<VerticalCoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformation>
	{
		// If there is no vertical datum change we transform axes and units
		// We compare by DATUM, not by CRS code: two different vertical CRSs (height and
		// depth of the same datum, e.g. EPSG 5611 and 5706 over Caspian Sea 5106) share
		// datum and only differ in the axis direction (handled by the Affine -1). Without this
		// it looked for a non-existent EPSG operation and tried to create a datum with code 0.
		auto const verticalSourceDatum = std::dynamic_pointer_cast<VerticalDatum>(source->GetVerticalDatum());
		auto const sameVerticalDatum = nullptr != verticalSourceDatum && 0 == verticalSourceDatum->CompareTo(target->GetVerticalDatum());

		if (source->GetAuthorityCode() == target->GetAuthorityCode() || sameVerticalDatum)
			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				"",
				"",
				"",
				source,
				target,
				nullptr,
				false);

		// If there is a vertical datum change, we locate the best transformation:

		// We first try from source to target
		auto const transformations = _provider->OperationCodesToTransformSystemAtoB(
			source->GetAuthorityCode(), target->GetAuthorityCode());

		if (transformations.size() > 1)
		{
			if (nullptr == options.selectOperation)
				throw TransformationNotFoundException(std::format("Multiple transformations between the {} coordinate system and the {} coordinate system were located.", source->GetAuthorityCode(), target->GetAuthorityCode()));

			auto const operationCode = options.selectOperation(source->GetName(), target->GetName(), transformations);

			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				source,
				target,
				CreateMathTransform(operationCode),
				false,
				operationCode);
		}

		if (1 == transformations.size())
		{
			auto const operationCode = transformations[0].Code;

			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				source,
				target,
				CreateMathTransform(operationCode),
				false,
				operationCode);
		}

		// There is no direct source->target operation. We try the INVERSE (target->source) and the
		// we invert them: EPSG operations are reversible and are often only defined in one
		// direction (e.g. "Baltic to Caspian" exists but not "Caspian to Baltic"). The query already
		// it filters out deprecated operations, so only current operations are used.
		auto const inverseTransformations = _provider->OperationCodesToTransformSystemAtoB(
			target->GetAuthorityCode(), source->GetAuthorityCode());

		if (inverseTransformations.size() > 1)
		{
			if (nullptr == options.selectOperation)
				throw TransformationNotFoundException(std::format("Multiple transformations between the {} coordinate system and the {} coordinate system were located.", target->GetAuthorityCode(), source->GetAuthorityCode()));

			auto const operationCode = options.selectOperation(target->GetName(), source->GetName(), inverseTransformations);
			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				source,
				target,
				CreateMathTransform(operationCode)->GetInverse(),
				false,
				operationCode);
		}

		if (1 == inverseTransformations.size())
		{
			auto const operationCode = inverseTransformations[0].Code;

			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				source,
				target,
				CreateMathTransform(operationCode)->GetInverse(),
				false,
				operationCode);
		}

		throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", source->GetName().c_str(), target->GetName().c_str()));
	}

	auto CoordinateTransformationAuthorityFactory::CreateCoordinateTransformBetweenGeographicSystems(std::shared_ptr<GeographicCoordinateSystem> source, std::shared_ptr<GeographicCoordinateSystem> target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformation>
	{
		// We first check whether there is a datum change
		if (0 == source->GetHorizontalDatum()->CompareTo(target->GetHorizontalDatum()))
		{
			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				"",
				"",
				"",
				source,
				target,
				nullptr,
				false);
		}

		// If we run the following query:
		//
		//SELECT        COUNT(COORD_OP_CODE) AS Expr1
		//	FROM            Coordinate_Operation
		//	WHERE        (SOURCE_CRS_CODE IN
		//	(SELECT        COORD_REF_SYS_CODE
		//	FROM            [Coordinate Reference System]
		//WHERE        (COORD_REF_SYS_KIND = 'geographic 2D'))) AND (TARGET_CRS_CODE IN
		//	(SELECT        COORD_REF_SYS_CODE
		//	FROM            [Coordinate Reference System] AS [Coordinate Reference System_1]
		//WHERE        (COORD_REF_SYS_KIND = 'geographic 3D')))
		//
		// The result is 0
		// If we run the following:
		//
		//SELECT        COUNT(COORD_OP_CODE) AS Expr1
		//	FROM            Coordinate_Operation
		//	WHERE        (SOURCE_CRS_CODE IN
		//	(SELECT        COORD_REF_SYS_CODE
		//	FROM            [Coordinate Reference System]
		//WHERE        (COORD_REF_SYS_KIND = 'geographic 3D'))) AND (TARGET_CRS_CODE IN
		//	(SELECT        COORD_REF_SYS_CODE
		//	FROM            [Coordinate Reference System] AS [Coordinate Reference System_1]
		//WHERE        (COORD_REF_SYS_KIND = 'geographic 2D')))
		//
		// The result is 0 as well
		//
		// And if finally we run this query:
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
		//
		// Only two appear: "RGPF to WGS 84 (1)" and "RGPF to WGS 84 (2)", which I am not going to implement; therefore, if any of the CRSs passed as parameter is Geographic 3D, I will
		// locate its 2D equivalent and I will look for the transformation between these two 
		if (3 == source->GetDimension())
			source = GetCoordinateSystemAuthorityFactory()->CreateGeographic2DSystemForGeographic3DSystem(source->GetAuthorityCode(), source->GetAxis(0), source->GetAxis(1));
		if (3 == target->GetDimension())
			target = GetCoordinateSystemAuthorityFactory()->CreateGeographic2DSystemForGeographic3DSystem(target->GetAuthorityCode(), target->GetAxis(0), target->GetAxis(1));

		// There is a datum change. We locate the best transformation:

		auto transformations = _provider->OperationCodesToTransformSystemAtoB(
			source->GetAuthorityCode(),
			target->GetAuthorityCode());

		if (0 == transformations.size())
		{
			transformations = _provider->OperationCodesToTransformSystemAtoB(
				target->GetAuthorityCode(),
				source->GetAuthorityCode());

			if (transformations.size() > 1)
			{
				if (nullptr == options.selectOperation)
					throw TransformationNotFoundException(std::format("Multiple transformations between the {} coordinate system and the {} coordinate system were located.", source->GetAuthorityCode(), target->GetAuthorityCode()));

				return CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
					dynamic_pointer_cast<GeographicCoordinateSystem>(target),
					dynamic_pointer_cast<GeographicCoordinateSystem>(source),
					options.selectOperation(source->GetName(), target->GetName(), transformations),
					true);
			}

			if (1 == transformations.size())
			{
				return CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
					dynamic_pointer_cast<GeographicCoordinateSystem>(target),
					dynamic_pointer_cast<GeographicCoordinateSystem>(source),
					transformations[0].Code,
					true);
			}

			throw TransformationNotFoundException(
				std::format("No transformations were found between the coordinate system {} ({}) and the coordinate system {} ({}).",
					source->GetAuthorityCode(),
					source->GetName().c_str(),
					target->GetAuthorityCode(),
					target->GetName().c_str()));
		}

		if (transformations.size() > 1)
		{
			if (nullptr == options.selectOperation)
			{
				throw TransformationNotFoundException(
					std::format("Multiple transformations were located between the {} ({}) coordinate system and the {} ({}) coordinate system.",
						source->GetAuthorityCode(),
						source->GetName().c_str(),
						target->GetAuthorityCode(),
						target->GetName().c_str()));
			}

			return CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				dynamic_pointer_cast<GeographicCoordinateSystem>(source),
				dynamic_pointer_cast<GeographicCoordinateSystem>(target),
				options.selectOperation(source->GetName(), target->GetName(), transformations),
				false);
		}

		if (transformations.size() == 1)
		{
			return CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				dynamic_pointer_cast<GeographicCoordinateSystem>(source),
				dynamic_pointer_cast<GeographicCoordinateSystem>(target),
				transformations[0].Code,
				false);
		}


		throw TransformationNotFoundException(std::format("No transformations were found between the coordinate system {} ({}) and the coordinate system {} ({}).",
			source->GetAuthorityCode(),
			source->GetName().c_str(),
			target->GetAuthorityCode(),
			target->GetName().c_str()));
	}

	auto CoordinateTransformationAuthorityFactory::CreateCoordinateTransformBetweenGeocentricSystems(std::shared_ptr<GeocentricCoordinateSystem> const& source, std::shared_ptr<GeocentricCoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformation>
	{
		auto transformations = _provider->OperationCodesToTransformSystemAtoB(
			source->GetAuthorityCode(), target->GetAuthorityCode());

		if (0 == transformations.size())
		{
			transformations = _provider->OperationCodesToTransformSystemAtoB(target->GetAuthorityCode(), source->GetAuthorityCode());

			if (0 == transformations.size())
			{
				throw TransformationNotFoundException(
					std::format("No transformations were found between the coordinate system {} ({}) and the coordinate system {} ({}).",
						source->GetAuthorityCode(),
						source->GetName().c_str(),
						target->GetAuthorityCode(),
						target->GetName().c_str()));
			}

			if (transformations.size() > 1)
			{
				if (nullptr == options.selectOperation)
				{
					throw TransformationNotFoundException(
						std::format("Multiple transformations were located between the {} ({}) coordinate system and the {} ({}) coordinate system.",
							source->GetAuthorityCode(),
							static_cast<const char*>(source->GetName().c_str()),
							target->GetAuthorityCode(),
							static_cast<const char*>(target->GetName().c_str())));
				}

				auto const operationCode = options.selectOperation(source->GetName(), target->GetName(), transformations);

				return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
					_provider->GetCoordinateOperationName(operationCode),
					_provider->GetCoordinateOperationDescription(operationCode),
					_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
					dynamic_pointer_cast<GeocentricCoordinateSystem>(source),
					dynamic_pointer_cast<GeocentricCoordinateSystem>(target),
					CreateMathTransform(operationCode),
					true,
					operationCode);
			}

			auto const operationCode = transformations[0].Code;

			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				dynamic_pointer_cast<GeocentricCoordinateSystem>(source),
				dynamic_pointer_cast<GeocentricCoordinateSystem>(target),
				CreateMathTransform(operationCode),
				true,
				operationCode);
		}

		if (transformations.size() > 1)
		{
			if (nullptr == options.selectOperation)
			{
				throw TransformationNotFoundException(
					std::format("Multiple transformations were located between the {} ({}) coordinate system and the {} ({}) coordinate system.",
						source->GetAuthorityCode(),
						source->GetName().c_str(),
						target->GetAuthorityCode(),
						target->GetName().c_str()));
			}

			auto const operationCode = options.selectOperation(source->GetName(), target->GetName(), transformations);
			return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				dynamic_pointer_cast<GeocentricCoordinateSystem>(source),
				dynamic_pointer_cast<GeocentricCoordinateSystem>(target),
				CreateMathTransform(operationCode),
				false,
				operationCode);
		}

		auto const operationCode = transformations[0].Code;
		return StaticCoordinateTransformationFactory::CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(
			_provider->GetCoordinateOperationName(operationCode),
			_provider->GetCoordinateOperationDescription(operationCode),
			_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
			dynamic_pointer_cast<GeocentricCoordinateSystem>(source),
			dynamic_pointer_cast<GeocentricCoordinateSystem>(target),
			CreateMathTransform(operationCode),
			false,
			operationCode);
	}

	auto CoordinateTransformationAuthorityFactory::CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformation>
	{
		if ((nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(source) || nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(target)) &&
			(nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(source) || nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(target)))
			throw runtime_error("It is not possible to transform between two reference coordinate systems if one of them is unknown or local.");

		if (0 != compareNoCase(source->GetAuthority().c_str(), "EPSG"))
			throw runtime_error("The origin coordinate system authority is not \"EPSG\".");

		if (0 != compareNoCase(target->GetAuthority().c_str(), "EPSG"))
			throw runtime_error("The target coordinate system authority is not \"EPSG\".");

		//bool aIsProjected = nullptr != dynamic_pointer_cast<ProjectedCoordinateSystem>(source);
		auto const aIsGeographic = nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(source);
		//bool aIsGeocentric = nullptr != dynamic_pointer_cast<GeocentricCoordinateSystem>(source);
		auto const aIsVertical = nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(source);

		//bool bIsProjected = nullptr != dynamic_pointer_cast<ProjectedCoordinateSystem>(target);
		auto const bIsGeographic = nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(target);
		//bool bIsGeocentric = nullptr != dynamic_pointer_cast<GeocentricCoordinateSystem>(target);
		auto const bIsVertical = nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(target);

		if (aIsVertical && bIsVertical)
		{
			return CreateCoordinateTransformBetweenVerticalSystems(
				dynamic_pointer_cast<VerticalCoordinateSystem>(source),
				dynamic_pointer_cast<VerticalCoordinateSystem>(target),
				options);
		}

		if (aIsGeographic && bIsVertical)
		{
			return CreateCoordinateTransformBetweenGeographicVerticalSystems(
				dynamic_pointer_cast<GeographicCoordinateSystem>(source),
				dynamic_pointer_cast<VerticalCoordinateSystem>(target),
				false,
				options);
		}

		if (aIsVertical && bIsGeographic)
		{
			return CreateCoordinateTransformBetweenGeographicVerticalSystems(
				dynamic_pointer_cast<GeographicCoordinateSystem>(target),
				dynamic_pointer_cast<VerticalCoordinateSystem>(source),
				true,
				options);
		}

		return CreateCoordinateTransformBetweenHorizontalSystems(
			dynamic_pointer_cast<HorizontalCoordinateSystem>(source),
			dynamic_pointer_cast<HorizontalCoordinateSystem>(target),
			options);
	}

	auto CoordinateTransformationAuthorityFactory::CreateCoordinateTransformBetweenHorizontalSystems(std::shared_ptr<HorizontalCoordinateSystem> const& source, std::shared_ptr<HorizontalCoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformation>
	{
		auto const aIsProjected = nullptr != dynamic_pointer_cast<ProjectedCoordinateSystem>(source);
		auto const aIsGeographic = nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(source);
		auto const aIsGeocentric = nullptr != dynamic_pointer_cast<GeocentricCoordinateSystem>(source);
		if (!aIsProjected && !aIsGeographic && !aIsGeocentric)
			throw runtime_error(std::format("The coordinate system origin: {} is neither projected nor 2D geographic nor vertical nor geocentric.", source->GetName().c_str()));

		auto const bIsProjected = nullptr != dynamic_pointer_cast<ProjectedCoordinateSystem>(target);
		auto const bIsGeographic = nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(target);
		auto const bIsGeocentric = nullptr != dynamic_pointer_cast<GeocentricCoordinateSystem>(target);
		if (!bIsProjected && !bIsGeographic && !bIsGeocentric)
			throw runtime_error(std::format("The target coordinate system: {} is neither projected nor 2D geographic nor vertical nor geocentric.", target->GetName().c_str()));

		// If the AuthorityCodes match, the axis orientation may still differ. We call CreateCoordinateTransformWithinSameCrs which takes care of
		// that particular case.
		if (0 == source->GetAuthorityCode() - target->GetAuthorityCode())
			return CreateCoordinateTransformWithinSameCrs(source, target);

		if (aIsGeographic && bIsProjected)
		{
			auto const s = dynamic_pointer_cast<GeographicCoordinateSystem>(source);
			auto const t = dynamic_pointer_cast<ProjectedCoordinateSystem>(target);

			if (0 == s->CompareTo(t->GetGeographicCoordinateSystem()))
			{
				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					s,
					t,
					StaticCoordinateTransformationFactory::CreateProjectionTransforms(t));
			}

			vector<shared_ptr<IMathTransform>> transformations;
			transformations.push_back(CreateCoordinateTransformBetweenGeographicSystems(s, dynamic_pointer_cast<GeographicCoordinateSystem>(t->GetGeographicCoordinateSystem()), options)->GetMathTransform());
			transformations.push_back(StaticCoordinateTransformationFactory::CreateProjectionTransforms(t));

			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				s,
				t,
				make_shared<ConcatenatedTransform>(transformations));
		}

		if (aIsProjected && bIsGeographic)
		{
			auto const s = dynamic_pointer_cast<ProjectedCoordinateSystem>(source);
			auto t = dynamic_pointer_cast<GeographicCoordinateSystem>(target);

			vector<shared_ptr<IMathTransform>> transformations;

			if (0 == t->CompareTo(s->GetGeographicCoordinateSystem()))
				transformations.push_back(StaticCoordinateTransformationFactory::CreateProjectionTransforms(s)->GetInverse());
			else
			{
				transformations.push_back(StaticCoordinateTransformationFactory::CreateProjectionTransforms(s)->GetInverse());
				transformations.push_back(CreateCoordinateTransformBetweenGeographicSystems(dynamic_pointer_cast<GeographicCoordinateSystem>(s->GetGeographicCoordinateSystem()), t, options)->GetMathTransform());
			}

			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				s,
				t,
				make_shared<ConcatenatedTransform>(transformations));
		}

		if (aIsProjected && bIsProjected)
		{
			// We check whether they are compatible
			auto s = dynamic_pointer_cast<ProjectedCoordinateSystem>(source);
			auto const t = dynamic_pointer_cast<ProjectedCoordinateSystem>(target);

			if (0 == s->GetGeographicCoordinateSystem()->CompareTo(t->GetGeographicCoordinateSystem()))
			{
				// They share the geographic coordinate system, so we concatenate the projection transform of proj
				vector<shared_ptr<IMathTransform>> transformations;
				transformations.push_back(StaticCoordinateTransformationFactory::CreateProjectionTransforms(s)->GetInverse());
				transformations.push_back(StaticCoordinateTransformationFactory::CreateProjectionTransforms(t));

				return make_shared<CoordinateTransformation>(
					"",
					"",
					"",
					s,
					t,
					make_shared<ConcatenatedTransform>(transformations));
			}

			vector<shared_ptr<IMathTransform>> transformations;
			transformations.push_back(StaticCoordinateTransformationFactory::CreateProjectionTransforms(s)->GetInverse());
			transformations.push_back(CreateCoordinateTransformBetweenGeographicSystems(dynamic_pointer_cast<GeographicCoordinateSystem>(s->GetGeographicCoordinateSystem()), dynamic_pointer_cast<GeographicCoordinateSystem>(t->GetGeographicCoordinateSystem()), options)->GetMathTransform());
			transformations.push_back(StaticCoordinateTransformationFactory::CreateProjectionTransforms(t));

			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				s,
				t,
				make_shared<ConcatenatedTransform>(transformations));
		}

		if (aIsGeographic && bIsGeographic)
		{
			return CreateCoordinateTransformBetweenGeographicSystems(dynamic_pointer_cast<GeographicCoordinateSystem>(source), dynamic_pointer_cast<GeographicCoordinateSystem>(target), options);
		}

		if (aIsGeocentric && bIsGeocentric)
		{
			return CreateCoordinateTransformBetweenGeocentricSystems(dynamic_pointer_cast<GeocentricCoordinateSystem>(source), dynamic_pointer_cast<GeocentricCoordinateSystem>(target), options);
		}

		throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", source->GetName().c_str(), target->GetName().c_str()));
	}

	auto CoordinateTransformationAuthorityFactory::CreateCoordinateTransformBetweenGeographicVerticalSystems(std::shared_ptr<GeographicCoordinateSystem> const& source, std::shared_ptr<VerticalCoordinateSystem> const& target, bool inverse, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformation>
	{
		auto const transformations = _provider->OperationCodesToTransformSystemAtoB(
			source->GetAuthorityCode(),
			target->GetAuthorityCode());

		if (0 == transformations.size())
		{
			// Datum-equivalent geoid fallback: the geoid operation may be catalogued against a specific
			// geographic realization (e.g. Alicante height -> EPSG 9410, whose source is ETRS89-ESP
			// [REGENTE], not the generic ETRS89/WGS 84 requested here), so the exact (source, target)
			// pair finds nothing. Locate the geoid operation by the vertical target and reuse it: the
			// grid is interpolated at the point's lat/lon, so the source realization is immaterial.
			auto const geoidOperations = _provider->GeographicToVerticalOperationCodes(target->GetAuthorityCode());

			// Several geoid models may serve the same vertical system (EGM2008 height has two: a 1' grid
			// and a 2.5' one), and they are not interchangeable. Ambiguity is the caller's to resolve
			// here exactly as it is below, when the pair IS catalogued: ask, and refuse to guess when
			// there is nobody to ask. Taking the first would silently pick a grid the caller never chose.
			if (geoidOperations.size() > 1)
			{
				if (nullptr == options.selectOperation)
					throw TransformationNotFoundException(std::format("Multiple transformations between the {} coordinate system and the {} coordinate system were located.", source->GetAuthorityCode(), target->GetAuthorityCode()));

				return CreateCoordinateTransformBetweenGeographicVerticalSystems(
					source,
					target,
					options.selectOperation(source->GetName(), target->GetName(), geoidOperations),
					inverse);
			}

			if (1 == geoidOperations.size())
				return CreateCoordinateTransformBetweenGeographicVerticalSystems(source, target, geoidOperations.front().Code, inverse);

			auto const transformation = options.resolveTransform ? options.resolveTransform(source, target) : nullptr;

			if (nullptr != transformation)
				return make_shared<CoordinateTransformation>("", "", "", source, target, transformation);

			throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", source->GetName().c_str(), target->GetName().c_str()));
		}

		if (transformations.size() > 1)
		{
			if (nullptr == options.selectOperation)
				throw TransformationNotFoundException(std::format("Multiple transformations between the {} coordinate system and the {} coordinate system were located.", source->GetAuthorityCode(), target->GetAuthorityCode()));

			return CreateCoordinateTransformBetweenGeographicVerticalSystems(
				dynamic_pointer_cast<GeographicCoordinateSystem>(source),
				dynamic_pointer_cast<VerticalCoordinateSystem>(target),
				options.selectOperation(source->GetName(), target->GetName(), transformations),
				false);
		}

		if (transformations.size() == 1)
		{
			return CreateCoordinateTransformBetweenGeographicVerticalSystems(
				dynamic_pointer_cast<GeographicCoordinateSystem>(source),
				dynamic_pointer_cast<VerticalCoordinateSystem>(target),
				transformations[0].Code,
				inverse);
		}

		throw TransformationNotFoundException(std::format("No transformations between the coordinate system {} and {} have been found.", source->GetName().c_str(), target->GetName().c_str()));
	}

	auto CoordinateTransformationAuthorityFactory::CreateCoordinateTransformBetweenGeographicVerticalSystems(std::shared_ptr<GeographicCoordinateSystem> const& source, std::shared_ptr<VerticalCoordinateSystem> const& target, int operationCode, bool inverse) const -> std::shared_ptr<CoordinateTransformation>
	{
		if (inverse)
		{
			return make_shared<CoordinateTransformation>(
				_provider->GetCoordinateOperationName(operationCode),
				_provider->GetCoordinateOperationDescription(operationCode),
				_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
				target,
				source,
				CreateMathTransform(operationCode)->GetInverse(),
				operationCode);
		}

		return make_shared<CoordinateTransformation>(
			_provider->GetCoordinateOperationName(operationCode),
			_provider->GetCoordinateOperationDescription(operationCode),
			_provider->GetAreaName(_provider->GetCoordinateOperationAreaOfUse(operationCode)),
			source,
			target,
			CreateMathTransform(operationCode),
			operationCode);
	}

	auto CoordinateTransformationAuthorityFactory::CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target) const -> std::shared_ptr<ICoordinateTransformation>
	{
		return CreateFromCoordinateSystems(source, target, {});
	}

	auto CoordinateTransformationAuthorityFactory::StaticCreateFromCoordinateSystemsCodes(int sourceCode, int targetCode) const -> std::shared_ptr<ICoordinateTransformation>
	{
		return CreateFromCoordinateSystems(
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(sourceCode),
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(targetCode),
			{});
	}

	auto CoordinateTransformationAuthorityFactory::StaticCreateFromCoordinateSystemsCodes(int sourceCode, int targetCode, CoordinateTransformationOptions const& options) const -> std::shared_ptr<ICoordinateTransformation>
	{
		return CreateFromCoordinateSystems(
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(sourceCode),
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(targetCode),
			options);
	}

	auto CoordinateTransformationAuthorityFactory::CreateCoordinateTransformWithinSameCrs(std::shared_ptr<HorizontalCoordinateSystem> const& source, std::shared_ptr<HorizontalCoordinateSystem> const& target) -> std::shared_ptr<CoordinateTransformation>
	{
		if (source->GetAxis(0).GetOrientation() == target->GetAxis(0).GetOrientation() && source->GetAxis(1).GetOrientation() == target->GetAxis(1).GetOrientation())
			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				source,
				target,
				Affine::AffineMainDiagonalTransform(1.0, source->GetDimension()));

		if (3 == source->GetDimension())
		{
			return make_shared<CoordinateTransformation>(
				"",
				"",
				"",
				source,
				target,
				Affine::TransformXyzToYxz());
		}

		return make_shared<CoordinateTransformation>(
			"",
			"",
			"",
			source,
			target,
			Affine::AffineSwapAxesTransform(2));
	}
}
