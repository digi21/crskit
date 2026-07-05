#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <format>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include "CrsKit.h"
#include "OpenGisException.h"
#include "sqlite3.h"


namespace CrsKit::Epsg
{
	class Stmt
	{
		sqlite3_stmt* _stmt{};
	public:
		Stmt() = delete;
		Stmt(Stmt const&) = delete;
		Stmt(Stmt&&) = delete;
		Stmt& operator=(Stmt const&) = delete;
		Stmt& operator=(Stmt&&) = delete;

		Stmt(sqlite3* db, std::string const& sql)
		{
			if (sqlite3_prepare_v2(db, sql.c_str(), -1, &_stmt, nullptr) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(db));
			}
		}


		[[nodiscard]] operator sqlite3_stmt* () const noexcept { return _stmt; }

		~Stmt()
		{
			sqlite3_finalize(_stmt);
		}

		[[nodiscard]] bool Step() const
		{
			return SQLITE_ROW == sqlite3_step(_stmt);
		}

		[[nodiscard]] std::map<std::string, std::variant<nullptr_t, int, double, std::string, std::unique_ptr<char[]>>> GetData() const
		{
			std::map<std::string, std::variant<nullptr_t, int, double, std::string, std::unique_ptr<char[]>>> result;

			for (auto i = 0; i < sqlite3_column_count(_stmt); i++)
			{
				auto name = std::string(sqlite3_column_name(_stmt, i));
				// The column name reported by SQLite respects the SCHEMA case (in the
				// official EPSG SQLite, the columns are lowercase). The rest of the provider
				// reads the keys in UPPERCASE, so we normalize here (ASCII names).
				for (auto& ch : name) if (ch >= 'a' && ch <= 'z') ch = static_cast<char>(ch - 'a' + 'A');

				switch (sqlite3_column_type(_stmt, i))
				{
				case SQLITE_INTEGER:
					result[name] = sqlite3_column_int(_stmt, i);
					break;
				case SQLITE_FLOAT:
					result[name] = sqlite3_column_double(_stmt, i);
					break;
				case SQLITE_TEXT:
					result[name] = std::string(reinterpret_cast<char const*>(sqlite3_column_text(_stmt, i)));
					break;
				case SQLITE_BLOB:
				{
					std::unique_ptr<char[]> blob{ new char[sqlite3_column_bytes(_stmt, i)] };
					memcpy(blob.get(), sqlite3_column_blob(_stmt, i), sqlite3_column_bytes(_stmt, i));
					result[name] = std::move(blob);
				}
				break;
				case SQLITE_NULL:
					result[name] = nullptr;
					break;
				}
			}

			return result;
		}

		template<int index, typename First>
		First ReadField() const
		{
			if constexpr (std::is_same_v<int, First>)
			{
				return static_cast<First>(sqlite3_column_int(_stmt, index));
			}

			if constexpr (std::is_same_v<std::optional<int>, First>)
			{
				if (SQLITE_NULL == sqlite3_column_type(_stmt, index))
					return{};

				return static_cast<First>(sqlite3_column_int(_stmt, index));
			}

			if constexpr (std::is_same_v<bool, First>)
			{
				return (0 != sqlite3_column_int(_stmt, index));
			}

			if constexpr (std::is_same_v<std::optional<bool>, First>)
			{
				if (SQLITE_NULL == sqlite3_column_type(_stmt, index))
					return{};

				return 0 != sqlite3_column_int(_stmt, index);
			}

			if constexpr (std::is_same_v<double, First>)
			{
				return sqlite3_column_double(_stmt, index);
			}

			if constexpr (std::is_same_v<std::optional<double>, First>)
			{
				if (SQLITE_NULL == sqlite3_column_type(_stmt, index))
					return{};

				return sqlite3_column_double(_stmt, index);
			}

			if constexpr (std::is_same_v<std::string, First>)
			{
				return std::string(reinterpret_cast<char const*>(sqlite3_column_text(_stmt, index)));
			}

			if constexpr (std::is_same_v<std::optional<std::string>, First>)
			{
				if (SQLITE_NULL == sqlite3_column_type(_stmt, index))
					return{};

				return std::string(reinterpret_cast<char const*>(sqlite3_column_text(_stmt, index)));
			}

			assert(false);
		}

