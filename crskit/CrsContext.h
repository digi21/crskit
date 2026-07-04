#pragma once

#include <string>

namespace CrsKit
{
	// The Well-Known Text flavour GetWkt() emits. A single selector for the five effective formats: the
	// two WKT1 dialects (OGC 01-009 "Simple Features" and ESRI, the latter with or without the EPSG code
	// appended) and the two ISO 19162 WKT2 generations (2015 and 2019). Parsing auto-detects the grammar
	// from the root keyword, so this only affects emission.
	enum class WktVersion
	{
		Wkt1,                 // OGC 01-009 (Simple Features)
		Wkt1Esri,             // ESRI WKT1, without the EPSG authority code
		Wkt1EsriWithEpsgCode, // ESRI WKT1, with the EPSG authority code
		Wkt2_2015,            // ISO 19162:2015 / OGC 12-063r5
		Wkt2_2019,            // ISO 19162:2019 / OGC 18-010r7
	};

	[[nodiscard]] constexpr auto IsWkt2(WktVersion version) -> bool
	{
		return version == WktVersion::Wkt2_2015 || version == WktVersion::Wkt2_2019;
	}

	[[nodiscard]] constexpr auto IsEsri(WktVersion version) -> bool
	{
		return version == WktVersion::Wkt1Esri || version == WktVersion::Wkt1EsriWithEpsgCode;
	}

	[[nodiscard]] constexpr auto IsEsriWithEpsgCode(WktVersion version) -> bool
	{
		return version == WktVersion::Wkt1EsriWithEpsgCode;
	}

	// How CreateFromWkt resolves a CRS (or sub-element) whose WKT carries an EPSG AUTHORITY/ID, or whose
	// name matches a known ESRI system. WKT2 always builds from the pieces; this selects the WKT1 behaviour.
	enum class WktParsePolicy
	{
		// "The text wins": build the object from the WKT pieces and keep the AUTHORITY/ID only as metadata,
		// never redirecting to the catalogue. Aligns WKT1 with WKT2 and with PROJ/GDAL/GeoTools. Default.
		TextWins,
		// "The code wins" (legacy): if the WKT carries an EPSG AUTHORITY, fetch the official object from the
		// catalogue (discarding the parsed pieces); when there is none, snap by matching the ESRI name.
		CatalogWins,
	};

	// Ambient, reusable settings that influence CRS construction and WKT emission ("Type A" options:
	// the ones you set once and keep). Held by the current Environment as its default context and read
	// by the deep code (grid loaders) that cannot receive it per call; entry points also accept an
	// explicit CrsContext to override these per call. Companion to CoordinateTransformationOptions,
	// which carries the per-call transform-building callbacks ("Type B"). Modelled after PROJ's
	// PJ_CONTEXT: a default exists for convenience, and any call may pass its own.
	struct CrsContext final
	{
		// Output flavour for the no-argument GetWkt(); the GetWkt(WktVersion) overload overrides it per call.
		WktVersion wktVersion{ WktVersion::Wkt1 };
		// How CreateFromWkt resolves a WKT carrying an EPSG AUTHORITY/ID or a known ESRI name (see enum).
		WktParsePolicy parsePolicy{ WktParsePolicy::TextWins };
		// Search path prepended to grid file names (NadCon/NTv2/EGM) and ESRI data files. Read deep in the
		// grid loaders, so it stays ambient rather than threaded through every transform constructor.
		std::string dataDirectory{};
		// Emit a horizontal-only WKT for a compound CRS whose vertical component is local/unknown.
		bool createHorizontalWktIfVerticalLocal{};
	};
}
