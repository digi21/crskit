#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

// Pulls in the CoordinateSystem / IMathTransform types and the Delegate* aliases used below.
#include "CoordinateTransformationOptions.h"

namespace CrsKit
{
	struct IUtilities
	{
		[[nodiscard]] virtual auto GetCompoundLocalWkt() const -> std::string = 0;
		[[nodiscard]] virtual auto GetCompoundLocalWkt(std::string const& title) const -> std::string = 0;
		[[nodiscard]] virtual auto GetCompoundHorizontalPlusVerticalLocalWkt(const char* title, const char* horizontalWkt) const -> std::string = 0;
		[[nodiscard]] virtual auto GetUnknownVerticalWkt() const -> std::string = 0;
		[[nodiscard]] virtual auto GetWktFrom3DSystem(std::string const& wkt) const -> std::string = 0;
		[[nodiscard]] virtual auto GetUnknownHorizontalWkt() const -> std::string = 0;
		[[nodiscard]] virtual auto CreateCompoundCrsWktFromHorizontalCrs(std::string const& horizontalWkt) const -> std::string = 0;
		[[nodiscard]] virtual auto CreateCompoundCrsWktFromHorizontalAndVerticalCrs(std::string const& horizontalWkt, std::string const& verticalWkt) const -> std::string = 0;
		[[nodiscard]] virtual auto GetCompoundCrsWktFromHorizontalCrs(std::string const& horizontalWkt) const -> std::string = 0;
		[[nodiscard]] virtual auto GetCoordinateReferenceSystemName(int code) const -> std::string = 0;
		[[nodiscard]] virtual auto GetCoordinateReferenceSystemName(std::string const& wkt) const -> std::string = 0;
		[[nodiscard]] virtual auto GetWktFromProjectionCodeForGeographic(int projectionCode, int geographicCode) const -> std::string = 0;
		[[nodiscard]] virtual auto GetWktFromGeographic3DSystemForProjected(std::string const& wkt) const -> std::string = 0;
		[[nodiscard]] virtual auto GetWktFromGeographicEpsgCode(int epsgCode, bool longitudeLatitude) const -> std::string = 0;
		[[nodiscard]] virtual auto GetWktFromEpsgCode(int code) const -> std::string = 0;
		[[nodiscard]] virtual auto GetWktFromEpsgCodeWithEastNorthAxes(int epsgCode) const -> std::string = 0;
		[[nodiscard]] virtual auto GetWktFromCrsCodeWithEastNorthAxes(std::string const& crsCode) const -> std::string = 0;
		[[nodiscard]] virtual auto GetWktWithEastNorthAxes(std::string const& wkt) const -> std::string = 0;
		[[nodiscard]] virtual auto GetAxisNames(std::string const& wkt) const -> std::vector<std::string> = 0;
		[[nodiscard]] virtual auto GetUnitNames(std::string const& wkt) const -> std::vector<std::string> = 0;
		[[nodiscard]] virtual auto GetBothHorizontalSystemsAreIdentical(std::string const& wktA, std::string const& wktB) const -> bool = 0;
		[[nodiscard]] virtual auto GetEpsgCodeFromWkt(std::string const& wkt) const -> int = 0;
		virtual auto ExtractHorizontalVertical(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& scr) const -> std::pair<std::shared_ptr<CoordinateSystems::CoordinateSystem>, std::shared_ptr<CoordinateSystems::CoordinateSystem>> = 0;
		virtual auto ExtractHorizontalVertical(std::string const& wkt) const -> std::pair<std::string, std::string> = 0;
		virtual auto ExtractUtmParametersFromWkt(std::string const& coordinateSystemWkt, double& semiMinorAxis, double& semiMajorAxis, double& latitudeOfNaturalOrigin, double& longitudeOfNaturalOrigin, double& scaleFactorAtNaturalOrigin, double& falseEasting, double& falseNorthing) const -> bool = 0;
		virtual auto LoadPrjFileForFileOrThrow(std::string const& filePath) const -> std::string = 0;
		virtual auto LoadPrjFileForFileOrLocalSystem(std::string const& filePath) const -> std::string = 0;
		virtual auto CreatePrjFileForFileOrThrow(std::string const& filePath, std::string const& wkt) const -> void = 0;

		virtual auto CreateTransformToGeocentric(
			std::shared_ptr<CoordinateSystems::CoordinateSystem> const& horizontal,
			std::shared_ptr<CoordinateSystems::CoordinateSystem> const& vertical,
			std::vector<std::shared_ptr<CoordinateTransformations::IMathTransform>>& direct,
			std::vector<std::shared_ptr<CoordinateTransformations::IMathTransform>>& inverse,
			DelegateCreateVerticalTransformation const& createVerticalSystemDialogCallback,
			DelegateDetectedMultipleCoordinateOperations const& selectTransformationCallback) const -> void = 0;

		[[nodiscard]] virtual auto CreateAffineMainDiagonalTransform(int sideSize) const -> std::shared_ptr<CoordinateTransformations::IMathTransform> = 0;
		[[nodiscard]] virtual auto CreateConcatenatedTransform(std::vector<std::shared_ptr<CoordinateTransformations::IMathTransform>> const& transforms) const -> std::shared_ptr<CoordinateTransformations::IMathTransform> = 0;
		[[nodiscard]] virtual auto CreateGeocentricToTopocentric(double semiMajorAxis, double semiMinorAxis, double geocentricXOfTopocentricOrigin, double geocentricYOfTopocentricOrigin, double geocentricZOfTopocentricOrigin, bool inverse) const -> std::shared_ptr<CoordinateTransformations::IMathTransform> = 0;
		[[nodiscard]] virtual auto CreateGeographicToGeocentric(double semiMajorAxis, double semiMinorAxis, bool inverse) const -> std::shared_ptr<CoordinateTransformations::IMathTransform> = 0;
		[[nodiscard]] virtual auto CreateObliqueStereographic(double semiMajorAxis, double semiMinorAxis, double latitudeOfOrigin, double centralMeridian, double scaleFactor, double falseEasting, double falseNorthing, bool inverse) const->std::shared_ptr<CoordinateTransformations::IMathTransform> = 0;
		[[nodiscard]] virtual auto ToSexagesimal(int angularUnitCode, double factorToStandardUnit, double value) -> double = 0;
	};
}
