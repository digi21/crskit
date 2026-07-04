#include "pch.h"
#include "GridAusGeoid.h"
#include "../../../CrsContext.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <format>
#include <fstream>
#include <limits>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	namespace
	{
		constexpr int RECORD = 16;          // NTv2 record: 8-byte name + 8-byte value
		constexpr int OVERVIEW_RECORDS = 11;
		constexpr int SUBGRID_RECORDS = 11;
		constexpr float NoDataFlag = -999.0f;
		constexpr float NaNf = std::numeric_limits<float>::quiet_NaN();

		template <typename T>
		auto read(unsigned char const* p, bool little) -> T
		{
			std::array<unsigned char, sizeof(T)> b{};
			std::memcpy(b.data(), p, sizeof(T));
			if (!little) std::ranges::reverse(b);
			T v{}; std::memcpy(&v, b.data(), sizeof(T));
			return v;
		}
	}

	GridAusGeoid::GridAusGeoid(const char* file)
	{
		_filePath = GetDefaultContext()->dataDirectory + file;
		if (!std::filesystem::exists(std::filesystem::path(_filePath)))
			throw CrsKit::GridFileNotFoundException{ file, _filePath };
		if (!Load(_filePath.c_str()))
			throw UnsupportedFormatException(std::format("Unsupported or malformed NTv2 geoid grid: {}", file));
	}

	auto GridAusGeoid::Load(const char* gridFilePath) -> bool
	{
		std::ifstream is{ std::filesystem::path(gridFilePath), std::ios::binary | std::ios::ate };
		if (!is.is_open()) return false;
		std::vector<unsigned char> data(static_cast<std::size_t>(is.tellg()));
		is.seekg(0);
		is.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
		if (data.size() < static_cast<std::size_t>((OVERVIEW_RECORDS + SUBGRID_RECORDS) * RECORD)) return false;

		// NUM_OREC (record 0 value) is 11; use it to pick the byte order.
		bool little = true;
		if (read<std::int32_t>(data.data() + 8, true) == OVERVIEW_RECORDS) little = true;
		else if (read<std::int32_t>(data.data() + 8, false) == OVERVIEW_RECORDS) little = false;
		else return false;

		auto const subgrid = static_cast<std::size_t>(OVERVIEW_RECORDS) * RECORD;   // first subgrid header
		auto dval = [&](int record) { return read<double>(data.data() + subgrid + record * RECORD + 8, little); };
		auto const sLat = dval(4), nLat = dval(5), eLong = dval(6), wLong = dval(7);
		auto const latInc = dval(8), longInc = dval(9);
		auto const gsCount = read<std::int32_t>(data.data() + subgrid + 10 * RECORD + 8, little);
		if (!(latInc > 0.0) || !(longInc > 0.0)) return false;

		auto const columns = static_cast<int>(std::lround((wLong - eLong) / longInc)) + 1;   // positive-west
		auto const rows = static_cast<int>(std::lround((nLat - sLat) / latInc)) + 1;
		if (columns < 2 || rows < 2 || static_cast<std::int64_t>(columns) * rows != gsCount) return false;

		auto const grid = subgrid + static_cast<std::size_t>(SUBGRID_RECORDS) * RECORD;
		if (grid + static_cast<std::size_t>(gsCount) * 16 > data.size()) return false;

		_horizontalCellCount = columns;
		_verticalCellCount = rows;
		_cellSize = { longInc / 3600.0, latInc / 3600.0 };
		_origin = { -wLong / 3600.0, nLat / 3600.0 };   // NW corner (west edge east-longitude, north lat)
		CreateCorrectionsArray();

		// NTv2 node order: row 0 = south, column 0 = east (positive-west increases westward). Flip both
		// into the base grid, whose row 0 is north and column 0 is west. The separation N is field 0.
		for (int ntRow = 0; ntRow < rows; ++ntRow)
			for (int ntCol = 0; ntCol < columns; ++ntCol)
			{
				auto const node = grid + (static_cast<std::size_t>(ntRow) * columns + ntCol) * 16;
				auto const n = read<float>(data.data() + node, little);
				correction(rows - 1 - ntRow, columns - 1 - ntCol) = (n == NoDataFlag) ? NaNf : n;
			}

		return true;
	}

	auto GridAusGeoid::ComputeUndulation(Point2D<double> const& geoSexa) -> float
	{
		try
		{
			return ComputeBilinearCorrections(geoSexa);
		}
		catch (CrsKit::OpenGisException const&)
		{
			throw CoordinateOutsideDomainException(std::format(
				"The geographical coordinates Longitude={:f}, Latitude={:f} are outside the range covered by the geoid model file: \"{}\".",
				geoSexa.x,
				geoSexa.y,
				GetFilePath().c_str()));
		}
	}

	auto GridAusGeoid::looksLikeNtv2(std::filesystem::path const& path) -> bool
	{
		std::ifstream is{ path, std::ios::binary };
		if (!is.is_open()) return false;
		char name[9]{};
		is.read(name, 8);
		if (!is) return false;
		return std::string_view{ name, 8 } == "NUM_OREC";
	}
}
