#include "pch.h"
#include "EsriProjectionEngine.h"
#include "CrsContext.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <mutex>

using namespace std;
using namespace CrsKit;

// File obtenido de https://github.com/Esri/projection-engine-db-doc/blob/master/gdal/esri_epsg.GetWkt()

// Kept as file-static (not class members) so the exported EsriProjectionEngine has no STL data
// members crossing the DLL boundary (would otherwise trigger C4251).
namespace
{
	map<int, std::string> horizontalSystems;
	map<int, std::string> verticalSystems;
	std::atomic<bool> horizontalLoaded{ false };
	std::atomic<bool> verticalLoaded{ false };
	std::mutex loadMutex;

	// Fills 'table' from the file 'fileName' of the data directory the first time it can be opened. Until
	// then every call retries; once 'loaded' is set, 'table' is never written again and is read without lock.
	template <typename Read>
	auto LoadTable(std::atomic<bool>& loaded, map<int, std::string>& table, char const* fileName, Read read) -> bool
	{
		if (loaded.load(std::memory_order_acquire))
			return true;

		std::scoped_lock const lock{ loadMutex };
		if (loaded.load(std::memory_order_relaxed))
			return true;

		std::ifstream file{ std::filesystem::path(CrsKit::GetDefaultContext()->dataDirectory + fileName) };
		if (!file.is_open())
			return false;

		map<int, std::string> loadedTable;
		read(file, loadedTable);
		table = std::move(loadedTable);
		loaded.store(true, std::memory_order_release);
		return true;
	}

	// The quoted name of a WKT node, e.g. GEOGCS["GCS_WGS_1984",...] -> "GCS_WGS_1984".
	auto nodeName(Wkt::TokenWkt const& node) -> std::string
	{
		return node.GetChildCount() > 0 ? node.GetChild(0).GetString() : std::string{};
	}

	// ESRI names its geographic CRS "GCS_..." and its datums "D_..."; OGC WKT1 does not.
	auto isEsriGeographic(Wkt::TokenWkt const& geogcs) -> bool
	{
		if (nodeName(geogcs).starts_with("GCS_"))
			return true;
		return geogcs.HasChild("DATUM") && nodeName(geogcs.GetChild("DATUM")).starts_with("D_");
	}
}

auto EsriProjectionEngine::LooksLikeEsri(Wkt::TokenWkt const& root) -> bool
{
	if (0 == compareNoCase(root.GetName().c_str(), "GEOGCS"))
		return isEsriGeographic(root);
	if (0 == compareNoCase(root.GetName().c_str(), "PROJCS"))
		return root.GetChildCount() > 1 && isEsriGeographic(root.GetChild(1));
	return false;
}

auto EsriProjectionEngine::IsEsriCompound(Wkt::TokenWkt const& root) -> bool
{
	// ESRI stores a compound CRS as a VERTCS embedded in the PROJCS (there is no COMPD_CS).
	return 0 == compareNoCase(root.GetName().c_str(), "PROJCS") && root.HasChild("VERTCS");
}

auto EsriProjectionEngine::NameFromEpsgCode(int code) -> std::string
{
	auto name = NameFromHorizontalEpsgCode(code);
	if (name.size() != 0)
		return name;

	name = NameFromVerticalEpsgCode(code);
	if (name.size() != 0)
		return name;

	return "";
}

auto EsriProjectionEngine::NameFromHorizontalEpsgCode(int code) -> std::string
{
	if (!LoadHorizontalSystems())
		return {};

	if (horizontalSystems.contains(code))
	{
		auto const system = horizontalSystems.at(code);
		auto words = split(system, "\"");
		return words[1];
	}

	return "";
}

auto EsriProjectionEngine::NameFromVerticalEpsgCode(int code) -> std::string
{
	if (!LoadVerticalSystems())
		return {};

	if (verticalSystems.contains(code))
	{
		auto const system = verticalSystems.at(code);
		auto words = split(system, "\"");
		return words[1];
	}

	return "";
}

auto EsriProjectionEngine::HorizontalWkt(int epsgCode, std::string const& defecto, bool includeEpsgCode) -> std::string
{
	if (!LoadHorizontalSystems() || !horizontalSystems.contains(epsgCode))
		return defecto;

	auto system = horizontalSystems.at(epsgCode);

	if (!includeEpsgCode)
		system = system.substr(0, system.find(",AUTHORITY")) + "]";

	return system;
}

