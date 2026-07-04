#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "../CoordinateSystems/GeocentricCoordinateSystem.h"
#include "../CoordinateSystems/GeographicCoordinateSystem.h"
#include "../CoordinateSystems/ProjectedCoordinateSystem.h"
#include "../CoordinateSystems/VerticalCoordinateSystem.h"
#include "../CoordinateTransformationOptions.h"
#include "CoordinateTransformation.h"
#include "ICoordinateTransformation.h"
#include "ICoordinateTransformationFactory.h"
#include "IMathTransform.h"

namespace CrsKit::CoordinateTransformations
{
	class _modeCrsKit CoordinateTransformationFactory final
		: public ICoordinateTransformationFactory
	{
	public:
		[[nodiscard]] auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS) const->std::shared_ptr<ICoordinateTransformation> override;
		[[nodiscard]] auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation> override;
		[[nodiscard]] auto CreateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation> override;
		[[nodiscard]] auto CreateFrom3DCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation> override;
		[[nodiscard]] auto CreateFromHorizontalCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& geographic3D, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation> override;
	};

	class _modeCrsKit StaticCoordinateTransformationFactory final
	{
	public:
		[[nodiscard]] static auto CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::string const& transformationName, std::string const& description, std::string const& areaName, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& target, std::shared_ptr<IMathTransform> const& mathTransform, bool inverse, int authorityCode = 0) -> std::shared_ptr<CoordinateTransformation>;
		[[nodiscard]] static auto CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::string const& transformationName, std::string const& description, std::string const& areaName, std::shared_ptr<CoordinateSystems::GeocentricCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::GeocentricCoordinateSystem> const& target, std::shared_ptr<IMathTransform> const& mathTransform, bool inverse, int authorityCode = 0) -> std::shared_ptr<CoordinateTransformation>;
		[[nodiscard]] static auto CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::string const& transformationName, std::string const& description, std::string const& areaName, std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& target, std::shared_ptr<IMathTransform> const& mathTransform, bool inverse, int authorityCode = 0) -> std::shared_ptr<CoordinateTransformation>;
		[[nodiscard]] static auto CreateProjectionTransforms(std::shared_ptr<CoordinateSystems::ProjectedCoordinateSystem> const& pcs) -> std::shared_ptr<IMathTransform>;

		[[nodiscard]] static auto CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& system) -> std::vector<std::shared_ptr<IMathTransform>>;
		[[nodiscard]] static auto CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& system) -> std::vector<std::shared_ptr<IMathTransform>>;

		[[nodiscard]] static auto CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(std::shared_ptr<CoordinateSystems::GeocentricCoordinateSystem> const& system) -> std::vector<std::shared_ptr<IMathTransform>>;
		[[nodiscard]] static auto CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(std::shared_ptr<CoordinateSystems::GeocentricCoordinateSystem> const& system) -> std::vector<std::shared_ptr<IMathTransform>>;

		[[nodiscard]] static auto CreateTransformsFromSystemToSystemCompatibleWithTransformMethods(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& system, int pointDimensions) -> std::vector<std::shared_ptr<IMathTransform>>;
		[[nodiscard]] static auto CreateTransformsFromSystemCompatibleWithTransformMethodsToSystem(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& system) -> std::vector<std::shared_ptr<IMathTransform>>;

		static auto LocateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::vector<std::shared_ptr<IMathTransform>>;
		static auto LocateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options, bool inverse) -> std::vector<std::shared_ptr<IMathTransform>>;

		[[nodiscard]] static auto FindTransformsGeographic3DToCompound(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& horizontalTarget, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& verticalTarget, CoordinateTransformationOptions const& options) -> std::vector<std::shared_ptr<IMathTransform>>;
		[[nodiscard]] static auto FindTransformsCompoundToGeographic3D(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& horizontalSource, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& verticalSource, std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& target, CoordinateTransformationOptions const& options) -> std::vector<std::shared_ptr<IMathTransform>>;
		[[nodiscard]] static auto FindTransformsCompoundToCompound(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& horizontalSource, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& verticalSource, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& horizontalTarget, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& verticalTarget, CoordinateTransformationOptions const& options) -> std::vector<std::shared_ptr<IMathTransform>>;
		[[nodiscard]] static auto FindTransformsToTransformBetween3DCrs(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::vector<std::shared_ptr<IMathTransform>>;

	public:
		// Identity (or axis swap) transform to place data "as-is" in the working frame.
		[[nodiscard]] static auto CreateAssumeWorkingFrameTransform(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& a, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& b) -> std::shared_ptr<IMathTransform>;
		// Resolves the "exactly one side is local/unknown" case per the options
		// (resolveTransform delegate wins; otherwise unknownCrsPolicy). Returns the transform or throws (reject).
		[[nodiscard]] static auto ResolveUnknownCrs(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target, CoordinateTransformationOptions const& options) -> std::shared_ptr<IMathTransform>;

		[[nodiscard]] static auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS) -> std::shared_ptr<ICoordinateTransformation>;
		[[nodiscard]] static auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>;
		[[nodiscard]] static auto CreateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>;
		[[nodiscard]] static auto CreateFrom3DCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>;
		[[nodiscard]] static auto CreateFromHorizontalCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& geographic3D, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>;

