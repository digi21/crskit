#pragma once

// Native port of GigsDataset/GigsFactory (the C# GIGS tests project).
// Builds coordinate systems from the GIGS reference data, using the public
// Digi21.OpenGis API.
//
// Data ported from GIGSDataSet.cs (units, ellipsoids, meridians, datums,
// geodeticCRS, userProjectedCRS and projections). The collections with EPSG
// library components (datumLibraryComponents, etc.) and the vertical data are
// still missing; they will be ported when those tests are tackled.

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "../crskit/CrsKit.h"

namespace Gigs
{
	using namespace CrsKit;
	using namespace CrsKit::CoordinateSystems;
	using namespace CrsKit::CoordinateTransformations;

	inline constexpr double kPi = 3.14159265358979323846;

	struct LinearUnitData { int Code; const char* Name; double MetersPerUnit; };
	struct AngularUnitData { int Code; const char* Name; double RadiansPerUnit; };
	struct EllipsoidData { int Code; const char* Name; double SemiMajorAxis; std::optional<double> SemiMinorAxis; std::optional<double> InverseFlattening; int Unit; };
	struct PrimeMeridianData { int Code; const char* Name; double LongitudeFromGreenwich; int Unit; };
	struct DatumData { int Code; const char* Name; const char* EllipsoidName; const char* PrimeMeridianName; };
	struct GeodeticCrsData { int Code; const char* Name; int DatumCode; bool AxesChanged; };
	struct ProjectionParam { const char* Name; double Value; int Unit; };
	struct ProjectionData { int Code; const char* OgsName; ProjectionParam P[7]; };
	struct ProjectedCrsData
	{
		int Code; const char* Name; int BaseCrsCode; int ProjCode;
		const char* CsAxis1Abbreviation; AxisOrientationEnum CsAxis1Orientation; int CsAxis1Unit;
		const char* CsAxis2Abbreviation; AxisOrientationEnum CsAxis2Orientation; int CsAxis2Unit;
	};
	struct VerticalDatumData { int Code; const char* Name; };
	struct VerticalCrsData { int Code; const char* Name; int GigsDatumCode; const char* CsAxis1Abbreviation; AxisOrientationEnum CsAxis1Orientation; int CsAxis1Unit; };

	inline const std::vector<LinearUnitData> linearUnits = {
		{69001, "GIGS unit L0 (metre)", 1.0},
		{69036, "GIGS unit L1 (kilometre)", 1000.0},
		{69002, "GIGS unit L2 (foot)", 0.3048},
		{69003, "GIGS unit L3 (US survey foot)", 0.30480060960121920243840487680975},
		{69031, "GIGS unit L4 (German legal metre)", 1.000013597},
		{69005, "GIGS unit L5 (Clarke's foot)", 0.304797265},
		{69039, "GIGS unit L6 (Clarke's link)", 0.201166195},
		{69042, "GIGS unit L7 (British chain (Sears 1922))", 20.11676512},
		{69041, "GIGS unit L8 (British foot (Sears 1922))", 0.304799472},
		{69040, "GIGS unit L9 (British yard (Sears 1922))", 0.914398415},
		{69301, "GIGS unit L10 (British chain (Sears 1922 truncated))", 20.116756},
		{69084, "GIGS unit L11 (Indian yard )", 0.914398531},
		{69094, "GIGS unit L12 (Gold Coast foot)", 0.30479971},
		{69098, "GIGS unit L13 (link)", 0.201168},
	};

	inline const std::vector<AngularUnitData> angularUnits = {
		{69101, "GIGS unit A0 (radian)", 1.0},
		{69109, "GIGS unit A1 (microradian)", 0.000001},
		{69102, "GIGS unit A2 (degree)", 0.01745329251994327777777777777778},
		{69104, "GIGS unit A3 (arc-second)", 4.85E-06},
		{69105, "GIGS unit A4 (grad)", 0.01570796326794895},
		{69113, "GIGS unit A5 (centesimal second)", 1.57E-06},
	};