auto EsriProjectionEngine::VerticalWkt(int epsgCode, std::string const& defecto, bool includeEpsgCode) -> std::string
{
	if (!LoadVerticalSystems() || !verticalSystems.contains(epsgCode))
		return defecto;

	auto system = verticalSystems.at(epsgCode);

	if (includeEpsgCode)
		system = system.substr(0, system.size() - 1) + std::format(",AUTHORITY[\"EPSG\",{}]]", epsgCode);

	return system;
}

auto EsriProjectionEngine::CompoundWkt(int horizontalEpsgCode, int verticalEpsgCode, std::string const& defecto) -> std::string
{
	if (!LoadHorizontalSystems() || !LoadVerticalSystems())
		return defecto;

	if (!horizontalSystems.contains(horizontalEpsgCode))
		return defecto;

	if (!verticalSystems.contains(verticalEpsgCode))
		return defecto;

	auto const horizontalSystem = horizontalSystems.at(horizontalEpsgCode);
	auto const verticalSystem = verticalSystems.at(verticalEpsgCode);

	return horizontalSystem + "," + verticalSystem;
}

auto EsriProjectionEngine::LoadHorizontalSystems() -> bool
{
	return LoadTable(horizontalLoaded, horizontalSystems, "esri_epsg.wkt", [](std::ifstream& file, map<int, std::string>& table)
	{
		std::string text;
		while (std::getline(file, text))
		{
			if (text.empty() || text[0] == '#')
				continue;
			auto const primeraComa = text.find(',');
			if (std::string::npos == primeraComa)
				continue;
			try
			{
				table[parse_int(text.substr(0, primeraComa).c_str())] = text.substr(primeraComa + 1);
			}
			catch (...)
			{
				// A malformed line is skipped.
			}
		}
	});
}

auto EsriProjectionEngine::LoadVerticalSystems() -> bool
{
	return LoadTable(verticalLoaded, verticalSystems, "pe_list_vertcs.csv", [](std::ifstream& file, map<int, std::string>& table)
	{
		std::string text;
		// The first line is the header.
		if (!std::getline(file, text))
			return;
		while (std::getline(file, text))
		{
			auto const words = split(text, ";");
			if (words.size() < 2)
				continue;
			try
			{
				table[parse_int(words[0].c_str())] = words[1];
			}
			catch (...)
			{
				// A malformed line is skipped.
			}
		}
	});
}

auto EsriProjectionEngine::FindHorizontalEpsgCode(std::string const& wkt) -> int
{
	if (!LoadHorizontalSystems())
		return {};

	auto const it = std::ranges::find_if(horizontalSystems, [&](auto const& system)
	{
		if (0 == compareNoCase(system.second.c_str(), wkt.c_str()))
			return true;

		auto const wktWithoutEpsgCode = system.second.substr(0, system.second.find(",AUTHORITY")) + "]";
		return 0 == compareNoCase(wktWithoutEpsgCode.c_str(), wkt.c_str());
	});

	return it != horizontalSystems.end() ? it->first : 0;
}

auto EsriProjectionEngine::FindHorizontalEpsgCodeByName(std::string const& name) -> int
{
	if (!LoadHorizontalSystems())
		return {};

	auto const it = std::ranges::find_if(horizontalSystems, [&](auto const& system)
	{
		return 0 == compareNoCase(name.c_str(), split(system.second, "\"")[1].c_str());
	});

	return it != horizontalSystems.end() ? it->first : 0;
}

auto EsriProjectionEngine::FindVerticalEpsgCodeByName(std::string const& name) -> int
{
	if (!LoadVerticalSystems())
		return {};

	auto const it = std::ranges::find_if(verticalSystems, [&](auto const& system)
	{
		return 0 == compareNoCase(name.c_str(), split(system.second, "\"")[1].c_str());
	});

	return it != verticalSystems.end() ? it->first : 0;
}

auto EsriProjectionEngine::FindVerticalEpsgCode(std::string const& wkt) -> int
{
	if (!LoadVerticalSystems())
		return {};

	auto const it = std::ranges::find_if(verticalSystems, [&](auto const& system)
	{
		return 0 == compareNoCase(system.second.c_str(), wkt.c_str());
	});

	return it != verticalSystems.end() ? it->first : 0;
}
