#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "GeographicCoordinateSystem.h"
#include "HorizontalCoordinateSystem.h"
#include "IProjectedCoordinateSystem.h"
#include "IProjection.h"
#include "LinearUnit.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit ProjectedCoordinateSystem final : public HorizontalCoordinateSystem, virtual public IProjectedCoordinateSystem
	{
	private:
		std::shared_ptr<GeographicCoordinateSystem> _geographicCoordinateSystem;
		LinearUnit _linearUnit;
		std::shared_ptr<IProjection> _projection;

	public:
		ProjectedCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::shared_ptr<GeographicCoordinateSystem> const& gcs, std::shared_ptr<IProjection> const& projection, LinearUnit const& linearUnit, AxisInfo const& axis0, AxisInfo const& axis1);
		static auto FromWkt(Wkt::TokenWkt const& token) -> std::shared_ptr<ProjectedCoordinateSystem>;

#pragma region GeographicCoordinateSystem members

	public:
		auto GetGeographicCoordinateSystem() const -> std::shared_ptr<IGeographicCoordinateSystem> override;

		auto GetLinearUnit() const -> LinearUnit override;

		auto GetProjection() const -> std::shared_ptr<IProjection> override;
#pragma endregion
#pragma region IBaseInfo members
		using CoordinateSystem::GetWkt;
		[[nodiscard]] auto GetWkt(CrsContext const& context) const -> std::string override;
#pragma endregion
	};
}
