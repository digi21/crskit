#pragma once

#include <map>
#include <string>

#include "CrsKitExport.h"

namespace CrsKit::Wkt { class TokenWkt; }

namespace CrsKit
{

class _modeCrsKit EsriProjectionEngine
{
public:
	// True when a WKT1 node uses ESRI's ArcGIS naming conventions (geographic CRS "GCS_...", datum "D_...").
	static auto LooksLikeEsri(Wkt::TokenWkt const& root) -> bool;
	// True when a PROJCS embeds a VERTCS (ESRI's way of writing a compound CRS; OGC uses COMPD_CS).
	static auto IsEsriCompound(Wkt::TokenWkt const& root) -> bool;

	static auto HorizontalWkt(int horizontalEpsgCode, std::string const& defecto, bool includeEpsgCode) -> std::string;
	static auto VerticalWkt(int verticalEpsgCode, std::string const& defecto, bool includeEpsgCode) -> std::string;
	static auto CompoundWkt(int horizontalEpsgCode, int verticalEpsgCode, std::string const& defecto) -> std::string;
	static auto FindHorizontalEpsgCode(std::string const& wkt) -> int;
	static auto FindVerticalEpsgCode(std::string const& wkt) -> int;
	static auto FindHorizontalEpsgCodeByName(std::string const& name) -> int;
	static auto FindVerticalEpsgCodeByName(std::string const& name) -> int;
	static auto NameFromEpsgCode(int code) -> std::string;
	static auto NameFromHorizontalEpsgCode(int code) -> std::string;
	static auto NameFromVerticalEpsgCode(int code) -> std::string;

private:
	static auto LoadHorizontalSystems() -> void;
	static auto LoadVerticalSystems() -> void;
};

}