	inline const std::vector<EllipsoidData> ellipsoids = {
		{67030, "GIGS ellipsoid A", 6378137.000, std::nullopt, 298.257223563, 69001},
		{67001, "GIGS ellipsoid B", 6377563.396, std::nullopt, 299.3249646, 69001},
		{67004, "GIGS ellipsoid C", 6377397.155, std::nullopt, 299.1528128, 69001},
		{67022, "GIGS ellipsoid E", 6378388.000, std::nullopt, 297, 69001},
		{67019, "GIGS ellipsoid F", 6378.137000, std::nullopt, 298.257222101, 69036},
		{67011, "GIGS ellipsoid H", 6378249.200, 6356515, std::nullopt, 69001},
		{67052, "GIGS ellipsoid I", 6370997.000, 6370997.000, std::nullopt, 69001},
		{67008, "GIGS ellipsoid J", 20925832.164, std::nullopt, 294.978698214, 69003},
		{67036, "GIGS ellipsoid K", 6378160.000, std::nullopt, 298.247167427, 69001},
		{67003, "GIGS ellipsoid X", 6378160.000, std::nullopt, 298.25, 69001},
		{67024, "GIGS ellipsoid Y", 6378245.000, std::nullopt, 298.3, 69001},
	};

	inline const std::vector<PrimeMeridianData> primeMeridians = {
		{68901, "GIGS PM A", 0.0, 69102},
		{68908, "GIGS PM D", 106.807719444444, 69102},
		{68903, "GIGS PM H", 2.33722917, 69105},
		{68904, "GIGS PM I", -74.08091666667, 69102},
	};

	inline const std::vector<DatumData> datums = {
		{66001, "GIGS geodetic datum A", "GIGS ellipsoid A", "GIGS PM A"},
		{66002, "GIGS geodetic datum B", "GIGS ellipsoid B", "GIGS PM A"},
		{66003, "GIGS geodetic datum C", "GIGS ellipsoid C", "GIGS PM A"},
		{66004, "GIGS geodetic datum D", "GIGS ellipsoid C", "GIGS PM D"},
		{66005, "GIGS geodetic datum E", "GIGS ellipsoid E", "GIGS PM A"},
		{66006, "GIGS geodetic datum F", "GIGS ellipsoid F", "GIGS PM A"},
		{66007, "GIGS geodetic datum G", "GIGS ellipsoid F", "GIGS PM A"},
		{66008, "GIGS geodetic datum H", "GIGS ellipsoid H", "GIGS PM H"},
		{66009, "GIGS geodetic datum J", "GIGS ellipsoid J", "GIGS PM A"},
		{66012, "GIGS geodetic datum K", "GIGS ellipsoid K", "GIGS PM A"},
		{66011, "GIGS geodetic datum ", "GIGS ellipsoid C", "GIGS PM A"},
		{66016, "GIGS geodetic datum M", "GIGS ellipsoid E", "GIGS PM A"},
		{66010, "GIGS geodetic datum T", "GIGS ellipsoid H", "GIGS PM A"},
		{66013, "GIGS geodetic datum X", "GIGS ellipsoid X", "GIGS PM A"},
		{66014, "GIGS geodetic datum Y", "GIGS ellipsoid Y", "GIGS PM A"},
		{66015, "GIGS geodetic datum Z", "GIGS ellipsoid F", "GIGS PM A"},
	};