#pragma region Vertical transformations
		// If we run the following query to the database:
		//
		//SELECT DISTINCT X.COORD_REF_SYS_KIND, Y.COORD_REF_SYS_KIND AS Expr1
		//	FROM            Coordinate_Operation AS A INNER JOIN
		//	[Coordinate Reference System] AS X ON A.SOURCE_CRS_CODE = X.COORD_REF_SYS_CODE INNER JOIN
		//	[Coordinate Reference System] AS Y ON A.TARGET_CRS_CODE = Y.COORD_REF_SYS_CODE
		//	WHERE        (A.SOURCE_CRS_CODE IN
		//	(SELECT        COORD_REF_SYS_CODE
		//	FROM            [Coordinate Reference System]
		//WHERE        (COORD_REF_SYS_KIND = 'vertical'))) OR
		//	(A.TARGET_CRS_CODE IN
		//	(SELECT        COORD_REF_SYS_CODE
		//	FROM            [Coordinate Reference System] AS [Coordinate Reference System_1]
		//WHERE        (COORD_REF_SYS_KIND = 'vertical')))
		// 
		// We check that there are transformations between the following systems:
		//
		// Geographic 2D -> Vertical
		// Geographic 3D -> Vertical
		// vertical -> vertical
		//
		//
		// But if we show the transformation names:
		//
		//SELECT        X.COORD_REF_SYS_NAME, Y.COORD_REF_SYS_NAME AS Expr2, X.COORD_REF_SYS_KIND, Y.COORD_REF_SYS_KIND AS Expr1
		//	FROM            Coordinate_Operation AS A INNER JOIN
		//	[Coordinate Reference System] AS X ON A.SOURCE_CRS_CODE = X.COORD_REF_SYS_CODE INNER JOIN
		//	[Coordinate Reference System] AS Y ON A.TARGET_CRS_CODE = Y.COORD_REF_SYS_CODE
		//	WHERE        (A.SOURCE_CRS_CODE IN
		//	(SELECT        COORD_REF_SYS_CODE
		//	FROM            [Coordinate Reference System]
		//WHERE        (COORD_REF_SYS_KIND = 'vertical'))) OR
		//	(A.TARGET_CRS_CODE IN
		//	(SELECT        COORD_REF_SYS_CODE
		//	FROM            [Coordinate Reference System] AS [Coordinate Reference System_1]
		//WHERE        (COORD_REF_SYS_KIND = 'vertical')))
		//
		// We check that the only geographic-2D-to-vertical transformations are NAD83 to NAVD88, which I have not implemented and probably will not, so that
		// there are only Vertical-to-Vertical and geographic-3D-to-vertical transformations.
		//
		// We are at this point because the CRSs had no EPSG code, which means one (or both) are compound systems.
		// Here we analyze the systems by decomposing them 
		[[nodiscard]] static auto CreateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& target, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>;
		[[nodiscard]] static auto CreateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& target, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation>;
#pragma endregion
	};
}
