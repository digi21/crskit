#include "pch.h"
using namespace CrsKit::Math;
#include "Utilities.h"

#include "CoordinateSystems/CoordinateSystemAuthorityFactory.h"
#include "CoordinateTransformations/ConcatenatedTransform.h"
#include "CoordinateTransformations/CoordinateTransformationFactory.h"
#include "CoordinateTransformations/Algorithms/Afine.h"
#include "CoordinateTransformations/Passthrough.h"
#include "CoordinateTransformations/ThreeCoordinateHeightTransform.h"
#include "CoordinateTransformations/Algorithms/GeocentricToTopocentric.h"
#include "CoordinateTransformations/Algorithms/Geographic2D2Geocentric.h"
#include "CoordinateTransformations/Algorithms/ObliqueStereographic.h"
#include <filesystem>
#include <fstream>

using namespace std;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations::Algorithms;
using namespace CrsKit::CoordinateTransformations;
using namespace CrsKit::Epsg;

namespace CrsKit
{
	namespace
	{
		// False if the EPSG catalogue has no CRS with this code or its kind cannot be built from the code
		// alone (compound, engineering, Local...). A database failure propagates.
		auto IsCreatableEpsgCode(int const code, bool const geographicOnly) -> bool
		{
			auto const kind = GetCoordinateSystemAuthorityFactory()->GetKindOfCoordinateSystem(code);
			if (0 == compareNoCase(kind.c_str(), "geographic 2D") || 0 == compareNoCase(kind.c_str(), "geographic 3D"))
				return true;

			return !geographicOnly && (0 == compareNoCase(kind.c_str(), "projected") || 0 == compareNoCase(kind.c_str(), "vertical") || 0 == compareNoCase(kind.c_str(), "geocentric"));
		}

		// Returns steps that receive and return three coordinates: a height transformation from one coordinate is applied to
		// the third coordinate, and one from three coordinates to one replaces the third coordinate.
		auto CreateThreeCoordinateHeightSteps(std::shared_ptr<IMathTransform> const& heightTransformation) -> std::vector<std::shared_ptr<IMathTransform>>
		{
			if (1 == heightTransformation->GetSourceDimension())
				return { make_shared<PassThrough>(2, heightTransformation) };

			if (1 == heightTransformation->GetTargetDimension())
				return { make_shared<ThreeCoordinateHeightTransform>(nullptr, heightTransformation) };

			return { heightTransformation };
		}

		// Returns steps that receive and return longitude and latitude in degrees and a height, the convention of the
		// transformation methods: the steps change the coordinates to the axes and the angular unit of geographicSystem3D,
		// apply transformationInAxesOfGeographicSystem3D and change the coordinates back to the convention.
		auto CreateStepsWithLongitudeLatitudeInDegreesAndHeight(std::shared_ptr<GeographicCoordinateSystem> const& geographicSystem3D, std::shared_ptr<IMathTransform> const& transformationInAxesOfGeographicSystem3D) -> std::vector<std::shared_ptr<IMathTransform>>
		{
			auto const geographicAxesToLongitudeLatitudeInDegreesAndHeightSteps = StaticCoordinateTransformationFactory::CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(geographicSystem3D, 3);

			std::vector<std::shared_ptr<IMathTransform>> steps;
			for (auto const& step : geographicAxesToLongitudeLatitudeInDegreesAndHeightSteps | std::views::reverse)
				steps.push_back(step->GetInverse());
			steps.push_back(transformationInAxesOfGeographicSystem3D);
			std::ranges::copy(geographicAxesToLongitudeLatitudeInDegreesAndHeightSteps, std::back_inserter(steps));
			return steps;
		}
	}

	auto Utilities::GetCompoundLocalWkt() const -> std::string
	{
		return "COMPD_CS[\"Local or unknown horizontal coordinate system + Local or unknown vertical coordinate system\",LOCAL_CS[\"Local or unknown horizontal coordinate system\", LOCAL_DATUM[\"Local or unknown horizontal datum\", 32767],UNIT[\"unknown\",1.0000000000],AXIS[\"X\", EAST],AXIS[\"Y\",NORTH]],LOCAL_CS[\"Local or unknown vertical coordinate system\", LOCAL_DATUM[\"Local or unknown vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]";
	}

