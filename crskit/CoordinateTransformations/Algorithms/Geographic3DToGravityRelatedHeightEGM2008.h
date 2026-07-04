#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../../Mathematics/Point.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	struct IGridInterpolation
	{
		virtual ~IGridInterpolation() = default;

		virtual auto GetFilePath() const -> std::string = 0;

		virtual auto ComputeUndulation(Math::Point2D<double> const& geoSexa) -> float = 0;
	};

	// Applies a geoid undulation (ellipsoidal <-> gravity-related height). The grid file format is
	// picked from the file itself (see CreateLoaderFromFileName), so several EPSG methods that share
	// this algorithm and differ only in the grid format reuse this class through thin subclasses
	// below (each a distinct type, as the transform factory keys registrations by type).
	class Geographic3DToGravityRelatedHeightEGM2008
	: public IMathTransform
	{

#pragma region Private fields
		bool _inverse;
		// The WKT method name to emit (from the operation this was built for): the algorithm — apply a
		// geoid undulation — is shared by several EPSG methods that differ only in the grid file format,
		// so the name is carried rather than hard-coded.
		std::string _methodName;
		std::shared_ptr<IGridInterpolation> _grid;

		Geographic3DToGravityRelatedHeightEGM2008(std::string const& methodName, std::string const& geoidModelFile, bool inverse);
#pragma endregion

#pragma region Constructors

	public:
		explicit Geographic3DToGravityRelatedHeightEGM2008(std::string const& geoidModelFile);
		Geographic3DToGravityRelatedHeightEGM2008(std::string const& geoidModelFile, bool inverse);
		Geographic3DToGravityRelatedHeightEGM2008(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		explicit Geographic3DToGravityRelatedHeightEGM2008(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
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

	// Same algorithm, registered for the EPSG "(gtx)" geoid method (a distinct type so the transform
	// factory registers it separately). The ".gtx" grid reader is selected by CreateLoaderFromFileName.
	class Geographic3DToGravityRelatedHeightGtx final : public Geographic3DToGravityRelatedHeightEGM2008
	{
	public:
		using Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008;
	};

	// Same algorithm, registered for the EPSG "(gtg)" geoid method: a geodetic GeoTIFF (".tif") grid
	// (the format PROJ/GDAL standardised on). The reader is selected by CreateLoaderFromFileName.
	class Geographic3DToGravityRelatedHeightGtg final : public Geographic3DToGravityRelatedHeightEGM2008
	{
	public:
		using Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008;
	};

	// Same algorithm for the two AUSGeoid methods (98 and v2): an NTv2 (".gsb") geoid grid. Distinct
	// types so the transform factory registers each separately; the reader is content-detected.
	class Geographic3DToGravityRelatedHeightAusGeoid98 final : public Geographic3DToGravityRelatedHeightEGM2008
	{
	public:
		using Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008;
	};

	class Geographic3DToGravityRelatedHeightAusGeoidV2 final : public Geographic3DToGravityRelatedHeightEGM2008
	{
	public:
		using Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008;
	};
}
