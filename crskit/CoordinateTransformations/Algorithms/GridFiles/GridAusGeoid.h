#pragma once

#include <filesystem>
#include <string>

#include "../../../Mathematics/GridInterpolation.h"
#include "../../../Mathematics/Point.h"
#include "../Geographic3DToGravityRelatedHeightEGM2008.h"

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	// Reads an AUSGeoid geoid grid: an NTv2 (".gsb") file repurposed to carry the geoid-ellipsoid
	// separation N (metres) in its first grid field, with -999 marking no-data (ocean). NTv2 orders
	// nodes south-to-north and, in its positive-west longitude convention, east-to-west; both are
	// flipped into the bilinear base's NW-origin grid. Handles a single subgrid (as AUSGeoid ships).
	class GridAusGeoid final
		: public Math::Transformations::GridInterpolation<float>
		, public IGridInterpolation
	{
		std::string _filePath;

	public:
		explicit GridAusGeoid(const char* file);

		auto Load(const char* gridFilePath) -> bool override;

		auto ComputeUndulation(Math::Point2D<double> const& geoSexa) -> float override;
		auto GetFilePath() const -> std::string override { return _filePath; }

		// True if `path` begins with an NTv2 overview header (so the geoid registry can recognise a
		// ".gsb" geoid grid by content). Full validation happens in Load.
		static auto looksLikeNtv2(std::filesystem::path const& path) -> bool;
	};
}
