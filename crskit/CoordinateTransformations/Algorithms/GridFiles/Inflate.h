#pragma once

#include <cstddef>

#include "../../../CrsKitExport.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	// Decompress a raw DEFLATE stream (RFC 1951 — no zlib or gzip wrapper) into `dst`, which must be
	// pre-sized to the exact expected output length. Returns true only if the whole stream decodes and
	// fills `dst` exactly; false on malformed input, an over/under-run, or a bad back-reference.
	// A minimal self-contained inflate (after zlib's public-domain "puff" reference) so the library
	// stays dependency-free; used to read DEFLATE-compressed geodetic GeoTIFF (".tif") grid tiles.
	_modeCrsKit bool InflateRaw(unsigned char const* src, std::size_t srcLen, unsigned char* dst, std::size_t dstLen);
}
