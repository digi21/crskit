#pragma once

#include <filesystem>
#include <string>

#include "../../../Mathematics/GridInterpolation.h"
#include "../../../Mathematics/Point.h"
#include "../Geographic3DToGravityRelatedHeightEGM2008.h"

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	// Reads a geodetic GeoTIFF (".tif") geoid grid, the format PROJ/GDAL standardised on and that
	// cdn.proj.org serves the whole grid catalogue as: a single-sample float32 TIFF, tiled and
	// DEFLATE-compressed (usually with the TIFF floating-point predictor), georeferenced with
	// ModelPixelScale + ModelTiepoint. Only that subset is handled; anything else is rejected. The
	// grid is self-describing, so its geometry comes from the file. Reuses the bilinear base.
	class GridGtg final
		: public Math::Transformations::GridInterpolation<float>
		, public IGridInterpolation
	{
		std::string _filePath;

	public:
		explicit GridGtg(const char* file);

		auto Load(const char* gridFilePath) -> bool override;

		auto ComputeUndulation(Math::Point2D<double> const& geoSexa) -> float override;
		auto GetFilePath() const -> std::string override { return _filePath; }

		// True if `path` starts with a classic-TIFF signature (so the grid registry can recognise the
		// format by content). The full validation happens in Load.
		static auto looksLikeGtg(std::filesystem::path const& path) -> bool;
	};
}
