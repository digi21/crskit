#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class Geographic3DToGravityRelatedHeightFile
	{
		struct Wgs84ToEgm96Header
		{
			int Width;
			int Height;
			float OriginLongitude;
			float CellLongitudeWidth;
			float OriginLatitude;
			float CellLatitudeWidth;
		};

		Wgs84ToEgm96Header _header;
		std::string _filePath;
		std::vector<float> _data;

	public:
		explicit Geographic3DToGravityRelatedHeightFile(std::string const& path);

		auto GetFilePath() const -> std::string;

		auto ComputeOffset(double longitude, double latitude) const -> float;
	};

	class Geographic3DToGravityRelatedHeightEGM final
	: public IMathTransform
	{

#pragma region Private fields
		bool _inverse;
		std::unique_ptr<Geographic3DToGravityRelatedHeightFile> _grid;
#pragma endregion

#pragma region Constructors

	public:
		explicit Geographic3DToGravityRelatedHeightEGM(std::string const& geoidModelFile);
		Geographic3DToGravityRelatedHeightEGM(std::string const& geoidModelFile, bool inverse);
		Geographic3DToGravityRelatedHeightEGM(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		explicit Geographic3DToGravityRelatedHeightEGM(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
#pragma endregion
#pragma region IMathTransform members

	public:
		auto GetWkt() const -> std::string override;

		auto GetSourceDimension() const -> int override;

		auto GetTargetDimension() const -> int override;

		auto GetIsIdentity() const -> bool override;

		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;

		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
