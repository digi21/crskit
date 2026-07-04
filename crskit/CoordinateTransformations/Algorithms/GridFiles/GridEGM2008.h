#pragma once

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

#include "../../../Mathematics/Point.h"
#include "../../../Mathematics/Window2D.h"
#include "../Geographic3DToGravityRelatedHeightEGM2008.h"

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	class GridEgm2008 : public IGridInterpolation
	{
#pragma region Private fields
		int _rows;
		int _columns;
		std::string _fileName;
		std::ifstream _file;
		std::mutex _sc;
		Math::Window2D<long> _maxminCache;
		std::vector<float> _cache;
#pragma endregion

#pragma region Constructors and destructor

	public:
		GridEgm2008(std::string const& fileName, int rows, int columns);
#pragma endregion

#pragma region IGridInterpolation members

	public:
		auto ComputeUndulation(Math::Point2D<double> const& geoSexa) -> float override;
		auto GetFilePath() const -> std::string override { return _fileName + ".gz"; }
#pragma endregion
#pragma region Private methods

	private:
		auto LoadCache(long column, long row) -> void;
#pragma endregion
	};
}
