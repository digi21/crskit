#include "pch.h"
using namespace CrsKit::Math;
#include "Utilities.h"

#include "CoordinateSystems/CoordinateSystemAuthorityFactory.h"
#include "CoordinateTransformations/ConcatenatedTransform.h"
#include "CoordinateTransformations/CoordinateTransformationFactory.h"
#include "CoordinateTransformations/Algorithms/Afine.h"
#include "CoordinateTransformations/Passthrough.h"
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
		try
		{
			return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code)->GetName();
		}
		catch (exception& ex)
		{
			return std::string(ex.what());
		}
	}

	auto Utilities::GetCoordinateReferenceSystemName(std::string const& wkt) const -> std::string
	{
		return GetCoordinateSystemFactory()->CreateFromWkt(wkt)->GetName();
	}

	auto Utilities::GetWktFromEpsgCode(int const code) const -> std::string
	{
		try
		{
			return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code)->GetWkt();
		}
		catch (exception& ex)
		{
			return std::string(ex.what());
		}
	}

	auto Utilities::GetWktFromProjectionCodeForGeographic(int const projectionCode, int const geographicCode) const -> std::string
	{
		return GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(projectionCode, geographicCode)->GetWkt();
	}


	auto Utilities::GetWktFromGeographic3DSystemForProjected(std::string const& wkt) const->std::string
	{
		try {
			auto const sc = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
			auto const projected = dynamic_pointer_cast<ProjectedCoordinateSystem>(sc);
			if (!projected)
				return std::format("The {} coordinate system is not projected.", sc->GetName().c_str());

			auto const geographic = projected->GetGeographicCoordinateSystem();
			return geographic->GetWkt();
		}
		catch (exception& exception)
		{
			return std::string(exception.what());
		}
	}

	auto Utilities::GetWktFromGeographicEpsgCode(int const epsgCode, bool const longitudeLatitude) const -> std::string
	{
		try
		{
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
		catch (exception& exception)
		{
			return std::string(exception.what());
		}
	}

	auto Utilities::GetWktFromEpsgCodeWithEastNorthAxes(int const epsgCode) const -> std::string
	{
		auto coordinateSystem = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(epsgCode);
		coordinateSystem = CoordinateSystemFactory::WithAxesEastNorth(coordinateSystem);
		return coordinateSystem->GetWkt();
	}

	auto Utilities::GetWktFromCrsCodeWithEastNorthAxes(std::string const& crsCode) const->std::string
	{
		if (crsCode.substr(0, 5) == "EPSG:") {
			auto const epsgCode = std::stoi(crsCode.substr(crsCode.size(), crsCode.size() - 5));
			return GetWktFromEpsgCodeWithEastNorthAxes(epsgCode);
		}

		if (crsCode.substr(0, 4) == "CRS:") {
			auto const crsNumber = std::stoi(crsCode.substr(crsCode.size(), crsCode.size() - 4));

			if (84 == crsNumber)
				return GetWktFromEpsgCodeWithEastNorthAxes(4326);
			if (83 == crsNumber)
				return GetWktFromEpsgCodeWithEastNorthAxes(4269);
			if (27 == crsNumber)
				return GetWktFromEpsgCodeWithEastNorthAxes(4267);
		}

		throw runtime_error{ std::format("The coordinate reference system: {} is not recognized", crsCode.c_str()).c_str() };
	}

	auto Utilities::GetWktWithEastNorthAxes(std::string const& wkt) const -> std::string
	{
		try
		{
			auto coordinateSystem = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
			coordinateSystem = CoordinateSystemFactory::WithAxesEastNorth(coordinateSystem);
			return coordinateSystem->GetWkt();
		}
		catch (exception& exception)
		{
			return std::string(exception.what());
		}
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

		// Creamos el file .prj
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

		// Mathematical operations expect (Longitude, Latitude) and not (Latitude, Longitude)
		if (geographicSystem->GetAxis(0).GetName() == "Lat" && (geographicSystem->GetAxis(1).GetName() == "Long" || geographicSystem->GetAxis(1).GetName() == "Lon"))
			horizontalTransformations.push_back(Affine::TransformXyzToYxz());

		// The units of the transformations are always sexagesimal
		if (fabs(geographicSystem->GetAngularUnit().GetRadiansPerUnit() - M_PI / 180.0) > 1E-15)
			horizontalTransformations.push_back(Affine::AffineMainDiagonalTransform(geographicSystem->GetAngularUnit().GetRadiansPerUnit() * 180 / M_PI, 2));

		std::ranges::copy(horizontalTransformations, std::back_inserter(direct));

		if (geographicSystem->GetDimension() == 3)
		{
			auto const geographicToGeocentric = make_shared<GeographicToGeocentric>(geographicSystem->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), geographicSystem->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis(), false);
			direct.push_back(geographicToGeocentric);

			// We compute the inverse
			inverse.push_back(geographicToGeocentric->GetInverse());
		}
		else
		{
			shared_ptr<IMathTransform> verticalTransformation;
			if (nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(vertical))
			{
				// Now I have to apply the vertical transformation to convert the orthometric Z coordinate into an ellipsoidal one

				// Vertical transformations only exist between Geographic 3D and Vertical systems or between Vertical and Vertical. Here we locate the geographic 3D system associated with the geographic
				// that we have:
				auto const geographicSystem3D = CoordinateSystemFactory::FindGeographicSystem3D(geographicSystem);

				if (nullptr == geographicSystem3D)
				{
					auto const selectedVerticalTransformation = createVerticalSystemDialogCallback(geographicSystem->GetName(), vertical->GetName());
					if (!selectedVerticalTransformation)
						throw runtime_error("The user has not selected a vertical coordinate system.");

					verticalTransformation = selectedVerticalTransformation->GetInverse();
				}
				else
				{
					verticalTransformation = StaticCoordinateTransformationFactory::CreateFromCoordinateSystems(
						geographicSystem3D,
						vertical,
						CoordinateTransformationOptions{ .resolveTransform = [createVerticalSystemDialogCallback](std::shared_ptr<CoordinateSystem> const& s, std::shared_ptr<CoordinateSystem> const& t) { return createVerticalSystemDialogCallback(s->GetName(), t->GetName()); }, .selectOperation = selectTransformationCallback })->GetMathTransform()->GetInverse();
				}

				if (1 == verticalTransformation->GetSourceDimension())
				{
					direct.push_back(make_shared<PassThrough>(2, verticalTransformation));
				}
				else
				{
					if (1 == verticalTransformation->GetTargetDimension())
					{
						// The transformation receives XYZ and returns Z
						// but what we must return is XYZ, so I do the following:
						// XYZ -> XYXYZ
						// PASSTHROUGH(2, transformation)
						direct.push_back(Affine::TransformXyzToXyXyz());
						direct.push_back(make_shared<PassThrough>(2, verticalTransformation));
					}
					else
					{
						direct.push_back(verticalTransformation);
					}
				}
			}

			auto const geographicToGeocentric = make_shared<GeographicToGeocentric>(geographicSystem->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), geographicSystem->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis(), false);
			direct.push_back(geographicToGeocentric);

			// We compute the inverse
			inverse.push_back(geographicToGeocentric->GetInverse());

			if (nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(vertical))
			{
				// Tenemos Longitude, Latitude, h
				if (1 == verticalTransformation->GetSourceDimension())
				{
					inverse.push_back(make_shared<PassThrough>(2, verticalTransformation->GetInverse()));
				}
				else
				{
					if (1 == verticalTransformation->GetTargetDimension())
					{
						// The transformation receives Longitude, Latitude, Z and returns Z
						// but what we must return is Longitude, Latitude, Z, so I do the following:
						// XYZ -> XYXYZ
						// PASSTHROUGH(2, transformation)
						inverse.push_back(Affine::TransformXyzToXyXyz());
						inverse.push_back(make_shared<PassThrough>(2, verticalTransformation->GetInverse()));
					}
					else
					{
						inverse.push_back(verticalTransformation->GetInverse());
					}
				}
			}

			for (auto const& transformation : horizontalTransformations | std::views::reverse)
				inverse.push_back(transformation->GetInverse());
		}
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
		if (MEASURE_UNIT_DEGREES == value)
			return value;

		if (ANGULAR_UNIT_SEXAGESIMAL_DMS == angularUnitCode)
		{
			// This is a dangerous case due to rounding problems. A character string was stored in a float (which the .NET framework reads as a double, if 
			// we tried to read the value from the database as a float, an exception would be thrown).
			//
			// The rounding problem is that, for example, in coordinate operation 18086 the value 46.48 was stored, but it reaches me as 46.4799999999999 and this is a problem
			// because if the algorithm is followed I get 46º47'99.999999999968736" which when converted to decimal degrees yields 46.811111111111103
			// which is not the same as 46º48'00" which in decimal degrees is 46.80
			//
			// This makes, for example, test 5102_part_1 not fail when instantiating the GIGS objects but fail when instantiating from the EPSG database (the only case where
			// we will enter this block of code, since only the EPSG database defines angular values of type ANGULAR_UNIT_SEXAGESIMAL_DMS).
			//
			// I found two solutions: print the string with %.16g and read the values from that string, or add a sigma 0.00000000000001 that gives us 46º48'3.9790393202565610e-011" which when converted to decimal degrees yields 46.800000000000011
			// The one I like most is the text-based one, so I have commented out the original solution (the + 0.00000000000001 is kept so the tests do not fail).
			auto text = std::format("{:.16g}", value);

			auto pointPos = text.find('.');
			if (std::string::npos == pointPos) {
				text += ".0";
				pointPos = text.find('.');
			}
			text += "0000000000";

			auto const degrees = abs(static_cast<int>(value));

			text = text.substr(pointPos + 1);

			auto const minutes = std::stoi(text.substr(0, 2));

			text = text.substr(2);

			auto const temp = std::format("{}.{}", text.substr(0, 2).c_str(), text.substr(2).c_str());
			auto const seconds = std::stod(temp);

			double sexagesimalDegrees;
			DmsToDecimalDegrees(degrees, minutes, seconds, value > 0.0, sexagesimalDegrees);
			return sexagesimalDegrees;
		}

		return RADIANS_TO_DEGREES(factorToStandardUnit * value);
	}

}
