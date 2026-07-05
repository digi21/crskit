#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "CrsContext.h"
#include "CoordinateSystems/AxisOrientationEnum.h"
#include "CoordinateTransformations/CoordinateOperation.h"
#include "Positioning/Envelope.h"

namespace CrsKit::Epsg
{
	class IAuthorityProvider
	{
	public:
		virtual ~IAuthorityProvider() = default;
		[[nodiscard]] virtual auto GetUnitIsLength(int angularUnitCode) -> bool = 0;
		[[nodiscard]] virtual auto GetUnitIsAngle(int angularUnitCode) -> bool = 0;
		[[nodiscard]] virtual auto GetCoordinateSystemName(int coordinateSystemCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetCoordinateReferenceSystemName(int crsCode, bool useEsriName) -> std::string = 0;
		[[nodiscard]] virtual auto GetCoordinateReferenceSystemNames(std::string const& tipoCRS) -> std::unordered_map<int, std::string> = 0;
		[[nodiscard]] virtual auto GetCoordinateSystemType(int crsCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetGeographicCrsCodeForProjectedCrs(int crsCode) -> int = 0;
		[[nodiscard]] virtual auto GetGeographic2DCrsCodeForGeographic3DCrs(int crsCode) -> int = 0;
		[[nodiscard]] virtual auto GetGeographic3DCrsCodeForGeographic2DCrs(int crsCode) -> int = 0;
		[[nodiscard]] virtual auto GetOperationCodeForCrs(int crsCode) -> int = 0;
		[[nodiscard]] virtual auto GetAlgorithmCodeForOperation(int operationCode) -> int = 0;
		[[nodiscard]] virtual auto GetDatumCodeForCrs(int crsCode) -> int = 0;
		[[nodiscard]] virtual auto GetAlias(int datumCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetInfoDatum(int datumCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetDatumOrigin(int datumCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetDatumAreaOfUse(int datumCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetEllipsoidCodeForDatum(int datumCode) -> int = 0;
		[[nodiscard]] virtual auto GetCoordinateReferenceSystemInfo(int crsCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetMeridianCodeForDatum(int datumCode) -> int = 0;
		[[nodiscard]] virtual auto GetDatumName(int datumCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetEllipsoidName(int ellipsoidCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetMeridianName(int meridianCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetMeridianInfo(int meridianCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetMeridianLongitude(int meridianCode) -> double = 0;
		[[nodiscard]] virtual auto GetMeridianAngularUnit(int meridianCode) -> int = 0;
		[[nodiscard]] virtual auto GetUnitName(int angularUnitCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetUnitInfo(int angularUnitCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetCoordinateSystemCodeForCrs(int crsCode) -> int = 0;
		[[nodiscard]] virtual auto GetCoordinateSystemAxisNames(int coordinateSystemCode) -> std::vector<std::string> = 0;
		[[nodiscard]] virtual auto GetCoordinateSystemAxisOrientation(int coordinateSystemCode) -> std::vector<CoordinateSystems::AxisOrientationEnum> = 0;
		[[nodiscard]] virtual auto GetCoordinateSystemAxisUnitCodes(int coordinateSystemCode) -> std::vector<int> = 0;
		[[nodiscard]] virtual auto GetAreaOfUseCodeForCrs(int crsCode) -> int = 0;
		[[nodiscard]] virtual auto GetApplicationAreaDescription(int crsCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetBounds(int areaOfUseCode) -> Positioning::Envelope = 0;
		[[nodiscard]] virtual auto GetGeographicCrsCodeForDatum(int datumCode) -> int = 0;
		[[nodiscard]] virtual auto GetSemiMajorAxis(int ellipsoidCode) -> double = 0;
		[[nodiscard]] virtual auto GetInverseFlattening(int ellipsoidCode) -> double = 0;
		[[nodiscard]] virtual auto GetSemiMinorAxis(int ellipsoidCode) -> double = 0;
		[[nodiscard]] virtual auto GetEllipsoidLinearUnit(int ellipsoidCode) -> int = 0;
		virtual auto OperationCodesToTransformSystemAtoB(int sourceSystem, int targetSystem) -> std::vector<CoordinateTransformations::CoordinateOperation> = 0;

		// Geoid (Geographic3D-to-GravityRelatedHeight) operations that produce the given vertical CRS,
		// regardless of which geographic realization is their source. Used as a datum-equivalent fallback
		// when the exact (geographic, vertical) pair has no operation because the geoid is catalogued
		// against a specific realization (e.g. Alicante height's geoid is EPSG 9410 with source
		// ETRS89-ESP [REGENTE], not the generic ETRS89/WGS 84). Default: none (optional capability).
		[[nodiscard]] virtual auto GeographicToVerticalOperationCodes(int verticalTargetSystem) -> std::vector<CoordinateTransformations::CoordinateOperation> { return {}; }
		[[nodiscard]] virtual auto GetCoordinateOperationName(int operationCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetCoordinateOperationDescription(int operationCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetCoordinateOperationAreaOfUse(int coordinateOperationCode) -> int = 0;
		[[nodiscard]] virtual auto GetAreaName(int areaOfUseCode) -> std::string = 0;
		// EPSG Geodetic Parameter Dataset version of the underlying database (e.g. "12.057"); "" if the
		// provider cannot determine it. Non-pure so alternative providers need not implement it.
		[[nodiscard]] virtual auto GetEpsgVersion() -> std::string { return {}; }
		[[nodiscard]] virtual auto GetCoordinateOperationInformationSource(int operationCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetCoordinateOperationSourceCrs(int coordinateOperationCode) -> int = 0;
		[[nodiscard]] virtual auto GetCoordinateOperationTargetCrs(int coordinateOperationCode) -> int = 0;
		[[nodiscard]] virtual auto GetAlgorithmName(int algorithmCode) -> std::string = 0;
		[[nodiscard]] virtual auto GetFactorToStandardUnit(int angularUnitCode) -> double = 0;
		[[nodiscard]] virtual auto GetTransformationParameters(int operationCode, int algorithmCode) -> std::vector<std::variant<std::string, double>> = 0;
		[[nodiscard]] virtual auto GetProjectedCrsCountFromProjConvCodeAndBaseCrsCode(int projectionCode, int geographicCrsCode) const -> int = 0;
		[[nodiscard]] virtual auto GetCrsCodeFromProjConvCodeAndBaseCrsCode(int projectionCode, int geographicCrsCode) const -> int = 0;
	};
}
