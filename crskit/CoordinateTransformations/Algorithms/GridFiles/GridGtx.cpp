#include "pch.h"
#include "GridGtx.h"
#include "../../../CrsContext.h"

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <optional>
#include <ranges>

using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	namespace
	{
		// GTX header: 4 float64 + 2 int32, all big-endian = 40 bytes; data is rows*cols float32.
		constexpr std::size_t GTX_HEADER_BYTES = 4 * sizeof(double) + 2 * sizeof(std::int32_t);

		// Read one big-endian value of a trivially-copyable type from the stream.
		template <typename T>
		auto readBigEndian(std::istream& is) -> T
		{
			std::array<std::byte, sizeof(T)> bytes{};
			is.read(reinterpret_cast<char*>(bytes.data()), sizeof(T));
			if constexpr (std::endian::native == std::endian::little)
				std::ranges::reverse(bytes);
			T value{};
			std::memcpy(&value, bytes.data(), sizeof(T));
			return value;
		}

		struct GtxHeader
		{
			double southLatitude;   // latitude of the south (lower) edge
			double westLongitude;   // longitude of the west (left) edge
			double latitudeStep;
			double longitudeStep;
			std::int32_t rows;      // number of latitude rows
			std::int32_t columns;   // number of longitude columns
		};

		auto readHeader(std::istream& is) -> std::optional<GtxHeader>
		{
			GtxHeader h{};
			h.southLatitude = readBigEndian<double>(is);
			h.westLongitude = readBigEndian<double>(is);
			h.latitudeStep = readBigEndian<double>(is);
			h.longitudeStep = readBigEndian<double>(is);
			h.rows = readBigEndian<std::int32_t>(is);
			h.columns = readBigEndian<std::int32_t>(is);
			if (!is || h.rows < 2 || h.columns < 2 || !(h.latitudeStep > 0.0) || !(h.longitudeStep > 0.0))
				return std::nullopt;
			return h;
		}
	}

	GridGtx::GridGtx(const char* file)
	{
		_filePath = GetDefaultContext()->dataDirectory + file;
		if (!std::filesystem::exists(std::filesystem::path(_filePath)))
			throw CrsKit::GridFileNotFoundException{ file, _filePath };
		Load(_filePath.c_str());
	}

	auto GridGtx::Load(const char* gridFilePath) -> bool
	{
		std::ifstream is{ std::filesystem::path(gridFilePath), std::ios::binary };
		if (!is.is_open())
			return false;

		auto const header = readHeader(is);
		if (!header)
			return false;

		_horizontalCellCount = header->columns;
		_verticalCellCount = header->rows;
		_cellSize = { header->longitudeStep, header->latitudeStep };
		// The bilinear base treats _origin as the NW corner (row 0 = north); the file is south-to-north.
		_origin = { header->westLongitude, header->southLatitude + (header->rows - 1) * header->latitudeStep };
		CreateCorrectionsArray();

		for (int row = 0; row < header->rows; ++row)      // row 0 is the SOUTH row in the file
		{
			auto const destRow = header->rows - 1 - row;  // flip so destRow 0 is the NORTH row
			for (int column = 0; column < header->columns; ++column)
			{
				auto const value = readBigEndian<float>(is);
				if (!is)
					return false;
				correction(destRow, column) = value;
			}
		}
		return true;
	}

	auto GridGtx::ComputeUndulation(Point2D<double> const& geoSexa) -> float
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

	auto GridGtx::looksLikeGtx(std::filesystem::path const& path) -> bool
	{
		std::error_code ec;
		auto const size = std::filesystem::file_size(path, ec);
		if (ec || size < GTX_HEADER_BYTES)
			return false;

		std::ifstream is{ path, std::ios::binary };
		if (!is.is_open())
			return false;

		auto const header = readHeader(is);
		if (!header)
			return false;

		// The declared geometry must account for exactly the whole file.
		auto const expected = GTX_HEADER_BYTES
			+ static_cast<std::uintmax_t>(header->rows) * header->columns * sizeof(float);
		return expected == size;
	}
}
