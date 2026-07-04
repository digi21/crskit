#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../CoordinateSystems/GeocentricCoordinateSystem.h"
#include "../IAuthorityProvider.h"
#include "ICoordinateTransformationAuthorityFactory.h"
#include "IMathTransform.h"

namespace CrsKit::Epsg
{
	class _modeCrsKit CoordinateTransformationAuthorityFactory final
	: public CoordinateTransformations::ICoordinateTransformationAuthorityFactory
	{
		std::shared_ptr<Epsg::IAuthorityProvider> _provider;
#pragma region ICoordinateTransformationAuthorityFactory members

	public:
		CoordinateTransformationAuthorityFactory(std::shared_ptr<Epsg::IAuthorityProvider> const& authorityProvider);

		[[nodiscard]] auto GetAuthority() const -> std::string override;

		[[nodiscard]] auto CreateFromTransformationCode(int code) -> std::shared_ptr<CoordinateTransformations::ICoordinateTransformation> override;
		[[nodiscard]] auto CreateFromCoordinateSystemsCodes(int sourceCode, int targetCode) -> std::shared_ptr<CoordinateTransformations::ICoordinateTransformation> override;
		[[nodiscard]] auto CreateFromCoordinateSystemsCodes(int sourceCode, int targetCode, CoordinateTransformationOptions const& options) -> std::shared_ptr<CoordinateTransformations::ICoordinateTransformation> override;
#pragma endregion

#pragma region Implementation
		auto StaticCreateFromTransformationCode(int code) const -> std::shared_ptr<CoordinateTransformations::ICoordinateTransformation>;
		auto StaticCreateFromCoordinateSystemsCodes(int sourceCode, int targetCode) const -> std::shared_ptr<CoordinateTransformations::ICoordinateTransformation>;
		auto StaticCreateFromCoordinateSystemsCodes(int sourceCode, int targetCode, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformations::ICoordinateTransformation>;
		[[nodiscard]] auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target) const -> std::shared_ptr<CoordinateTransformations::ICoordinateTransformation> override;
		[[nodiscard]] auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation> override;
		[[nodiscard]] auto CreateFromTransformationCode(int code, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target, bool inverse) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation>;
		[[nodiscard]] auto CreateMathTransform(int code) const -> std::shared_ptr<CoordinateTransformations::IMathTransform>;
		[[nodiscard]] auto CreateAxisUnitChangeAndMathOperationTransformBetweenCoordinateSystems(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& target, int operationCode, bool inverse) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation>;
		[[nodiscard]] auto CreateCoordinateTransformBetweenHorizontalSystems(std::shared_ptr<CoordinateSystems::HorizontalCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::HorizontalCoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation> override;

		[[nodiscard]] auto CreateCoordinateTransformBetweenVerticalSystems(std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation>;
		[[nodiscard]] auto CreateCoordinateTransformBetweenGeographicSystems(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> source, std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation>;
		[[nodiscard]] auto CreateCoordinateTransformBetweenGeocentricSystems(std::shared_ptr<CoordinateSystems::GeocentricCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::GeocentricCoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation>;

		[[nodiscard]] auto CreateCoordinateTransformBetweenGeographicVerticalSystems(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& target, bool inverse, CoordinateTransformationOptions const& options) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation> override;
		[[nodiscard]] auto CreateCoordinateTransformBetweenGeographicVerticalSystems(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& target, int operationCode, bool inverse) const -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation> override;

		///	<summary>
		///		Creates a transformation between two coordinate reference systems with the same authority code, taking into account that they may have swapped axes.
		///	</summary>
		[[nodiscard]] static auto CreateCoordinateTransformWithinSameCrs(std::shared_ptr<CoordinateSystems::HorizontalCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::HorizontalCoordinateSystem> const& target) -> std::shared_ptr<CoordinateTransformations::CoordinateTransformation>;
#pragma endregion
	};
}
