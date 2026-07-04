#pragma once

// WKT2 (ISO 19162:2019 / OGC 18-010r7) emission and parsing.
//
// WKT2 is structurally different from the OGC 01-009 WKT1 the rest of the library emits: a projected
// CRS is decomposed into BASEGEOGCRS + CONVERSION[METHOD,PARAMETER...] + CS[...] + AXIS..., authority
// references use ID["EPSG",code] instead of AUTHORITY, and the coordinate system declares its type and
// dimension with CS[type,dim]. To avoid scattering version branches across every value type's GetWkt(),
// all WKT2 logic lives here as free functions; each CRS type's GetWkt() delegates to ToWkt2() when
// CrsContext::wktVersion selects WKT2, and CoordinateSystemFactory routes a WKT2 root keyword
// to FromWkt2().

#include <memory>
#include <string>

#include "../CrsContext.h"
#include "../CrsKitExport.h"
#include "../TokenWkt.h"

namespace CrsKit::CoordinateSystems
{
	class CoordinateSystem;
	class GeographicCoordinateSystem;
	class ProjectedCoordinateSystem;
	class GeocentricCoordinateSystem;
	class VerticalCoordinateSystem;
	class CompoundCoordinateSystem;
	class LocalCoordinateSystem;
}

namespace CrsKit::Wkt
{
	// True if 'keyword' (case-insensitive) is a WKT2 CRS keyword, i.e. the root names a WKT2 string.
	[[nodiscard]] _modeCrsKit auto IsWkt2Keyword(std::string const& keyword) -> bool;

	// Parse a WKT2 token tree (its root must be a WKT2 CRS keyword) into a coordinate system.
	[[nodiscard]] _modeCrsKit auto FromWkt2(TokenWkt const& root) -> std::shared_ptr<CoordinateSystems::CoordinateSystem>;

	// Emit a coordinate system as WKT2 (the version chooses 2015 vs 2019, which only differ in the
	// geographic CRS keyword). Pure: no global state is read, so the caller fully controls the output.
	[[nodiscard]] _modeCrsKit auto ToWkt2(CoordinateSystems::GeographicCoordinateSystem const& gcs, WktVersion version) -> std::string;
	[[nodiscard]] _modeCrsKit auto ToWkt2(CoordinateSystems::ProjectedCoordinateSystem const& pcs, WktVersion version) -> std::string;
	[[nodiscard]] _modeCrsKit auto ToWkt2(CoordinateSystems::GeocentricCoordinateSystem const& gcs, WktVersion version) -> std::string;
	[[nodiscard]] _modeCrsKit auto ToWkt2(CoordinateSystems::VerticalCoordinateSystem const& vcs, WktVersion version) -> std::string;
	[[nodiscard]] _modeCrsKit auto ToWkt2(CoordinateSystems::CompoundCoordinateSystem const& ccs, WktVersion version) -> std::string;
	[[nodiscard]] _modeCrsKit auto ToWkt2(CoordinateSystems::LocalCoordinateSystem const& lcs, WktVersion version) -> std::string;
}