	auto Utilities::GetCompoundLocalWkt(std::string const& title) const -> std::string
	{
		return std::format("COMPD_CS[\"{}\",LOCAL_CS[\"Unknown horizontal coordinate system\", LOCAL_DATUM[\"Local horizontal datum\", 32767],UNIT[\"unknown\",1.0000000000],AXIS[\"X\", EAST],AXIS[\"Y\",NORTH]],LOCAL_CS[\"Unknown vertical coordinate system\", LOCAL_DATUM[\"Local vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]", title.c_str());
	}

	auto Utilities::GetCompoundHorizontalPlusVerticalLocalWkt(const char* const title, const char* const horizontalWkt) const -> std::string
	{
		return std::format("COMPD_CS[\"{} + Unknown vertical coordinate system\",{},LOCAL_CS[\"Unknown vertical coordinate system\", LOCAL_DATUM[\"Local vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]", title, horizontalWkt);
	}

	auto Utilities::GetUnknownVerticalWkt() const -> std::string
	{
		return "LOCAL_CS[\"Local or unknown vertical coordinate system\", LOCAL_DATUM[\"Local or unknown vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]";
	}

	auto Utilities::GetWktFrom3DSystem(std::string const& wkt)  const->std::string
	{
		try {
			auto coordinateSystem = GetCoordinateSystemFactory()->CreateFromWkt(wkt);

			if (coordinateSystem->GetDimension() == 2) {
				auto const verticalLocal = GetCoordinateSystemFactory()->CreateFromWkt(GetUnknownVerticalWkt());

				coordinateSystem = GetCoordinateSystemFactory()->CreateCompoundCoordinateSystem(
					std::format("{} + {}", coordinateSystem->GetName().c_str(), verticalLocal->GetName().c_str()),
					coordinateSystem,
					verticalLocal);
			}

			return coordinateSystem->GetWkt();
		}
		catch (...)
		{
			return "";
		}
	}


	auto Utilities::GetUnknownHorizontalWkt() const->std::string
	{
		return "LOCAL_CS[\"Local or unknown horizontal coordinate system\", LOCAL_DATUM[\"Local or unknown horizontal datuml\", 32767],UNIT[\"unknown\",1.0000000000],AXIS[\"X\", EAST],AXIS[\"Y\",NORTH]]";
	}

	auto Utilities::CreateCompoundCrsWktFromHorizontalAndVerticalCrs(std::string const& horizontalWkt, std::string const& verticalWkt) const -> std::string
	{
		return std::format("COMPD_CS[\"{} + {}\",{},{}]",
			GetCoordinateReferenceSystemName(horizontalWkt).c_str(),
			GetCoordinateReferenceSystemName(verticalWkt).c_str(),
			horizontalWkt.c_str(),
			verticalWkt.c_str());
	}

	std::string Utilities::CreateCompoundCrsWktFromHorizontalCrs(std::string const& horizontalWkt) const
	{
		return std::format("COMPD_CS[\"{} + Unknown vertical coordinate system\",{},LOCAL_CS[\"Unknown vertical coordinate system\", LOCAL_DATUM[\"Local vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]",
			GetCoordinateReferenceSystemName(horizontalWkt).c_str(),
			horizontalWkt.c_str());
	}

	auto Utilities::GetCompoundCrsWktFromHorizontalCrs(std::string const& horizontalWkt) const -> std::string
	{
		return std::format("COMPD_CS[\"{} + Unknown vertical coordinate system\",{},LOCAL_CS[\"Unknown vertical coordinate system\", LOCAL_DATUM[\"Local vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]",
			GetCoordinateReferenceSystemName(horizontalWkt).c_str(),
			horizontalWkt.c_str());
	}

