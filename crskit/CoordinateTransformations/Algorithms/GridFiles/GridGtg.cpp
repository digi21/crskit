#include "pch.h"
#include "GridGtg.h"
#include "Inflate.h"
#include "../../../CrsContext.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	namespace
	{
		// TIFF tags used by geodetic GeoTIFF grids.
		enum Tag : std::uint16_t
		{
			ImageWidth = 256, ImageLength = 257, BitsPerSample = 258, Compression = 259,
			SamplesPerPixel = 277, Predictor = 317, TileWidth = 322, TileLength = 323,
			TileOffsets = 324, TileByteCounts = 325, SampleFormat = 339,
			ModelPixelScale = 33550, ModelTiepoint = 33922, GeoKeyDirectory = 34735, GdalNoData = 42113,
		};

		// Reads the TIFF byte-order-dependent scalars from an in-memory file.
		struct TiffReader
		{
			std::vector<unsigned char> const& d;
			bool little{ true };

			auto u16(std::size_t o) const -> std::uint32_t
			{
				if (o + 2 > d.size()) return 0;
				return little ? (d[o] | (d[o + 1] << 8)) : (d[o + 1] | (d[o] << 8));
			}
			auto u32(std::size_t o) const -> std::uint32_t
			{
				if (o + 4 > d.size()) return 0;
				return little
					? (d[o] | (d[o + 1] << 8) | (d[o + 2] << 16) | (static_cast<std::uint32_t>(d[o + 3]) << 24))
					: (d[o + 3] | (d[o + 2] << 8) | (d[o + 1] << 16) | (static_cast<std::uint32_t>(d[o]) << 24));
			}
			auto f64(std::size_t o) const -> double
			{
				if (o + 8 > d.size()) return 0.0;
				std::array<unsigned char, 8> b{};
				for (int i = 0; i < 8; ++i) b[i] = d[o + i];
				if (!little) std::ranges::reverse(b);
				double v{}; std::memcpy(&v, b.data(), 8); return v;
			}
		};

		struct Entry { std::uint16_t type; std::uint32_t count; std::size_t valueOffset; };

		auto typeSize(std::uint16_t type) -> std::size_t
		{
			switch (type) { case 1: case 2: return 1; case 3: return 2; case 4: case 11: return 4;
				case 5: case 12: return 8; default: return 1; }
		}

		// Parse the first IFD into a tag -> entry map. Entry.valueOffset points at the value(s),
		// resolved through the out-of-line offset when they do not fit inline.
		auto parseIfd(TiffReader const& t) -> std::optional<std::unordered_map<std::uint16_t, Entry>>
		{
			if (t.d.size() < 8) return std::nullopt;
			auto const ifd = t.u32(4);
			if (ifd + 2 > t.d.size()) return std::nullopt;
			auto const n = t.u16(ifd);
			std::unordered_map<std::uint16_t, Entry> tags;
			for (std::uint32_t i = 0; i < n; ++i)
			{
				auto const e = ifd + 2 + static_cast<std::size_t>(i) * 12;
				if (e + 12 > t.d.size()) return std::nullopt;
				auto const tag = static_cast<std::uint16_t>(t.u16(e));
				auto const type = static_cast<std::uint16_t>(t.u16(e + 2));
				auto const count = t.u32(e + 4);
				auto const bytes = typeSize(type) * count;
				tags[tag] = Entry{ type, count, bytes <= 4 ? e + 8 : t.u32(e + 8) };
			}
			return tags;
		}

		auto intAt(TiffReader const& t, Entry const& e, std::uint32_t i) -> std::uint32_t
		{
			return e.type == 3 ? t.u16(e.valueOffset + 2 * i) : t.u32(e.valueOffset + 4 * i);
		}

		constexpr float NaNf = std::numeric_limits<float>::quiet_NaN();

		// Interpret four big-endian bytes as a float.
		auto floatBE(unsigned char const* p) -> float
		{
			std::uint32_t const u = (static_cast<std::uint32_t>(p[0]) << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
			return std::bit_cast<float>(u);
		}
	}

	GridGtg::GridGtg(const char* file)
	{
		_filePath = GetDefaultContext()->dataDirectory + file;
		if (!std::filesystem::exists(std::filesystem::path(_filePath)))
			throw CrsKit::GridFileNotFoundException{ file, _filePath };
		if (!Load(_filePath.c_str()))
			throw UnsupportedFormatException(std::format("Unsupported or malformed GeoTIFF geoid grid: {}", file));
	}

	auto GridGtg::Load(const char* gridFilePath) -> bool
	{
		std::ifstream is{ std::filesystem::path(gridFilePath), std::ios::binary | std::ios::ate };
		if (!is.is_open()) return false;
		std::vector<unsigned char> data(static_cast<std::size_t>(is.tellg()));
		is.seekg(0);
		is.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));

		TiffReader t{ data };
		if (data.size() < 8) return false;
		if (data[0] == 'I' && data[1] == 'I') t.little = true;
		else if (data[0] == 'M' && data[1] == 'M') t.little = false;
		else return false;
		if (t.u16(2) != 42) return false;              // classic TIFF only (not BigTIFF)

		auto const tagsOpt = parseIfd(t);
		if (!tagsOpt) return false;
		auto const& tags = *tagsOpt;
		auto has = [&](std::uint16_t tag) { return tags.contains(tag); };
		auto first = [&](std::uint16_t tag) { return intAt(t, tags.at(tag), 0); };

		// Only the PROJ grid subset: a single float32 sample, tiled, deflate (or none), predictor 1/3.
		if (!has(ImageWidth) || !has(ImageLength) || !has(TileWidth) || !has(TileLength) ||
			!has(TileOffsets) || !has(TileByteCounts) || !has(ModelPixelScale) || !has(ModelTiepoint))
			return false;
		if (has(SamplesPerPixel) && first(SamplesPerPixel) != 1) return false;
		if (has(BitsPerSample) && first(BitsPerSample) != 32) return false;
		if (has(SampleFormat) && first(SampleFormat) != 3) return false;   // IEEE float
		auto const compression = has(Compression) ? first(Compression) : 1u;
		if (compression != 1 && compression != 8 && compression != 32946) return false; // none / deflate
		auto const predictor = has(Predictor) ? first(Predictor) : 1u;
		if (predictor != 1 && predictor != 3) return false;

		auto const width = static_cast<int>(first(ImageWidth));
		auto const height = static_cast<int>(first(ImageLength));
		auto const tileW = static_cast<int>(first(TileWidth));
		auto const tileH = static_cast<int>(first(TileLength));
		if (width < 2 || height < 2 || tileW < 1 || tileH < 1) return false;

		auto const& tScale = tags.at(ModelPixelScale);
		auto const& tTie = tags.at(ModelTiepoint);
		if (tScale.count < 2 || tTie.count < 6) return false;
		auto const scaleX = t.f64(tScale.valueOffset);
		auto const scaleY = t.f64(tScale.valueOffset + 8);
		auto const originX = t.f64(tTie.valueOffset + 24);   // tiepoint model x (assumes raster tie (0,0))
		auto const originY = t.f64(tTie.valueOffset + 32);   // tiepoint model y
		if (!(scaleX > 0.0) || !(scaleY > 0.0)) return false;

		float noData = NaNf;
		if (has(GdalNoData))
		{
			auto const& e = tags.at(GdalNoData);
			std::string s(reinterpret_cast<char const*>(data.data()) + e.valueOffset, e.count);
			try { noData = std::stof(s); } catch (...) {}
		}

		_horizontalCellCount = width;
		_verticalCellCount = height;
		_cellSize = { scaleX, scaleY };
		_origin = { originX, originY };   // GeoTIFF pixel-is-point node grid; row 0 = north (top)
		CreateCorrectionsArray();

		auto const& tOff = tags.at(TileOffsets);
		auto const& tBc = tags.at(TileByteCounts);
		auto const tilesAcross = (width + tileW - 1) / tileW;
		auto const tilesDown = (height + tileH - 1) / tileH;
		if (tOff.count < static_cast<std::uint32_t>(tilesAcross * tilesDown)) return false;

		std::vector<unsigned char> tile(static_cast<std::size_t>(tileW) * tileH * 4);
		std::vector<unsigned char> row(static_cast<std::size_t>(tileW) * 4);
		for (int ty = 0; ty < tilesDown; ++ty)
			for (int tx = 0; tx < tilesAcross; ++tx)
			{
				auto const ti = static_cast<std::uint32_t>(ty) * tilesAcross + tx;
				auto const off = intAt(t, tOff, ti);
				auto const bc = intAt(t, tBc, ti);
				if (off + bc > data.size()) return false;

				unsigned char const* src = data.data() + off;
				std::size_t srcLen = bc;
				if (compression != 1)                       // deflate: skip the 2-byte zlib header
				{
					if (bc < 2) return false;
					src += 2; srcLen -= 2;
					if (!InflateRaw(src, srcLen, tile.data(), tile.size())) return false;
				}
				else
				{
					if (bc < tile.size()) return false;
					std::memcpy(tile.data(), src, tile.size());
				}

				if (predictor == 3)                         // TIFF floating-point predictor
					for (int r = 0; r < tileH; ++r)
					{
						unsigned char* p = tile.data() + static_cast<std::size_t>(r) * tileW * 4;
						for (int i = 1; i < tileW * 4; ++i) p[i] = static_cast<unsigned char>(p[i] + p[i - 1]);
						for (int s = 0; s < tileW; ++s)
							for (int b = 0; b < 4; ++b)
								row[static_cast<std::size_t>(s) * 4 + b] = p[static_cast<std::size_t>(b) * tileW + s];
						std::memcpy(p, row.data(), row.size());
					}

				for (int r = 0; r < tileH; ++r)
				{
					int const imageRow = ty * tileH + r;
					if (imageRow >= height) break;
					for (int c = 0; c < tileW; ++c)
					{
						int const imageCol = tx * tileW + c;
						if (imageCol >= width) continue;
						unsigned char const* p = tile.data() + (static_cast<std::size_t>(r) * tileW + c) * 4;
						// After predictor 3 the samples are big-endian; uncompressed data follows the
						// file byte order.
						float v;
						if (predictor == 3 || !t.little) v = floatBE(p);
						else { std::memcpy(&v, p, 4); }
						correction(imageRow, imageCol) = (v == noData) ? NaNf : v;
					}
				}
			}

		return true;
	}

	auto GridGtg::ComputeUndulation(Point2D<double> const& geoSexa) -> float
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

	auto GridGtg::looksLikeGtg(std::filesystem::path const& path) -> bool
	{
		std::ifstream is{ path, std::ios::binary };
		if (!is.is_open()) return false;
		unsigned char h[4]{};
		is.read(reinterpret_cast<char*>(h), 4);
		if (!is) return false;
		bool const little = h[0] == 'I' && h[1] == 'I';
		bool const big = h[0] == 'M' && h[1] == 'M';
		if (!little && !big) return false;
		std::uint16_t const magic = little ? (h[2] | (h[3] << 8)) : (h[3] | (h[2] << 8));
		return magic == 42;   // classic TIFF
	}
}
