#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "IUtilities.h"

namespace CrsKit
{
	class Utilities final : public IUtilities
	{
		auto GetCompoundLocalWkt() const -> std::string override;
		auto GetCompoundLocalWkt(std::string const& title) const -> std::string override;
		auto GetCompoundHorizontalPlusVerticalLocalWkt(const char* title, const char* horizontalWkt) const -> std::string override;
		auto GetUnknownVerticalWkt() const -> std::string override;
		auto GetWktFrom3DSystem(std::string const& wkt)  const->std::string override;;
		auto GetUnknownHorizontalWkt() const -> std::string override;
		auto CreateCompoundCrsWktFromHorizontalCrs(std::string const& horizontalWkt) const -> std::string override;
		auto CreateCompoundCrsWktFromHorizontalAndVerticalCrs(std::string const& horizontalWkt, std::string const& verticalWkt) const -> std::string override;
		auto GetCompoundCrsWktFromHorizontalCrs(std::string const& horizontalWkt) const -> std::string override;
		auto GetCoordinateReferenceSystemName(int code) const -> std::string override;
		auto GetCoordinateReferenceSystemName(std::string const& wkt) const -> std::string override;
		auto GetWktFromProjectionCodeForGeographic(int projectionCode, int geographicCode) const -> std::string override;
		auto GetWktFromGeographic3DSystemForProjected(std::string const& wkt) const -> std::string override;
		auto GetWktFromGeographicEpsgCode(int epsgCode, bool longitudeLatitude) const -> std::string override;
		auto GetWktFromEpsgCode(int code) const -> std::string override;
		auto GetWktFromEpsgCodeWithEastNorthAxes(int epsgCode) const -> std::string override;
		auto GetWktFromCrsCodeWithEastNorthAxes(std::string const& crsCode) const -> std::string override;
		auto GetWktWithEastNorthAxes(std::string const& wkt) const -> std::string override;
		auto GetAxisNames(std::string const& wkt) const -> std::vector<std::string> override;
		auto GetUnitNames(std::string const& wkt) const -> std::vector<std::string> override;
		auto GetBothHorizontalSystemsAreIdentical(std::string const& wktA, std::string const& wktB) const -> bool override;
		auto GetEpsgCodeFromWkt(std::string const& wkt) const -> int override;
		auto LoadPrjFileForFileOrThrow(std::string const& filePath) const -> std::string override;
		auto LoadPrjFileForFileOrLocalSystem(std::string const& filePath) const -> std::string override;
		auto CreatePrjFileForFileOrThrow(std::string const& filePath, std::string const& wkt) const -> void override;

		auto ExtractHorizontalVertical(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& scr) const -> std::pair<std::shared_ptr<CoordinateSystems::CoordinateSystem>, std::shared_ptr<CoordinateSystems::CoordinateSystem>> override;
		auto ExtractHorizontalVertical(std::string const& wkt) const -> std::pair<std::string, std::string> override;
		auto ExtractUtmParametersFromWkt(std::string const& coordinateSystemWkt, double& semiMinorAxis, double& semiMajorAxis, double& latitudeOfNaturalOrigin, double& longitudeOfNaturalOrigin, double& scaleFactorAtNaturalOrigin, double& falseEasting, double& falseNorthing) const -> bool override;

		auto CreateTransformToGeocentric(
			std::shared_ptr<CoordinateSystems::CoordinateSystem> const& horizontal,
			std::shared_ptr<CoordinateSystems::CoordinateSystem> const& vertical,
			std::vector<std::shared_ptr<CoordinateTransformations::IMathTransform>>& direct,
			std::vector<std::shared_ptr<CoordinateTransformations::IMathTransform>>& inverse,
			DelegateCreateVerticalTransformation const& createVerticalSystemDialogCallback,
			DelegateDetectedMultipleCoordinateOperations const& selectTransformationCallback) const -> void override;

		auto CreateAffineMainDiagonalTransform(int sideSize) const -> std::shared_ptr<CoordinateTransformations::IMathTransform> override;
		auto CreateConcatenatedTransform(std::vector<std::shared_ptr<CoordinateTransformations::IMathTransform>> const& transforms) const -> std::shared_ptr<CoordinateTransformations::IMathTransform> override;
		auto CreateGeocentricToTopocentric(double semiMajorAxis, double semiMinorAxis, double geocentricXOfTopocentricOrigin, double geocentricYOfTopocentricOrigin, double geocentricZOfTopocentricOrigin, bool inverse) const -> std::shared_ptr<CoordinateTransformations::IMathTransform> override;
		auto CreateGeographicToGeocentric(double semiMajorAxis, double semiMinorAxis, bool inverse) const -> std::shared_ptr<CoordinateTransformations::IMathTransform> override;
		auto CreateObliqueStereographic(double semiMajorAxis, double semiMinorAxis, double latitudeOfOrigin, double centralMeridian, double scaleFactor, double falseEasting, double falseNorthing, bool inverse) const->std::shared_ptr<CoordinateTransformations::IMathTransform> override;

		auto ToSexagesimal(int angularUnitCode, double factorToStandardUnit, double value) -> double override;
	};
}
