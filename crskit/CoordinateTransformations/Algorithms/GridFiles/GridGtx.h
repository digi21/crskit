#pragma once

#include <filesystem>
#include <string>

#include "../../../Mathematics/GridInterpolation.h"
#include "../../../Mathematics/Point.h"
#include "../Geographic3DToGravityRelatedHeightEGM2008.h"

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	// Reads a NOAA/PROJ ".gtx" geoid grid: a big-endian header (south-edge latitude, west-edge
	// longitude, latitude and longitude spacing as float64, then row and column counts as int32),
	// followed by rows*cols big-endian float32 undulation values ordered south-to-north, west-to-east.
	// The file is self-describing, so the geometry comes from its header. Reuses the bilinear base.
	class GridGtx final
		: public Math::Transformations::GridInterpolation<float>
		, public IGridInterpolation
	{
		std::string _filePath;

	public:
		explicit GridGtx(const char* file);

		auto Load(const char* gridFilePath) -> bool override;

		auto ComputeUndulation(Math::Point2D<double> const& geoSexa) -> float override;
		auto GetFilePath() const -> std::string override { return _filePath; }

		// True if `path` is a self-consistent GTX file: a valid header whose geometry matches the
		// exact file size. Used to recognise the format by content, independent of the file name.
		static auto looksLikeGtx(std::filesystem::path const& path) -> bool;
	};
}