		template<int index, typename First, typename Second, typename... Args>
		std::tuple<First, Second, Args...> ReadField() const
		{
			if constexpr (std::is_same_v<int, First>)
			{
				return std::tuple_cat(std::tuple(sqlite3_column_int(_stmt, index)), ReadField<index + 1, Second, Args...>());
			}
			if constexpr (std::is_same_v<bool, First>)
			{
				return std::tuple_cat(std::tuple(0 != sqlite3_column_int(_stmt, index)), ReadField<index + 1, Second, Args...>());
			}
			else if constexpr (std::is_same_v<double, First>)
			{
				return std::tuple_cat(std::tuple(sqlite3_column_double(_stmt, index)), ReadField<index + 1, Second, Args...>());
			}
			else if constexpr (std::is_same_v<std::string, First>)
			{
				return std::tuple_cat(std::tuple(std::string(reinterpret_cast<char const*>(sqlite3_column_text(_stmt, index)))), ReadField<index + 1, Second, Args...>());
			}
			else {
				assert(false);
			}
		}
	};

	class SQliteProvider final : public IAuthorityProvider
	{
		std::shared_ptr<sqlite3> _db;

	public:
		explicit SQliteProvider(std::string const& connectionString)
		{
			// The single connection is shared across threads (the provider lives in the Environment), so we
			// open it in serialized mode (SQLITE_OPEN_FULLMUTEX): SQLite then guards the connection with its
			// own mutex and concurrent use from several threads is safe, regardless of the global default.
			// The EPSG catalogue is only ever read, so READONLY is both correct and makes a missing file
			// fail loudly here instead of silently creating an empty database. FULLMUTEX is a no-op when
			// SQLite is built without threading (SQLITE_THREADSAFE=0), which would leave the shared
			// connection unguarded -- a precondition we reject up front (and in release builds too, hence
			// an exception rather than assert).
			if (sqlite3_threadsafe() == 0)
				throw OpenGisException("SQLite was built without thread-safety (SQLITE_THREADSAFE=0); the EPSG provider shares one connection across threads and requires a thread-safe SQLite build.");

			sqlite3* db = nullptr;
			if (auto const error = sqlite3_open_v2(connectionString.c_str(), &db, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, nullptr))
				throw std::runtime_error(sqlite3_errstr(error));

			_db = std::shared_ptr<sqlite3>(
				db,
				[](sqlite3* db)
				{
					sqlite3_close(db);
				});
		}

		virtual ~SQliteProvider() = default;

		auto GetUnitIsLength(int angularUnitCode) -> bool override
		{
			try
			{
				auto const value = ExecuteScalar<std::string>(std::format("SELECT UNIT_OF_MEAS_TYPE FROM epsg_unitofmeasure WHERE UOM_CODE={}", angularUnitCode));
				return value == "length";
			}
			catch (...)
			{
				return false;
			}
		}

		auto GetUnitIsAngle(int angularUnitCode) -> bool override
		{
			try
			{
				auto const value = ExecuteScalar<std::string>(std::format("SELECT UNIT_OF_MEAS_TYPE FROM epsg_unitofmeasure WHERE UOM_CODE={}", angularUnitCode));
				return value == "angle";
			}
			catch (...)
			{
				return false;
			}
		}

		auto GetCoordinateSystemName(int coordinateSystemCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT COORD_SYS_NAME FROM epsg_coordinatesystem WHERE COORD_SYS_CODE={}", coordinateSystemCode));
		}

		auto GetCoordinateReferenceSystemName(int crsCode, bool useEsriName) -> std::string override
		{
			if (LOCAL_COORDINATE_SYSTEM == crsCode)
				return "Local";

			if (useEsriName)
			{
				auto const name = EsriProjectionEngine::NameFromEpsgCode(crsCode);
				if (!name.empty())
					return name;
			}

			return ExecuteScalar<std::string>(std::format("SELECT COORD_REF_SYS_NAME FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_CODE={}", crsCode));
		}

		auto GetCoordinateReferenceSystemNames(std::string const& tipoCRS) -> std::unordered_map<int, std::string> override
		{
			std::unordered_map<int, std::string> lista;

			auto const sql = std::format("SELECT COORD_REF_SYS_CODE, COORD_REF_SYS_NAME FROM epsg_coordinatereferencesystem WHERE (COORD_REF_SYS_KIND='{}' AND SHOW_CRS=1 AND DEPRECATED=0)", tipoCRS.c_str());
			Stmt const stmt{ _db.get(), sql };
			while (stmt.Step())
			{
				auto const data = stmt.GetData();
				auto const code = std::get<int>(data.at("COORD_REF_SYS_CODE"));
				auto const name = std::get<std::string>(data.at("COORD_REF_SYS_NAME"));

				lista[code] = name;
			}

			return lista;
		}

		auto GetCoordinateSystemType(int crsCode) -> std::string override
		{
			if (LOCAL_COORDINATE_SYSTEM == crsCode)
				return "Local";

			return ExecuteScalar<std::string>(std::format("SELECT COORD_REF_SYS_KIND FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_CODE={}", crsCode));
		}

		auto GetGeographicCrsCodeForProjectedCrs(int crsCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT BASE_CRS_CODE FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_CODE={}", crsCode));
		}

		auto GetGeographic2DCrsCodeForGeographic3DCrs(int crsCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT COORD_REF_SYS_CODE FROM epsg_coordinatereferencesystem WHERE (COORD_REF_SYS_KIND = 'geographic 2D') AND (BASE_CRS_CODE = {})", crsCode));
		}

		auto GetGeographic3DCrsCodeForGeographic2DCrs(int crsCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT BASE_CRS_CODE FROM epsg_coordinatereferencesystem WHERE (COORD_REF_SYS_CODE={}) AND (BASE_CRS_CODE IN (SELECT COORD_REF_SYS_CODE FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_KIND='geographic 3D'))", crsCode));
		}

		auto GetOperationCodeForCrs(int crsCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT PROJECTION_CONV_CODE FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_CODE={}", crsCode));
		}

		auto GetAlgorithmCodeForOperation(int operationCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT COORD_OP_METHOD_CODE FROM epsg_coordoperation WHERE COORD_OP_CODE={}", operationCode));
		}

		auto GetDatumCodeForCrs(int crsCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT datum_code FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_CODE={}", crsCode));
		}

		auto GetGeographicCrsCodeForDatum(int datumCode) -> int override
		{
			// The geographic 2D CRS based on this datum (e.g. datum 6230 ED50 -> CRS 4230). Used to look
			// up the datum's transformation to WGS 84 when emitting TOWGS84. Returns 0 if there is none.
			return ExecuteScalar<int>(std::format("SELECT COORD_REF_SYS_CODE FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_KIND='geographic 2D' AND DEPRECATED=0 AND datum_code={} LIMIT 1", datumCode));
		}

		auto GetAlias(int datumCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT ALIAS FROM epsg_alias WHERE OBJECT_CODE={}", datumCode));
		}

		auto GetInfoDatum(int datumCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT REMARKS FROM epsg_datum WHERE datum_code={}", datumCode));
		}

		auto GetDatumOrigin(int datumCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT origin_description FROM epsg_datum WHERE datum_code={}", datumCode));
		}

		auto GetDatumAreaOfUse(int datumCode) -> std::string override
		{
			auto const codeAreaOfUse = ExecuteScalar<int>(std::format("SELECT EXTENT_CODE FROM epsg_usage WHERE OBJECT_TABLE_NAME='epsg_datum' AND OBJECT_CODE={}", datumCode));
			if (codeAreaOfUse == 0)
				return {};

			return ExecuteScalar<std::string>(std::format("SELECT EXTENT_DESCRIPTION FROM epsg_extent WHERE EXTENT_CODE={}", codeAreaOfUse));
		}

		auto GetEllipsoidCodeForDatum(int datumCode) -> int override
		{
			if (ExecuteScalar<int>(std::format("SELECT COUNT(DATUM_ENSEMBLE_CODE) FROM epsg_datumensemble WHERE DATUM_ENSEMBLE_CODE={}", datumCode)))
			{
				return ExecuteScalar<int>(std::format("SELECT ellipsoid_code FROM epsg_datum WHERE datum_code IN (SELECT DATUM_CODE FROM epsg_datumensemblemember WHERE DATUM_ENSEMBLE_CODE = {} LIMIT 1)", datumCode));
			}

			return ExecuteScalar<int>(std::format("SELECT ellipsoid_code FROM epsg_ellipsoid WHERE ellipsoid_code IN (SELECT ellipsoid_code FROM epsg_datum WHERE DATUM_CODE={})", datumCode));
		}

		auto GetCoordinateReferenceSystemInfo(int crsCode) -> std::string override
		{
			if (LOCAL_COORDINATE_SYSTEM == crsCode)
				return "Local";

			return ExecuteScalar<std::string>(std::format("SELECT REMARKS FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_CODE={}", crsCode));
		}

		auto GetMeridianCodeForDatum(int datumCode) -> int override
		{
			if (ExecuteScalar<int>(std::format("SELECT COUNT(DATUM_ENSEMBLE_CODE) FROM epsg_datumensemble WHERE DATUM_ENSEMBLE_CODE={}", datumCode)))
			{
				return ExecuteScalar<int>(std::format("SELECT prime_meridian_code FROM epsg_datum WHERE datum_code IN (SELECT DATUM_CODE FROM epsg_datumensemblemember WHERE DATUM_ENSEMBLE_CODE = {} LIMIT 1)", datumCode));
			}

			return ExecuteScalar<int>(std::format("SELECT prime_meridian_code FROM epsg_datum WHERE DATUM_CODE={}", datumCode));
		}

		auto GetDatumName(int datumCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT datum_name FROM epsg_datum WHERE datum_code={}", datumCode));
		}

		auto GetEllipsoidName(int ellipsoidCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT ellipsoid_name FROM epsg_ellipsoid WHERE ellipsoid_code={}", ellipsoidCode));
		}

		auto GetMeridianName(int meridianCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT prime_meridian_name FROM epsg_primemeridian WHERE prime_meridian_code={}", meridianCode));
		}

		auto GetMeridianInfo(int meridianCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT REMARKS FROM epsg_primemeridian WHERE prime_meridian_code={}", meridianCode));
		}

		auto GetMeridianLongitude(int meridianCode) -> double override
		{
			return ExecuteScalar<double>(std::format("SELECT GREENWICH_LONGITUDE FROM epsg_primemeridian WHERE prime_meridian_code={}", meridianCode));
		}

		auto GetMeridianAngularUnit(int meridianCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT UOM_CODE FROM epsg_primemeridian WHERE prime_meridian_code={}", meridianCode));
		}

		auto GetUnitName(int angularUnitCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT UNIT_OF_MEAS_NAME FROM epsg_unitofmeasure WHERE UOM_CODE={}", angularUnitCode));
		}

		auto GetUnitInfo(int angularUnitCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT REMARKS FROM epsg_unitofmeasure WHERE UOM_CODE={}", angularUnitCode));
		}

		auto GetCoordinateSystemCodeForCrs(int crsCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT COORD_SYS_CODE FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_CODE={}", crsCode));
		}

		auto GetCoordinateSystemAxisNames(int coordinateSystemCode) -> std::vector<std::string> override
		{
			std::vector<std::string> names;

			auto const sql = std::format(
				"SELECT COORD_AXIS_ABBREVIATION FROM epsg_coordinateaxis WHERE COORD_SYS_CODE={} ORDER BY coord_axis_order",
				coordinateSystemCode);

			Stmt const stmt{ _db.get(), sql };
			while (stmt.Step())
			{
				auto const data = stmt.GetData();
				names.push_back(std::get<std::string>(data.at("COORD_AXIS_ABBREVIATION")));
			}

			return names;
		}

		auto GetCoordinateSystemAxisOrientation(int coordinateSystemCode) -> std::vector<CoordinateSystems::AxisOrientationEnum> override
		{
			std::vector<CoordinateSystems::AxisOrientationEnum> orientaciones;

			auto const sql = std::format(
				"SELECT COORD_AXIS_ORIENTATION, coord_axis_order FROM epsg_coordinateaxis WHERE COORD_SYS_CODE={} ORDER BY coord_axis_order", coordinateSystemCode);

			Stmt const stmt{ _db.get(), sql };
			while (stmt.Step())
			{
				auto const data = stmt.GetData();
				auto const orientation = std::get<std::string>(data.at("COORD_AXIS_ORIENTATION"));

				if (0 == compareNoCase(orientation.c_str(), "north"))
					orientaciones.push_back(CoordinateSystems::AxisOrientationEnum::North);
				else if (0 == compareNoCase(orientation.c_str(), "south"))
					orientaciones.push_back(CoordinateSystems::AxisOrientationEnum::South);
				else if (0 == compareNoCase(orientation.c_str(), "east"))
					orientaciones.push_back(CoordinateSystems::AxisOrientationEnum::East);
				else if (0 == compareNoCase(orientation.c_str(), "west"))
					orientaciones.push_back(CoordinateSystems::AxisOrientationEnum::West);
				else if (0 == compareNoCase(orientation.c_str(), "up"))
					orientaciones.push_back(CoordinateSystems::AxisOrientationEnum::Up);
				else if (0 == compareNoCase(orientation.c_str(), "down"))
					orientaciones.push_back(CoordinateSystems::AxisOrientationEnum::Down);
				else
					orientaciones.push_back(CoordinateSystems::AxisOrientationEnum::Other);
			}

			return orientaciones;
		}

		auto GetCoordinateSystemAxisUnitCodes(int coordinateSystemCode) -> std::vector<int> override
		{
			std::vector<int> codes;
			auto const sql = std::format("SELECT UOM_CODE, coord_axis_order FROM epsg_coordinateaxis WHERE COORD_SYS_CODE={} ORDER BY coord_axis_order", coordinateSystemCode);

			Stmt const stmt{ _db.get(), sql };
			while (stmt.Step())
			{
				auto const uomCode = std::move(stmt.GetData().at("UOM_CODE"));
				if (std::holds_alternative<nullptr_t>(uomCode))
					continue;

				codes.push_back(std::get<int>(uomCode));
			}

			return codes;
		}

		auto GetAreaOfUseCodeForCrs(int crsCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT EXTENT_CODE FROM epsg_usage WHERE OBJECT_TABLE_NAME='epsg_coordinatereferencesystem' AND OBJECT_CODE={}", crsCode));
		}

		auto GetApplicationAreaDescription(int crsCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT EXTENT_DESCRIPTION FROM epsg_extent WHERE EXTENT_CODE={}", GetAreaOfUseCodeForCrs(crsCode)));
		}

		auto GetBounds(int areaOfUseCode) -> Positioning::Envelope override
		{
			auto const sql = std::format("SELECT BBOX_WEST_BOUND_LON, BBOX_SOUTH_BOUND_LAT, BBOX_EAST_BOUND_LON, BBOX_NORTH_BOUND_LAT FROM epsg_extent WHERE EXTENT_CODE={}", areaOfUseCode);

			std::vector<double> minCP;
			std::vector<double> maxCP;

			Stmt const stmt{ _db.get(), sql };
			if (stmt.Step())
			{
				auto const data = stmt.GetData();
				minCP.resize(2);
				maxCP.resize(2);

				minCP[0] = std::get<double>(data.at("BBOX_WEST_BOUND_LON"));
				minCP[1] = std::get<double>(data.at("BBOX_SOUTH_BOUND_LAT"));

				maxCP[0] = std::get<double>(data.at("BBOX_EAST_BOUND_LON"));
				maxCP[1] = std::get<double>(data.at("BBOX_NORTH_BOUND_LAT"));
			}

			return Positioning::Envelope{ minCP, maxCP };
		}

		auto GetSemiMajorAxis(int ellipsoidCode) -> double override
		{
			return ExecuteScalar<double>(std::format("SELECT SEMI_MAJOR_AXIS FROM epsg_ellipsoid WHERE ellipsoid_code={}", ellipsoidCode));
		}

		auto GetInverseFlattening(int ellipsoidCode) -> double override
		{
			return ExecuteScalar<double>(std::format("SELECT INV_FLATTENING FROM epsg_ellipsoid WHERE ellipsoid_code={}", ellipsoidCode));
		}

		auto GetSemiMinorAxis(int ellipsoidCode) -> double override
		{
			return ExecuteScalar<double>(std::format("SELECT SEMI_MINOR_AXIS FROM epsg_ellipsoid WHERE ellipsoid_code={}", ellipsoidCode));
		}

		auto GetEllipsoidLinearUnit(int ellipsoidCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT UOM_CODE FROM epsg_ellipsoid WHERE ellipsoid_code={}", ellipsoidCode));
		}

		auto OperationCodesToTransformSystemAtoB(int sourceSystem, int targetSystem) -> std::vector<CoordinateTransformations::CoordinateOperation> override
		{
			std::vector<CoordinateTransformations::CoordinateOperation> operations;

			auto const sql = std::format("SELECT COORD_OP_CODE, COORD_OP_NAME, COORD_OP_TYPE, COORD_TFM_VERSION, COORD_OP_VARIANT, COORD_OP_SCOPE, COORD_OP_ACCURACY, COORD_OP_METHOD_CODE, REMARKS, INFORMATION_SOURCE  FROM epsg_coordoperation WHERE SOURCE_CRS_CODE={} AND TARGET_CRS_CODE={} AND SHOW_OPERATION=1 AND DEPRECATED=0  AND COORD_OP_TYPE='transformation' ORDER BY COORD_OP_ACCURACY",
																sourceSystem,
																targetSystem);

			Stmt const stmt{ _db.get(), sql };
			while (stmt.Step())
			{
				auto const data = stmt.GetData();
				operations.push_back(CoordinateTransformations::CoordinateOperation(
					std::get<int>(data.at("COORD_OP_CODE")),
					std::get<std::string>(data.at("COORD_OP_NAME")),
					std::get<std::string>(data.at("COORD_OP_TYPE")),
					TypeOrDefaultValue<std::string>(data.at("COORD_TFM_VERSION")),
					TypeOrDefaultValue<int>(data.at("COORD_OP_VARIANT")),
					TypeOrDefaultValue<std::string>(data.at("COORD_OP_SCOPE")),
					TypeOrDefaultValue<double>(data.at("COORD_OP_ACCURACY")),
					std::get<int>(data.at("COORD_OP_METHOD_CODE")),
					TypeOrDefaultValue<std::string>(data.at("REMARKS")),
					TypeOrDefaultValue<std::string>(data.at("INFORMATION_SOURCE"))));
			}

			EnrichWithAreaAndGrids(operations);
			return operations;
		}

		auto GeographicToVerticalOperationCodes(int verticalTargetSystem) -> std::vector<CoordinateTransformations::CoordinateOperation> override
		{
			std::vector<CoordinateTransformations::CoordinateOperation> operations;

			// Operations producing the vertical CRS whose SOURCE is a Geographic 3D CRS (i.e. geoid /
			// Geographic3D-to-GravityRelatedHeight), ordered by accuracy. The source realization is not
			// constrained: any is acceptable because the geoid grid is interpolated at the point's lat/lon.
			auto const sql = std::format(
				"SELECT o.COORD_OP_CODE, o.COORD_OP_NAME, o.COORD_OP_TYPE, o.COORD_TFM_VERSION, o.COORD_OP_VARIANT, o.COORD_OP_SCOPE, o.COORD_OP_ACCURACY, o.COORD_OP_METHOD_CODE, o.REMARKS, o.INFORMATION_SOURCE "
				"FROM epsg_coordoperation o JOIN epsg_coordinatereferencesystem s ON o.SOURCE_CRS_CODE=s.COORD_REF_SYS_CODE "
				"WHERE o.TARGET_CRS_CODE={} AND o.SHOW_OPERATION=1 AND o.DEPRECATED=0 AND o.COORD_OP_TYPE='transformation' "
				"AND s.COORD_REF_SYS_KIND='geographic 3D' ORDER BY o.COORD_OP_ACCURACY",
				verticalTargetSystem);

			Stmt const stmt{ _db.get(), sql };
			while (stmt.Step())
			{
				auto const data = stmt.GetData();
				operations.push_back(CoordinateTransformations::CoordinateOperation(
					std::get<int>(data.at("COORD_OP_CODE")),
					std::get<std::string>(data.at("COORD_OP_NAME")),
					std::get<std::string>(data.at("COORD_OP_TYPE")),
					TypeOrDefaultValue<std::string>(data.at("COORD_TFM_VERSION")),
					TypeOrDefaultValue<int>(data.at("COORD_OP_VARIANT")),
					TypeOrDefaultValue<std::string>(data.at("COORD_OP_SCOPE")),
					TypeOrDefaultValue<double>(data.at("COORD_OP_ACCURACY")),
					std::get<int>(data.at("COORD_OP_METHOD_CODE")),
					TypeOrDefaultValue<std::string>(data.at("REMARKS")),
					TypeOrDefaultValue<std::string>(data.at("INFORMATION_SOURCE"))));
			}

			EnrichWithAreaAndGrids(operations);
			return operations;
		}

		auto GetCoordinateOperationName(int operationCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT COORD_OP_NAME FROM epsg_coordoperation WHERE COORD_OP_CODE={}", operationCode));
		}

		auto GetCoordinateOperationDescription(int operationCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT REMARKS FROM epsg_coordoperation WHERE COORD_OP_CODE={}", operationCode));
		}

		auto GetCoordinateOperationAreaOfUse(int coordinateOperationCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT EXTENT_CODE FROM epsg_usage WHERE OBJECT_TABLE_NAME='epsg_coordoperation' AND OBJECT_CODE={}", coordinateOperationCode));
		}

		auto GetAreaName(int areaOfUseCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT EXTENT_NAME FROM epsg_extent WHERE EXTENT_CODE={}", areaOfUseCode));
		}

		auto GetEpsgVersion() -> std::string override
		{
			try { return ExecuteScalar<std::string>("SELECT version_number FROM epsg_versionhistory ORDER BY version_date DESC LIMIT 1"); }
			catch (...) { return {}; }
		}

		// The grid file(s) an operation references (EPSG PARAM_VALUE_FILE_REF): a geoid model, an NTv2
		// datum-shift grid, etc. Empty for operations that need no file. Used to tell the caller which
		// file to obtain (e.g. an EGM2008 geoid grid) before the transform can be built.
		auto GetCoordinateOperationGridFiles(int operationCode) -> std::vector<std::string>
		{
			std::vector<std::string> files;
			auto const sql = std::format(
				"SELECT PARAM_VALUE_FILE_REF FROM epsg_coordoperationparamvalue "
				"WHERE COORD_OP_CODE={} AND PARAM_VALUE_FILE_REF IS NOT NULL AND PARAM_VALUE_FILE_REF!=''",
				operationCode);
			Stmt const stmt{ _db.get(), sql };
			while (stmt.Step())
			{
				auto const data = stmt.GetData();
				auto const& ref = data.at("PARAM_VALUE_FILE_REF");
				if (std::holds_alternative<std::string>(ref) && !std::get<std::string>(ref).empty())
					files.push_back(std::get<std::string>(ref));
			}
			return files;
		}

		// Fill in each candidate operation's area of use and required grid file(s) from its code, so the
		// caller can present both (choose by geography, and know which file to obtain).
		void EnrichWithAreaAndGrids(std::vector<CoordinateTransformations::CoordinateOperation>& operations)
		{
			for (auto& op : operations)
			{
				try { op.AreaOfUse = GetAreaName(GetCoordinateOperationAreaOfUse(op.Code)); }
				catch (...) { /* operation without a usage extent: leave area empty */ }
				op.GridFiles = GetCoordinateOperationGridFiles(op.Code);
			}
		}

		auto GetCoordinateOperationInformationSource(int operationCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT INFORMATION_SOURCE FROM epsg_coordoperation WHERE COORD_OP_CODE={}", operationCode));
		}

		auto GetCoordinateOperationSourceCrs(int coordinateOperationCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT SOURCE_CRS_CODE FROM epsg_coordoperation WHERE COORD_OP_CODE={}", coordinateOperationCode));
		}

		auto GetCoordinateOperationTargetCrs(int coordinateOperationCode) -> int override
		{
			return ExecuteScalar<int>(std::format("SELECT TARGET_CRS_CODE FROM epsg_coordoperation WHERE COORD_OP_CODE={}", coordinateOperationCode));
		}

		auto GetAlgorithmName(int algorithmCode) -> std::string override
		{
			return ExecuteScalar<std::string>(std::format("SELECT COORD_OP_METHOD_NAME FROM epsg_coordoperationmethod WHERE COORD_OP_METHOD_CODE={}", algorithmCode));
		}

		auto GetFactorToStandardUnit(int angularUnitCode) -> double override
		{
			try
			{
				auto const b = ExecuteScalar<double>(std::format("SELECT FACTOR_B FROM epsg_unitofmeasure WHERE UOM_CODE={}", angularUnitCode));
				auto const c = ExecuteScalar<double>(std::format("SELECT FACTOR_C FROM epsg_unitofmeasure WHERE UOM_CODE={}", angularUnitCode));
				return b / c;
			}
			catch (...)
			{
				return 0.0;
			}
		}

		auto GetProjectedCrsCountFromProjConvCodeAndBaseCrsCode(int projectionCode, int geographicCrsCode) const -> int override
		{
			return ExecuteScalar<int>(std::format(
				"SELECT COUNT(COORD_REF_SYS_CODE) FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_KIND='projected' AND PROJECTION_CONV_CODE={} AND BASE_CRS_CODE={}",
				projectionCode,
				geographicCrsCode));
		}

		auto GetCrsCodeFromProjConvCodeAndBaseCrsCode(int projectionCode, int geographicCrsCode) const -> int override
		{
			return ExecuteScalar<int>(std::format(
				"SELECT COORD_REF_SYS_CODE FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_KIND='projected' AND PROJECTION_CONV_CODE={} AND BASE_CRS_CODE={}",
				projectionCode,
				geographicCrsCode));
		}

		auto GetTransformationParameters(int operationCode, int algorithmCode) -> std::vector<std::variant<std::string, double>> override
		{
			std::vector<std::variant<std::string, double>> parameterList;

			auto sql = std::format(
				"SELECT a.PARAMETER_VALUE, a.PARAM_VALUE_FILE_REF, a.UOM_CODE, a.PARAMETER_CODE, b.SORT_ORDER FROM epsg_coordoperationparamvalue AS a INNER JOIN epsg_coordoperationparamusage AS b ON a.PARAMETER_CODE = b.PARAMETER_CODE and a.COORD_OP_METHOD_CODE=b.COORD_OP_METHOD_CODE WHERE a.COORD_OP_CODE={} AND a.COORD_OP_METHOD_CODE={} ORDER BY b.SORT_ORDER",
				operationCode, algorithmCode);

			Stmt const stmt{ _db.get(), sql };
			while (stmt.Step())
			{
				auto const data = stmt.GetData();
				if (std::holds_alternative<nullptr_t>(data.at("PARAMETER_VALUE")))
				{
					parameterList.push_back(std::get<std::string>(data.at("PARAM_VALUE_FILE_REF")));
				}
				else
				{
					auto const unit = std::get<int>(data.at("UOM_CODE"));
					auto value = std::get<double>(data.at("PARAMETER_VALUE"));

					if (GetUnitIsLength(unit))
						value = GetFactorToStandardUnit(unit) * value;
					else if (GetUnitIsAngle(unit))
						value = GetUtilities()->ToSexagesimal(unit, GetFactorToStandardUnit(unit), value);
					parameterList.push_back(value);
				}
			}

			return parameterList;
		}

	private:
		template<typename T>
		auto ExecuteScalar(std::string const& sql) const -> T
		{
			Stmt const stmt{ _db.get(), sql };

			assert(sqlite3_column_count(stmt) == 1);

			if (!stmt.Step())
				return {};// throw std::runtime_error("Could not read the value");


			auto result = stmt.ReadField<0, std::optional<T>>();
			if (!result.has_value())
				return {};

			return result.value();
		}

		template<typename T, typename T2>
		auto TypeOrDefaultValue(T2 const& value) -> std::optional<T>
		{
			if (std::holds_alternative<nullptr_t>(value))
				return {};

			if (std::holds_alternative<T>(value))
				return std::get<T>(value);

			if constexpr (std::is_same_v<float, T>)
			{
				if (std::holds_alternative<double>(value))
					return static_cast<float>(std::get<double>(value));
				if (std::holds_alternative<int>(value))
					return static_cast<float>(std::get<int>(value));
				return {};
			}

			if constexpr (std::is_same_v<double, T>)
			{
				if (std::holds_alternative<int>(value))
					return static_cast<double>(std::get<int>(value));
				return {};
			}

			return {};
		}
	};

}