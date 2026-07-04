#pragma once

#include <string>

#include "../../../Mathematics/GridInterpolation.h"
#include "../../../Mathematics/Point.h"
#include "../Geographic3DToGravityRelatedHeightEGM2008.h"

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	///	<summary>
	///		This class implements a bilinear interpolation grid for computing the geoid undulation.
	///		Loads ASCII files in the Spanish IGN format.
	///		Verified with the PAG program (Programa de Aplicaciones Geodesicas) giving the same results.
	///	</summary>
	class GridEGM2008Rednap final
	: public Math::Transformations::GridInterpolation<float>
	, public IGridInterpolation
	{
#pragma region Fields
		std::string _filePath;
#pragma endregion

#pragma region Constructor

	public:
		explicit GridEGM2008Rednap(const char* file);
#pragma endregion
#pragma region GridInterpolation members

	public:
		auto Load(const char* gridFilePath) -> bool override;
#pragma endregion

#pragma region IGridInterpolation members

	public:
		auto ComputeUndulation(Math::Point2D<double> const& geoSexa) -> float override;
		auto GetFilePath() const -> std::string override { return _filePath; }
#pragma endregion
	};
}
