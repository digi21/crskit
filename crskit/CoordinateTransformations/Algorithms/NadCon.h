#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class NadConFile
	{
		struct NADCON_HEADER
		{
			char NadconExtractedRegion[56];
			char Nadgrd[8];
			int Width;
			int Height;
			int ZCount;
			float OriginLongitude;
			float CellLongitudeWidth;
			float OriginLatitude;
			float CellLatitudeWidth;
			float Angle;
		};

		NADCON_HEADER _header;
		std::string _filePath;
		std::vector<float> _data;

	public:
		explicit NadConFile(std::string const& path);

		auto GetFilePath() const -> std::string;

		auto ComputeOffset(double longitude, double latitude) const -> float;
	};

	class NadCon final
	: public IMathTransform
	{

#pragma region Private fields
		bool _inverse;
		std::unique_ptr<NadConFile> _gridLatitud;
		std::unique_ptr<NadConFile> _gridLongitude;
#pragma endregion

#pragma region Constructors

	public:
		NadCon(std::string const& latitudeDifferenceFile, std::string const& longitudeDifferenceFile);
		NadCon(std::string const& latitudeDifferenceFile, std::string const& longitudeDifferenceFile, bool inverse);
		NadCon(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		explicit NadCon(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
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