	auto Utilities::GetCoordinateReferenceSystemName(int const code) const -> std::string
	{
		if (!IsCreatableEpsgCode(code, false))
			return {};

		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code)->GetName();
	}

	auto Utilities::GetCoordinateReferenceSystemName(std::string const& wkt) const -> std::string
	{
		return GetCoordinateSystemFactory()->CreateFromWkt(wkt)->GetName();
	}

	auto Utilities::GetWktFromEpsgCode(int const code) const -> std::string
	{
		if (!IsCreatableEpsgCode(code, false))
			return {};

		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code)->GetWkt();
	}

	auto Utilities::GetWktFromProjectionCodeForGeographic(int const projectionCode, int const geographicCode) const -> std::string
	{
		return GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(projectionCode, geographicCode)->GetWkt();
	}


	auto Utilities::GetWktFromGeographic3DSystemForProjected(std::string const& wkt) const->std::string
	{
		auto const projected = dynamic_pointer_cast<ProjectedCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt));
		if (!projected)
			return {};

		return projected->GetGeographicCoordinateSystem()->GetWkt();
	}

	auto Utilities::GetWktFromGeographicEpsgCode(int const epsgCode, bool const longitudeLatitude) const -> std::string
	{
		if (!IsCreatableEpsgCode(epsgCode, true))
			return {};

		shared_ptr<GeographicCoordinateSystem> coordinateSystem;

		if (longitudeLatitude)
		{
			vector<AxisInfo> axes;
			axes.resize(2);
			axes[0].SetName("Long");
			axes[0].SetOrientation(AxisOrientationEnum::East);
			axes[1].SetName("Lat");
			axes[1].SetOrientation(AxisOrientationEnum::North);

			coordinateSystem = GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(epsgCode, axes);
		}
		else
		{
			coordinateSystem = GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(epsgCode);
		}

		return coordinateSystem->GetWkt();
	}

	auto Utilities::GetWktFromEpsgCodeWithEastNorthAxes(int const epsgCode) const -> std::string
	{
		if (!IsCreatableEpsgCode(epsgCode, false))
			return {};

		auto coordinateSystem = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(epsgCode);
		coordinateSystem = CoordinateSystemFactory::WithAxesEastNorth(coordinateSystem);
		return coordinateSystem->GetWkt();
	}

	auto Utilities::GetWktFromCrsCodeWithEastNorthAxes(std::string const& crsCode) const->std::string
	{
		// parse_int returns 0 for a non-numeric code, which falls through to the "not recognized" error.
		if (crsCode.starts_with("EPSG:")) {
			auto const epsgCode = parse_int(crsCode.c_str() + 5);
			if (auto wkt = GetWktFromEpsgCodeWithEastNorthAxes(epsgCode); !wkt.empty())
				return wkt;
		}

		if (crsCode.starts_with("CRS:")) {
			auto const crsNumber = parse_int(crsCode.c_str() + 4);
			auto const epsgCode = 84 == crsNumber ? 4326 : 83 == crsNumber ? 4269 : 27 == crsNumber ? 4267 : 0;
			if (0 != epsgCode) {
				if (auto wkt = GetWktFromEpsgCodeWithEastNorthAxes(epsgCode); !wkt.empty())
					return wkt;
			}
		}

		throw runtime_error{ std::format("The coordinate reference system: {} is not recognized", crsCode.c_str()).c_str() };
	}

	auto Utilities::GetWktWithEastNorthAxes(std::string const& wkt) const -> std::string
	{
		auto coordinateSystem = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
		coordinateSystem = CoordinateSystemFactory::WithAxesEastNorth(coordinateSystem);
		return coordinateSystem->GetWkt();
	}


	auto Utilities::GetAxisNames(std::string const& wkt) const -> vector<std::string>
	{
		vector<std::string> result;

		auto const system = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
		if (auto const compound = dynamic_pointer_cast<CompoundCoordinateSystem>(system))
		{
			for (auto i = 0; i < compound->GetHeadCS()->GetDimension(); i++)
				result.push_back(compound->GetHeadCS()->GetAxis(i).GetName());

			for (auto i = 0; i < compound->GetTailCS()->GetDimension(); i++)
				result.push_back(compound->GetTailCS()->GetAxis(i).GetName());

			return result;
		}

		for (auto i = 0; i < system->GetDimension(); i++)
			result.push_back(system->GetAxis(i).GetName());
		return result;
	}

	auto Utilities::GetUnitNames(std::string const& wkt) const -> std::vector<std::string>
	{
		vector<std::string> result;

		try
		{
			auto const system = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
			std::ranges::transform(system->GetUnits(), std::back_inserter(result),
				[](auto const& unit) { return std::visit([](auto const& u) { return u.GetName(); }, unit); });
		}
		catch (...)
		{
		}

		return result;
	}

	auto Utilities::GetBothHorizontalSystemsAreIdentical(std::string const& wktA, std::string const& wktB) const -> bool
	{
		try
		{
			auto const coordinateSystemA = GetCoordinateSystemFactory()->CreateFromWkt(wktA);
			auto const coordinateSystemB = GetCoordinateSystemFactory()->CreateFromWkt(wktB);

			auto [horizontalA, verticalA] = coordinateSystemA->ExtractHorizontalVertical();
			auto [horizontalB, verticalB] = coordinateSystemB->ExtractHorizontalVertical();

			if (nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(horizontalA) &&
				nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(horizontalB))
			{
				return true;
			}

			if (!horizontalA->GetAuthority().empty() &&
				horizontalA->GetAuthorityCode() &&
				horizontalA->GetAuthority() == horizontalB->GetAuthority() &&
				horizontalA->GetAuthorityCode() == horizontalB->GetAuthorityCode())
				return true;

			auto const transformation = StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(
				horizontalA,
				horizontalB);

			return transformation->GetMathTransform()->GetIsIdentity();
		}
		catch (...)
		{
			return false;
		}
	}

	auto Utilities::GetEpsgCodeFromWkt(std::string const& wkt) const -> int
	{
		try
		{
			auto const coordinateSystem = GetCoordinateSystemFactory()->CreateFromWkt(wkt);

			if (compareNoCase(coordinateSystem->GetAuthority().c_str(), "EPSG"))
				return 0;

			return coordinateSystem->GetAuthorityCode();
		}
		catch (...)
		{
			return 0;
		}
	}

	std::string Utilities::LoadPrjFileForFileOrThrow(std::string const& filePath) const
	{
		auto const prjFileName = std::filesystem::path(filePath).replace_extension(".prj").string();

		if (std::ifstream isPrj{std::filesystem::path(prjFileName)}; isPrj.is_open()) {
			std::string wkt;
			std::getline(isPrj, wkt);
			return wkt;
		}

		throw runtime_error(std::format("Unable to open the file: {}", prjFileName.c_str()).c_str());
	}

	std::string Utilities::LoadPrjFileForFileOrLocalSystem(std::string const& filePath) const
	{
		auto const prjFileName = std::filesystem::path(filePath).replace_extension(".prj").string();

		if (std::ifstream isPrj{std::filesystem::path(prjFileName)}; isPrj.is_open()) {
			std::string wkt;
			std::getline(isPrj, wkt);
			return wkt;
		}

		return "COMPD_CS[\"Local or unknown horizontal coordinate system + Local or unknown vertical coordinate system\",LOCAL_CS[\"Local or unknown horizontal coordinate system\", LOCAL_DATUM[\"Local or unknown horizontal datum\", 32767],UNIT[\"unknown\",1.0000000000],AXIS[\"X\", EAST],AXIS[\"Y\",NORTH]],LOCAL_CS[\"Local or unknown vertical coordinate system\", LOCAL_DATUM[\"Local or unknown vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]";
	}

	void Utilities::CreatePrjFileForFileOrThrow(std::string const& filePath, std::string const& wkt) const
	{
		std::filesystem::path prjFilePath = std::filesystem::path(filePath).replace_extension(".prj");

		// Create the .prj file
		// If it cannot be created, we throw an exception
		std::ofstream os{std::filesystem::path(prjFilePath)};
		if (!os.is_open())
			throw runtime_error(std::format("Unable to open the file: {}", prjFilePath.string()).c_str());

		os << wkt;
		os.close();
	}

	auto Utilities::ExtractHorizontalVertical(std::shared_ptr<CoordinateSystem> const& scr) const -> pair<std::shared_ptr<CoordinateSystem>, std::shared_ptr<CoordinateSystem>>
	{
		std::shared_ptr<CoordinateSystem> horizontal;
		std::shared_ptr<CoordinateSystem> vertical;

		auto const compound = dynamic_pointer_cast<CompoundCoordinateSystem>(scr);
		if (nullptr == compound)
		{
			if (scr->GetDimension() == 3)
			{
				if (nullptr != dynamic_pointer_cast<GeocentricCoordinateSystem>(scr))
					return { scr, scr };

				horizontal = CoordinateSystemFactory::Create2DSystemFor3DSystem(scr);
				vertical = scr;
				return { horizontal, vertical };
			}

			if (nullptr != dynamic_pointer_cast<HorizontalCoordinateSystem>(scr))
				horizontal = scr;
			else if (nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(scr))
				vertical = scr;
			else if (nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(scr))
			{
				vertical = horizontal = scr;
			}
			return { horizontal, vertical };
		}

		if (nullptr != dynamic_pointer_cast<HorizontalCoordinateSystem>(compound->GetHeadCS()) ||
			nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(compound->GetHeadCS()))
			horizontal = dynamic_pointer_cast<CoordinateSystem>(compound->GetHeadCS());

		if (nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(compound->GetTailCS()) ||
			nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(compound->GetTailCS()))
			vertical = dynamic_pointer_cast<CoordinateSystem>(compound->GetTailCS());

		return { horizontal, vertical };
	}

	auto Utilities::ExtractHorizontalVertical(std::string const& wkt) const -> std::pair<std::string, std::string>
	{
		auto [horizontal, vertical] = ExtractHorizontalVertical(GetCoordinateSystemFactory()->CreateFromWkt(wkt));

		return {
			horizontal ? horizontal->GetWkt() : GetUnknownHorizontalWkt(),
			vertical ? vertical->GetWkt() : GetUnknownVerticalWkt()
		};
	}

	auto Utilities::CreateTransformToGeocentric(
		std::shared_ptr<CoordinateSystem> const& horizontal,
		std::shared_ptr<CoordinateSystem> const& vertical,
		std::vector<std::shared_ptr<IMathTransform>>& direct,
		std::vector<std::shared_ptr<IMathTransform>>& inverse,
		DelegateCreateVerticalTransformation const& createVerticalSystemDialogCallback,
		DelegateDetectedMultipleCoordinateOperations const& selectTransformationCallback) const -> void
	{
		direct.clear();
		inverse.clear();

		// For a geocentric coordinate reference system, direct converts X, Y and Z from its linear unit to metres and inverse
		// converts them from metres to its linear unit, each with one affine step; vertical is not used.
		if (auto const geocentricSystem = dynamic_pointer_cast<GeocentricCoordinateSystem>(horizontal))
		{
			direct.push_back(Affine::AffineMainDiagonalTransform(geocentricSystem->GetLinearUnit().GetMetersPerUnit(), 3));
			inverse.push_back(Affine::AffineMainDiagonalTransform(1.0 / geocentricSystem->GetLinearUnit().GetMetersPerUnit(), 3));
			return;
		}

		if (nullptr == dynamic_pointer_cast<GeographicCoordinateSystem>(horizontal) && nullptr == dynamic_pointer_cast<ProjectedCoordinateSystem>(horizontal))
			throw TransformationNotFoundException(std::format("No transformation to geocentric coordinates has been found for the horizontal coordinate system {}: it is not a geographic, projected or geocentric coordinate system.",
				horizontal ? horizontal->GetName() : std::string{ "null" }));

		std::vector<std::shared_ptr<IMathTransform>> horizontalTransformations;

		// We create a transformation to convert the system to geocentric coordinates
		shared_ptr<GeographicCoordinateSystem> geographicSystem;
		if (nullptr != dynamic_pointer_cast<GeographicCoordinateSystem>(horizontal))
		{
			geographicSystem = dynamic_pointer_cast<GeographicCoordinateSystem>(horizontal);
		}
		else
		{
			auto const source = dynamic_pointer_cast<ProjectedCoordinateSystem>(horizontal);

			horizontalTransformations.push_back(Affine::TransformXyzToZxy());
			horizontalTransformations.push_back(make_shared<PassThrough>(1, StaticCoordinateTransformationFactory::CreateProjectionTransforms(source)->GetInverse()));
			horizontalTransformations.push_back(Affine::TransformXyzToZxy()->GetInverse());
			geographicSystem = dynamic_pointer_cast<GeographicCoordinateSystem>(source->GetGeographicCoordinateSystem());
		}

		// GeographicToGeocentric receives longitude and latitude in degrees and the ellipsoidal height in metres.
		std::ranges::copy(StaticCoordinateTransformationFactory::CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(geographicSystem, 3), std::back_inserter(horizontalTransformations));

		std::ranges::copy(horizontalTransformations, std::back_inserter(direct));

		// GeographicToGeocentric receives the longitude from Greenwich; applying the steps in horizontalTransformations gives
		// the longitude from the prime meridian of geographicSystem.
		auto const primeMeridianToGreenwichLongitudeConversion = StaticCoordinateTransformationFactory::CreatePrimeMeridianToGreenwichLongitudeConversion(*geographicSystem, 3);

		if (geographicSystem->GetDimension() == 3)
		{
			auto const geographicToGeocentric = make_shared<GeographicToGeocentric>(geographicSystem->GetHorizontalDatum()->GetEllipsoid(), false);
			if (primeMeridianToGreenwichLongitudeConversion)
				direct.push_back(primeMeridianToGreenwichLongitudeConversion);
			direct.push_back(geographicToGeocentric);

			inverse.push_back(geographicToGeocentric->GetInverse());
			if (primeMeridianToGreenwichLongitudeConversion)
				inverse.push_back(primeMeridianToGreenwichLongitudeConversion->GetInverse());
		}
		else
		{
			// The height conversion steps receive and return longitude and latitude in degrees and a height. The direct
			// chain converts the orthometric height to ellipsoidal height and the inverse chain converts the ellipsoidal
			// height to orthometric height. With a geographic 3D CRS, the factory builds the conversion of each chain in the
			// direction of that chain.
			std::vector<std::shared_ptr<IMathTransform>> orthometricToEllipsoidalHeightSteps;
			std::vector<std::shared_ptr<IMathTransform>> ellipsoidalToOrthometricHeightSteps;

			// Returns the steps of heightTransformation, the transformation returned by createVerticalSystemDialogCallback or its
			// inverse. The steps receive and return the longitude from the prime meridian of geographicSystem; heightTransformation
			// receives the longitude from Greenwich.
			auto const createDialogHeightSteps = [&primeMeridianToGreenwichLongitudeConversion](std::shared_ptr<IMathTransform> const& heightTransformation)
			{
				std::vector<std::shared_ptr<IMathTransform>> steps;
				if (primeMeridianToGreenwichLongitudeConversion)
					steps.push_back(primeMeridianToGreenwichLongitudeConversion);
				std::ranges::copy(CreateThreeCoordinateHeightSteps(heightTransformation), std::back_inserter(steps));
				if (primeMeridianToGreenwichLongitudeConversion)
					steps.push_back(primeMeridianToGreenwichLongitudeConversion->GetInverse());
				return steps;
			};
			if (nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(vertical))
			{
				// Vertical transformations only exist between geographic 3D and vertical systems or between two vertical systems.
				auto const geographicSystem3D = CoordinateSystemFactory::FindGeographicSystem3D(geographicSystem);

				if (nullptr == geographicSystem3D)
				{
					// The dialog creates the transformation from the geographic CRS to the vertical CRS.
					auto const selectedVerticalTransformation = createVerticalSystemDialogCallback(geographicSystem->GetName(), vertical->GetName());
					if (!selectedVerticalTransformation)
						throw runtime_error("The user has not selected a vertical coordinate system.");

					orthometricToEllipsoidalHeightSteps = createDialogHeightSteps(selectedVerticalTransformation->GetInverse());
					ellipsoidalToOrthometricHeightSteps = createDialogHeightSteps(selectedVerticalTransformation);
				}
				else
				{
					// The factory asks for the operation once per direction. The second call reuses the selected code when that code
					// is among the operations offered.
					auto const selectedOperationCode = make_shared<optional<int>>();
					auto const selectOperationReusingTheSelectedCode = [selectTransformationCallback, selectedOperationCode](std::string const& sourceName, std::string const& targetName, std::vector<CoordinateOperation> const& operations) -> int
					{
						if (*selectedOperationCode && std::ranges::any_of(operations, [&](CoordinateOperation const& operation) { return operation.Code == **selectedOperationCode; }))
							return **selectedOperationCode;

						*selectedOperationCode = selectTransformationCallback(sourceName, targetName, operations);
						return **selectedOperationCode;
					};

					CoordinateTransformationOptions options;
					if (selectTransformationCallback)
						options.selectOperation = selectOperationReusingTheSelectedCode;

					try
					{
						// The factory builds the transformation from the vertical CRS to geographicSystem3D and the transformation from
						// geographicSystem3D to the vertical CRS. Both transformations receive and return three coordinates in the axes
						// of geographicSystem3D.
						orthometricToEllipsoidalHeightSteps = CreateStepsWithLongitudeLatitudeInDegreesAndHeight(geographicSystem3D,
							StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(vertical, geographicSystem3D, options)->GetMathTransform());
						ellipsoidalToOrthometricHeightSteps = CreateStepsWithLongitudeLatitudeInDegreesAndHeight(geographicSystem3D,
							StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(geographicSystem3D, vertical, options)->GetMathTransform());
					}
					catch (TransformationNotFoundException const&)
					{
						// If createVerticalSystemDialogCallback is set and returns a non-null transformation from the geographic CRS to the
						// vertical CRS, createDialogHeightSteps adapts that transformation and its inverse to steps that receive and return
						// longitude and latitude in degrees and a height.
						auto const selectedVerticalTransformation = createVerticalSystemDialogCallback ? createVerticalSystemDialogCallback(geographicSystem3D->GetName(), vertical->GetName()) : nullptr;
						if (!selectedVerticalTransformation)
							throw;

						orthometricToEllipsoidalHeightSteps = createDialogHeightSteps(selectedVerticalTransformation->GetInverse());
						ellipsoidalToOrthometricHeightSteps = createDialogHeightSteps(selectedVerticalTransformation);
					}
				}
			}

			std::ranges::copy(orthometricToEllipsoidalHeightSteps, std::back_inserter(direct));

			auto const geographicToGeocentric = make_shared<GeographicToGeocentric>(geographicSystem->GetHorizontalDatum()->GetEllipsoid(), false);
			if (primeMeridianToGreenwichLongitudeConversion)
				direct.push_back(primeMeridianToGreenwichLongitudeConversion);
			direct.push_back(geographicToGeocentric);

			inverse.push_back(geographicToGeocentric->GetInverse());
			if (primeMeridianToGreenwichLongitudeConversion)
				inverse.push_back(primeMeridianToGreenwichLongitudeConversion->GetInverse());
			std::ranges::copy(ellipsoidalToOrthometricHeightSteps, std::back_inserter(inverse));
		}

		// The inverse chain returns the coordinates in the axes and units of the horizontal coordinate reference system.
		for (auto const& transformation : horizontalTransformations | std::views::reverse)
			inverse.push_back(transformation->GetInverse());
	}


	auto Utilities::ExtractUtmParametersFromWkt(std::string const& coordinateSystemWkt, double& semiMinorAxis, double& semiMajorAxis, double& latitudeOfNaturalOrigin, double& longitudeOfNaturalOrigin, double& scaleFactorAtNaturalOrigin, double& falseEasting, double& falseNorthing) const -> bool
	{
		try
		{
			auto [horizontalWkt, verticalWkt] = ExtractHorizontalVertical(coordinateSystemWkt);

			auto const sc = GetCoordinateSystemFactory()->CreateFromWkt(horizontalWkt);
			auto const projected = dynamic_pointer_cast<ProjectedCoordinateSystem>(sc);
			if (!projected)
				return false;

			semiMinorAxis = projected->GetGeographicCoordinateSystem()->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis();
			semiMajorAxis = projected->GetGeographicCoordinateSystem()->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis();
			latitudeOfNaturalOrigin = get<double>(projected->GetProjection()->GetParameter("latitude_of_origin").GetValue());
			longitudeOfNaturalOrigin = get<double>(projected->GetProjection()->GetParameter("central_meridian").GetValue());
			scaleFactorAtNaturalOrigin = get<double>(projected->GetProjection()->GetParameter("scale_factor").GetValue());
			falseEasting = get<double>(projected->GetProjection()->GetParameter("false_easting").GetValue());
			falseNorthing = get<double>(projected->GetProjection()->GetParameter("false_northing").GetValue());
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	auto Utilities::CreateAffineMainDiagonalTransform(int const sideSize) const -> std::shared_ptr<IMathTransform>
	{
		return Affine::AffineMainDiagonalTransform(1.0, sideSize);
	}

	auto Utilities::CreateConcatenatedTransform(std::vector<std::shared_ptr<IMathTransform>> const& transforms) const -> std::shared_ptr<IMathTransform>
	{
		return make_shared<ConcatenatedTransform>(transforms);
	}

	auto Utilities::CreateGeocentricToTopocentric(double semiMajorAxis, double semiMinorAxis, double geocentricXOfTopocentricOrigin, double geocentricYOfTopocentricOrigin, double geocentricZOfTopocentricOrigin, bool inverse) const -> std::shared_ptr<IMathTransform>
	{
		return make_shared<GeocentricToTopocentric>(semiMajorAxis, semiMinorAxis, geocentricXOfTopocentricOrigin, geocentricYOfTopocentricOrigin, geocentricZOfTopocentricOrigin, inverse);
	}

	auto Utilities::CreateGeographicToGeocentric(double semiMajorAxis, double semiMinorAxis, bool inverse) const -> std::shared_ptr<IMathTransform>
	{
		return make_shared<GeographicToGeocentric>(semiMajorAxis, semiMinorAxis, inverse);
	}

	auto Utilities::CreateObliqueStereographic(double semiMajorAxis, double semiMinorAxis, double latitudeOfOrigin, double centralMeridian, double scaleFactor, double falseEasting, double falseNorthing, bool inverse) const->std::shared_ptr<IMathTransform>
	{
		return make_shared<MapProjections::ObliqueStereographic>(semiMajorAxis, semiMinorAxis, latitudeOfOrigin, centralMeridian, scaleFactor, falseEasting, falseNorthing, inverse);
	}


	static auto DmsToDecimalDegrees(unsigned int const degrees, unsigned int const minutes, double const seconds, bool const positive, double& sexagesimalDegrees) -> void
	{
		if (positive)
		{
			sexagesimalDegrees = degrees;
			sexagesimalDegrees += minutes / 60.0;
			sexagesimalDegrees += seconds / 3600.0;
		}
		else
		{
			sexagesimalDegrees = -static_cast<int>(degrees);
			sexagesimalDegrees -= minutes / 60.0;
			sexagesimalDegrees -= seconds / 3600.0;
		}
	}

	auto Utilities::ToSexagesimal(int const angularUnitCode, double const factorToStandardUnit, double value) -> double
	{
		if (MEASURE_UNIT_DEGREES == angularUnitCode)
			return value;

		if (ANGULAR_UNIT_SEXAGESIMAL_DMS == angularUnitCode)
		{
			// DDD.MMSSssssss: decimal digits stored in a binary double (-82.1 arrives as -82.0999...), so the value is
			// rounded to 10^-10, the last digit the EPSG dataset uses, before it is split into fields.
			auto const scaled = std::llround(std::abs(value) * 1e10);
			auto const degrees = static_cast<double>(scaled / 10'000'000'000LL);
			auto const minutes = static_cast<double>(scaled / 100'000'000LL % 100);
			auto const seconds = static_cast<double>(scaled % 100'000'000LL) / 1e6;
			auto const decimalDegrees = degrees + minutes / 60.0 + seconds / 3600.0;
			return value < 0.0 ? -decimalDegrees : decimalDegrees;
		}
		return RADIANS_TO_DEGREES(factorToStandardUnit * value);
	}

}