	inline const std::vector<GeodeticCrsData> geodeticCRS = {
		{64003, "GIGS geogCRS A", 66001, false},
		{64004, "GIGS geogCRS Alonlat", 66001, true},
		{64005, "GIGS geogCRS B", 66002, false},
		{64006, "GIGS geogCRS C", 66003, false},
		{64007, "GIGS geogCRS D", 66004, false},
		{64008, "GIGS geogCRS E", 66005, false},
		{64009, "GIGS geogCRS F", 66006, false},
		{64010, "GIGS geogCRS G", 66007, false},
		{64011, "GIGS geogCRS H", 66008, false},
		{64012, "GIGS geogCRS J", 66009, false},
		{64015, "GIGS geogCRS K", 66012, false},
		{64014, "GIGS geogCRS ", 66011, false},
		{64020, "GIGS geogCRS M", 66016, false},
		{64013, "GIGS geogCRS T", 66010, false},
		{64016, "GIGS geogCRS X", 66013, false},
		{64017, "GIGS geogCRS Y", 66014, false},
		{64018, "GIGS geogCRS Z", 66015, false},
	};

	inline const std::vector<ProjectedCrsData> userProjectedCRS = {
		{62001, "GIGS projCRS A1", 64003, 65001, "N", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62002, "GIGS projCRS A1-2", 64003, 65001, "N", AxisOrientationEnum::North, 69001, "E", AxisOrientationEnum::East, 69001},
		{62003, "GIGS projCRS A1-3", 64003, 65001, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62004, "GIGS projCRS A1-4", 64003, 65001, "Y", AxisOrientationEnum::North, 69001, "X", AxisOrientationEnum::East, 69001},
		{62005, "GIGS projCRS A1-5", 64003, 65001, "Y", AxisOrientationEnum::East, 69001, "X", AxisOrientationEnum::North, 69001},
		{62006, "GIGS projCRS A1-6", 64003, 65001, "X", AxisOrientationEnum::North, 69001, "Y", AxisOrientationEnum::East, 69001},
		{62007, "GIGS projCRS A2", 64003, 65002, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62008, "GIGS projCRS A21", 64003, 65021, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62027, "GIGS projCRS A23", 64003, 65023, "X", AxisOrientationEnum::East, 69003, "Y", AxisOrientationEnum::North, 69003},
		{62011, "GIGS projCRS C4", 64006, 65004, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62012, "GIGS projCRS D5", 64007, 65005, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62013, "GIGS projCRS E6", 64008, 65006, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62014, "GIGS projCRS F7", 64009, 65007, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62015, "GIGS projCRS F8", 64009, 65008, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62016, "GIGS projCRS F9", 64009, 65009, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62017, "GIGS projCRS G10", 64010, 65010, "Y", AxisOrientationEnum::West, 69001, "X", AxisOrientationEnum::South, 69001},
		{62018, "GIGS projCRS G11", 64010, 65011, "X", AxisOrientationEnum::North, 69001, "Y", AxisOrientationEnum::East, 69001},
		{62019, "GIGS projCRS G12", 64010, 65012, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62020, "GIGS projCRS G13", 64010, 65013, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62021, "GIGS projCRS G14", 64010, 65014, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62022, "GIGS projCRS G15", 64010, 65015, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62023, "GIGS projCRS G16", 64010, 65016, "Y", AxisOrientationEnum::North, 69001, "X", AxisOrientationEnum::East, 69001},
		{62024, "GIGS projCRS G17", 64010, 65017, "X", AxisOrientationEnum::East, 69002, "Y", AxisOrientationEnum::North, 69002},
		{62025, "GIGS projCRS G18", 64010, 65018, "X", AxisOrientationEnum::East, 69003, "Y", AxisOrientationEnum::North, 69003},
		{62026, "GIGS projCRS H19", 64011, 65019, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62038, "GIGS projCRS J28", 64012, 65028, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
		{62036, "GIGS projCRS K26", 64015, 65026, "Y", AxisOrientationEnum::East, 69001, "X", AxisOrientationEnum::North, 69001},
		{62037, "GIGS projCRS L27", 64014, 65027, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62035, "GIGS projCRS M25", 64020, 65025, "X", AxisOrientationEnum::East, 69001, "Y", AxisOrientationEnum::North, 69001},
		{62034, "GIGS projCRS Y24", 64017, 65024, "Y", AxisOrientationEnum::North, 69001, "X", AxisOrientationEnum::East, 69001},
		{62039, "GIGS projCRS Z28", 64012, 65028, "E", AxisOrientationEnum::East, 69001, "N", AxisOrientationEnum::North, 69001},
	};

	inline const std::vector<ProjectionData> projections = {
		{65001, "Transverse_Mercator", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", 3.0, 69102}, {"scale_factor", 0.9996, 69201}, {"false_easting", 500000, 69001}, {"false_northing", 0, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65002, "Transverse_Mercator", { {"latitude_of_origin", 49.0, 69102}, {"central_meridian", -2.0, 69102}, {"scale_factor", 0.999601272, 69201}, {"false_easting", 400000, 69001}, {"false_northing", -100000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65021, "Transverse_Mercator", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", -2.0, 69102}, {"scale_factor", 0.999601272, 69201}, {"false_easting", 400000, 69001}, {"false_northing", -5527462.688, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65022, "Transverse_Mercator", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", -2.0, 69102}, {"scale_factor", 0.999601272, 69201}, {"false_easting", 400000, 69001}, {"false_northing", -5527063.816, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65004, "Oblique_Stereographic", { {"latitude_of_origin", 52.1561606, 69102}, {"central_meridian", 5.3876389, 69102}, {"scale_factor", 0.9999079, 69201}, {"false_easting", 155000, 69001}, {"false_northing", 463000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65005, "Mercator_1SP", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", 3.1922806, 69102}, {"scale_factor", 0.997, 69201}, {"false_easting", 3900000, 69001}, {"false_northing", 900000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65006, "Lambert_Conformal_Conic_2SP", { {"latitude_of_origin", 90.0, 69102}, {"central_meridian", 4.367486667, 69102}, {"standard_parallel1", 51.16666723, 69102}, {"standard_parallel2", 49.8333339, 69102}, {"false_easting", 150000.013, 69001}, {"false_northing", 5400088.438, 69001}, {nullptr, 0, 0} }},
		{65007, "Transverse_Mercator", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", 141.0, 69102}, {"scale_factor", 0.9996, 69201}, {"false_easting", 500000, 69001}, {"false_northing", 10000000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65008, "Transverse_Mercator", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", 147.0, 69102}, {"scale_factor", 0.9996, 69201}, {"false_easting", 500000, 69001}, {"false_northing", 10000000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65009, "Albers_Conic_Equal_Area", { {"latitude_of_center", 0, 69102}, {"longitude_of_center", 132, 69102}, {"standard_parallel1", -18, 69102}, {"standard_parallel2", -36, 69102}, {"false_easting", 0, 69001}, {"false_northing", 0, 69001}, {nullptr, 0, 0} }},
		{65010, "Transverse_Mercator_South_Orientated", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", 21.0, 69102}, {"scale_factor", 1, 69201}, {"false_easting", 0, 69001}, {"false_northing", 0, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65011, "Transverse_Mercator", { {"latitude_of_origin", -90.0, 69102}, {"central_meridian", -60.0, 69102}, {"scale_factor", 1, 69201}, {"false_easting", 5500000, 69001}, {"false_northing", 0, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65012, "polyconic", { {"latitude_of_origin", 0, 69102}, {"central_meridian", -54, 69102}, {"false_easting", 5000000, 69001}, {"false_northing", 10000000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65013, "Oblique_Mercator", { {"latitude_of_center", 4, 69102}, {"longitude_of_center", 115, 69102}, {"azimuth", 53.31580994, 69102}, {"rectified_grid_angle", 53.13010236, 69102}, {"scale_factor", 0.99984, 69201}, {"false_easting", 590521.147, 69001}, {"false_northing", 442890.861, 69001} }},
		{65014, "Hotine_Oblique_Mercator", { {"latitude_of_center", 4, 69102}, {"longitude_of_center", 115, 69102}, {"azimuth", 53.31580994, 69102}, {"rectified_grid_angle", 53.13010236, 69102}, {"scale_factor", 0.99984, 69201}, {"false_easting", 0, 69001}, {"false_northing", 0, 69001} }},
		{65015, "Cassini_Soldner", { {"latitude_of_origin", 2.12167974, 69102}, {"central_meridian", 103.4279362361, 69102}, {"false_easting", -14810.562, 69001}, {"false_northing", 8758.32, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65016, "Lambert_azimuthal_equal_area", { {"latitude_of_center", 52, 69102}, {"longitude_of_center", 10, 69102}, {"false_easting", 4321000, 69001}, {"false_northing", 3210000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65017, "Lambert_Conformal_Conic_2SP", { {"latitude_of_origin", 40.3333333, 69102}, {"central_meridian", -111.5000000, 69102}, {"standard_parallel1", 41.7833333, 69102}, {"standard_parallel2", 40.7166667, 69102}, {"false_easting", 1640419.948, 69002}, {"false_northing", 3280839.895, 69002}, {nullptr, 0, 0} }},
		{65018, "Lambert_Conformal_Conic_2SP", { {"latitude_of_origin", 40.3333333, 69102}, {"central_meridian", -111.5000000, 69102}, {"standard_parallel1", 41.7833333, 69102}, {"standard_parallel2", 40.7166667, 69102}, {"false_easting", 1640416.667, 69003}, {"false_northing", 3280833.333, 69003}, {nullptr, 0, 0} }},
		{65019, "Lambert_Conformal_Conic_1SP", { {"latitude_of_origin", 52, 69105}, {"central_meridian", 0, 69105}, {"scale_factor", 0.99987742, 69201}, {"false_easting", 600000, 69001}, {"false_northing", 2200000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65023, "Transverse_Mercator", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", 3.0, 69102}, {"scale_factor", 0.9996, 69201}, {"false_easting", 1640416.667, 69003}, {"false_northing", 0, 69003}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65024, "Mercator_2SP", { {"standard_parallel_1", 42, 69102}, {"central_meridian", 51, 69102}, {"false_easting", 0, 69001}, {"false_northing", 0, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65025, "Lambert_Conformal_Conic_1SP", { {"latitude_of_origin", 46.8000000, 69102}, {"central_meridian", 2.3372292, 69102}, {"scale_factor", 0.99987742, 69201}, {"false_easting", 600000, 69001}, {"false_northing", 2200000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65026, "Oblique_Mercator", { {"latitude_of_center", 47.14439372222222, 69102}, {"longitude_of_center", 19.04857177777778, 69102}, {"azimuth", 90, 69102}, {"rectified_grid_angle", 90, 69102}, {"scale_factor", 0.99993, 69201}, {"false_easting", 650000, 69001}, {"false_northing", 200000, 69001} }},
		{65027, "Mercator_1SP", { {"latitude_of_origin", 0, 69102}, {"central_meridian", 110, 69102}, {"scale_factor", 0.997, 69201}, {"false_easting", 3900000, 69001}, {"false_northing", 900000, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
		{65028, "Transverse_Mercator", { {"latitude_of_origin", 0.0, 69102}, {"central_meridian", -135, 69102}, {"scale_factor", 0.9996, 69201}, {"false_easting", 500000, 69001}, {"false_northing", 0, 69001}, {nullptr, 0, 0}, {nullptr, 0, 0} }},
	};

	inline const std::vector<VerticalDatumData> verticalDatums = {
		{66601, "GIGS vertical datum U"},
		{66602, "GIGS vertical datum V"},
		{66603, "GIGS vertical datum W"},
	};

	inline const std::vector<VerticalCrsData> verticalCrs = {
		{64501, "GIGS vertCRS U1 height", 66601, "H", AxisOrientationEnum::Up, 69001},
		{64502, "GIGS vertCRS U1 depth", 66601, "D", AxisOrientationEnum::Down, 69001},
		{64503, "GIGS vertCRS U2 height", 66601, "H", AxisOrientationEnum::Up, 69002},
		{64504, "GIGS vertCRS U2 depth", 66601, "D", AxisOrientationEnum::Down, 69002},
		{64505, "GIGS vertCRS V1 height", 66602, "H", AxisOrientationEnum::Up, 69001},
		{64506, "GIGS vertCRS V1 depth", 66602, "D", AxisOrientationEnum::Down, 69001},
		{64509, "GIGS vertCRS V2 height", 66602, "H", AxisOrientationEnum::Up, 69003},
		{64507, "GIGS vertCRS W1 height", 66603, "H", AxisOrientationEnum::Up, 69001},
		{64508, "GIGS vertCRS W1 depth", 66603, "D", AxisOrientationEnum::Down, 69001},
	};

	class GigsFactory
	{
		template <typename T, typename Pred>
		static auto Find(std::vector<T> const& collection, Pred pred, std::string const& notFoundMessage) -> T const&
		{
			for (auto const& e : collection)
				if (pred(e))
					return e;
			throw std::runtime_error(notFoundMessage);
		}

		static auto IsLinearUnit(int code) -> bool
		{
			for (auto const& u : linearUnits) if (u.Code == code) return true;
			return false;
		}

		static auto IsAngularUnit(int code) -> bool
		{
			for (auto const& u : angularUnits) if (u.Code == code) return true;
			return false;
		}

		void AddParameter(std::vector<Parameter>& parameters, const char* name, double value, int unit)
		{
			if (!name)
				return;

			if (IsLinearUnit(unit))
				parameters.emplace_back(std::string(name), CreateLinearUnit(unit).GetMetersPerUnit() * value);
			else if (IsAngularUnit(unit))
				parameters.emplace_back(std::string(name), CreateAngularUnit(unit).GetRadiansPerUnit() * value * 180.0 / kPi);
			else
				parameters.emplace_back(std::string(name), value);
		}

	public:
		auto CreateLinearUnit(int code) -> LinearUnit
		{
			auto const& u = Find(linearUnits, [code](auto const& e) { return e.Code == code; }, "No linear unit found with the specified code");
			return LinearUnit{u.Name, "GIGS", code, "", "", u.MetersPerUnit};
		}

		auto CreateAngularUnit(int code) -> AngularUnit
		{
			auto const& u = Find(angularUnits, [code](auto const& e) { return e.Code == code; }, "No angular unit found with the specified code");
			return AngularUnit{u.Name, "GIGS", code, "", "", u.RadiansPerUnit};
		}

		auto CreateEllipsoidByName(std::string const& name) -> Ellipsoid
		{
			auto const& e = Find(ellipsoids, [&name](auto const& x) { return name == x.Name; }, "No ellipsoid found with the specified name");
			if (e.SemiMinorAxis.has_value())
				return Ellipsoid::CreateFromSemiMajorAxisSemiMinorAxis(e.Name, "GIGS", e.Code, e.SemiMajorAxis, e.SemiMinorAxis.value(), CreateLinearUnit(e.Unit));
			return Ellipsoid::CreateFromSemiMajorAxisInverseFlattening(e.Name, "GIGS", e.Code, e.SemiMajorAxis, e.InverseFlattening.value(), CreateLinearUnit(e.Unit));
		}

		auto CreatePrimeMeridian(int code) -> PrimeMeridian
		{
			auto const& m = Find(primeMeridians, [code](auto const& e) { return e.Code == code; }, "No prime meridian found with the specified code");
			return PrimeMeridian{m.Name, "GIGS", code, "", "", CreateAngularUnit(m.Unit), m.LongitudeFromGreenwich};
		}

		auto CreateHorizontalDatum(int code) -> std::shared_ptr<HorizontalDatum>
		{
			auto const& d = Find(datums, [code](auto const& e) { return e.Code == code; }, "No datum found with the specified code");
			return std::make_shared<HorizontalDatum>(d.Name, "GIGS", code, "", "", DatumType::HD_Geocentric, CreateEllipsoidByName(d.EllipsoidName));
		}

		auto GetPrimeMeridianCodeFromDatumCode(int datumCode) -> int
		{
			auto const& d = Find(datums, [datumCode](auto const& e) { return e.Code == datumCode; }, "No datum found with the specified code");
			auto const& m = Find(primeMeridians, [&d](auto const& e) { return std::string(d.PrimeMeridianName) == e.Name; }, "No prime meridian found with the specified name");
			return m.Code;
		}

		auto CreateGeographicCoordinateSystem(int code) -> std::shared_ptr<GeographicCoordinateSystem>
		{
			auto const& crs = Find(geodeticCRS, [code](auto const& e) { return e.Code == code; }, "No geographic system found with the specified code");

			auto const datumPtr = CreateHorizontalDatum(crs.DatumCode);
			auto const pm = CreatePrimeMeridian(GetPrimeMeridianCodeFromDatumCode(crs.DatumCode));

			AxisInfo axis0, axis1;
			if (crs.AxesChanged)
			{
				axis0 = AxisInfo("Long", AxisOrientationEnum::East);
				axis1 = AxisInfo("Lat", AxisOrientationEnum::North);
			}
			else
			{
				axis0 = AxisInfo("Lat", AxisOrientationEnum::North);
				axis1 = AxisInfo("Long", AxisOrientationEnum::East);
			}

			return GetCoordinateSystemFactory()->CreateGeographicCoordinateSystem(crs.Name, pm.GetAngularUnit(), datumPtr, pm, axis0, axis1);
		}

		auto CreateProjection(int code) -> std::shared_ptr<Projection>
		{
			auto const& p = Find(projections, [code](auto const& e) { return e.Code == code; }, "No projection found with the specified code");

			std::vector<Parameter> parameters;
			for (auto const& par : p.P)
				AddParameter(parameters, par.Name, par.Value, par.Unit);

			return std::make_shared<Projection>(std::string(p.OgsName), parameters);
		}

		auto CreateProjectedCoordinateSystem(std::string const& code) -> std::shared_ptr<ProjectedCoordinateSystem>
		{
			auto const numericCode = std::stoi(code);
			auto const& crs = Find(userProjectedCRS, [numericCode](auto const& e) { return e.Code == numericCode; }, "No projected system found with the specified code");

			auto const gcs = CreateGeographicCoordinateSystem(crs.BaseCrsCode);
			auto const projection = CreateProjection(crs.ProjCode);
			auto const linearUnit = CreateLinearUnit(crs.CsAxis1Unit);

			return std::make_shared<ProjectedCoordinateSystem>(
				crs.Name, "GIGS", numericCode, "", "",
				gcs,
				projection,
				linearUnit,
				AxisInfo(crs.CsAxis1Abbreviation, crs.CsAxis1Orientation),
				AxisInfo(crs.CsAxis2Abbreviation, crs.CsAxis2Orientation));
		}

		auto CreateVerticalDatum(int code) -> std::shared_ptr<VerticalDatum>
		{
			auto const& d = Find(verticalDatums, [code](auto const& e) { return e.Code == code; }, "No vertical datum found with the specified code");
			return std::make_shared<VerticalDatum>(d.Name, "GIGS", code, "", "", DatumType::VD_GeoidModelDerived);
		}

		auto CreateVerticalCoordinateSystem(std::string const& code) -> std::shared_ptr<VerticalCoordinateSystem>
		{
			auto const numericCode = std::stoi(code);
			auto const& crs = Find(verticalCrs, [numericCode](auto const& e) { return e.Code == numericCode; }, "No vertical system found with the specified code");

			return std::make_shared<VerticalCoordinateSystem>(
				crs.Name, "GIGS", numericCode, "", "",
				CreateLinearUnit(crs.CsAxis1Unit),
				CreateVerticalDatum(crs.GigsDatumCode),
				AxisInfo(crs.CsAxis1Abbreviation, crs.CsAxis1Orientation));
		}
	};
}
