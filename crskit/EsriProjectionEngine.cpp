#include "pch.h"
#include "EsriProjectionEngine.h"
#include "CrsContext.h"

using namespace std;
using namespace CrsKit;

// File obtenido de https://github.com/Esri/projection-engine-db-doc/blob/master/gdal/esri_epsg.GetWkt()

// Kept as file-static (not class members) so the exported EsriProjectionEngine has no STL data
// members crossing the DLL boundary (would otherwise trigger C4251).
namespace
{
	map<int, std::string> horizontalSystems;
	map<int, std::string> verticalSystems;

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
	if (horizontalSystems.size() == 0)
		LoadHorizontalSystems();

	if (horizontalSystems.contains(code))
	{
		auto const system = horizontalSystems[code];
		auto words = split(system, "\"");
		return words[1];
	}

	return "";
}

auto EsriProjectionEngine::NameFromVerticalEpsgCode(int code) -> std::string
{
	if (verticalSystems.size() == 0)
		LoadVerticalSystems();

	if (verticalSystems.contains(code))
	{
		auto const system = verticalSystems[code];
		auto words = split(system, "\"");
		return words[1];
	}

	return "";
}

auto EsriProjectionEngine::HorizontalWkt(int epsgCode, std::string const& defecto, bool includeEpsgCode) -> std::string
{
	if (horizontalSystems.empty())
		LoadHorizontalSystems();

	if (!horizontalSystems.contains(epsgCode))
		return defecto;

	auto system = horizontalSystems[epsgCode];

	if (!includeEpsgCode)
		system = system.substr(0, system.find(",AUTHORITY")) + "]";

	return system;
}

auto EsriProjectionEngine::VerticalWkt(int epsgCode, std::string const& defecto, bool includeEpsgCode) -> std::string
{
	if (verticalSystems.empty())
		LoadVerticalSystems();

	if (!verticalSystems.contains(epsgCode))
		return defecto;

	auto system = verticalSystems[epsgCode];

	if (includeEpsgCode)
		system = system.substr(0, system.size() - 1) + std::format(",AUTHORITY[\"EPSG\",{}]]", epsgCode);

	return system;
}

auto EsriProjectionEngine::CompoundWkt(int horizontalEpsgCode, int verticalEpsgCode, std::string const& defecto) -> std::string
{
	if (horizontalSystems.empty())
		LoadHorizontalSystems();

	if (verticalSystems.empty())
		LoadVerticalSystems();

	if (!horizontalSystems.contains(horizontalEpsgCode))
		return defecto;

	if (!verticalSystems.contains(verticalEpsgCode))
		return defecto;

	auto const horizontalSystem = horizontalSystems[horizontalEpsgCode];
	auto const verticalSystem = verticalSystems[verticalEpsgCode];

	return horizontalSystem + "," + verticalSystem;
}

auto EsriProjectionEngine::LoadHorizontalSystems() -> void
{
	// Load once even under concurrent first use (the file-static map has no other synchronization).
	static std::once_flag once;
	std::call_once(once, []
	{
		std::string const filePath = CrsKit::GetDefaultContext()->dataDirectory + "esri_epsg.wkt";

		try
		{
			std::ifstream file{std::filesystem::path(filePath)};
			if (!file.is_open())
				throw std::runtime_error("File could not be opened");
			std::string text;
			while (std::getline(file, text))
			{
				if (!text.empty() && text[0] == '#')
					continue;

				if (text.empty())
					continue;

				auto const primeraComa = text.find(',');
				auto epsgString = text.substr(0, primeraComa);

				horizontalSystems[parse_int(epsgString.c_str())] = text.substr(primeraComa + 1);
			}
		}
		catch (...)
		{
			// The ESRI WKT could not be loaded; it is ignored.
		}
	});
}

auto EsriProjectionEngine::LoadVerticalSystems() -> void
{
	static std::once_flag once;
	std::call_once(once, []
	{
		std::string const filePath = CrsKit::GetDefaultContext()->dataDirectory + "pe_list_vertcs.csv";

		try
		{
			std::ifstream file{std::filesystem::path(filePath)};
			if (!file.is_open())
				throw std::runtime_error("File could not be opened");
			std::string text;

			// Skip the first line
			if (!std::getline(file, text))
				return;

			while (std::getline(file, text))
			{
				if (text.empty())
					continue;

				auto words = split(text, ";");

				verticalSystems[parse_int(words[0].c_str())] = words[1];
			}
		}
		catch (...)
		{
			// The ESRI WKT could not be loaded; it is ignored.
		}
	});
}

auto EsriProjectionEngine::FindHorizontalEpsgCode(std::string const& wkt) -> int
{
	if (horizontalSystems.size() == 0)
		LoadHorizontalSystems();

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
	if (horizontalSystems.size() == 0)
		LoadHorizontalSystems();

	auto const it = std::ranges::find_if(horizontalSystems, [&](auto const& system)
	{
		return 0 == compareNoCase(name.c_str(), split(system.second, "\"")[1].c_str());
	});

	return it != horizontalSystems.end() ? it->first : 0;
}

auto EsriProjectionEngine::FindVerticalEpsgCodeByName(std::string const& name) -> int
{
	if (verticalSystems.size() == 0)
		LoadVerticalSystems();

	auto const it = std::ranges::find_if(verticalSystems, [&](auto const& system)
	{
		return 0 == compareNoCase(name.c_str(), split(system.second, "\"")[1].c_str());
	});

	return it != verticalSystems.end() ? it->first : 0;
}

auto EsriProjectionEngine::FindVerticalEpsgCode(std::string const& wkt) -> int
{
	if (verticalSystems.size() == 0)
		LoadVerticalSystems();

	auto const it = std::ranges::find_if(verticalSystems, [&](auto const& system)
	{
		return 0 == compareNoCase(system.second.c_str(), wkt.c_str());
	});

	return it != verticalSystems.end() ? it->first : 0;
}
