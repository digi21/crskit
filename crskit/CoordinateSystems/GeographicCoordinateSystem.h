#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "AngularUnit.h"
#include "HorizontalCoordinateSystem.h"
#include "IGeographicCoordinateSystem.h"
#include "PrimeMeridian.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit GeographicCoordinateSystem final : public HorizontalCoordinateSystem, virtual public IGeographicCoordinateSystem
	{
		AngularUnit _angularUnit;
		PrimeMeridian _primeMeridian;

	public:
		GeographicCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AngularUnit const& angularUnit, std::shared_ptr<IHorizontalDatum> const& horizontalDatum, PrimeMeridian const& primeMeridian, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units);
		static auto FromWkt(Wkt::TokenWkt const& token) -> std::shared_ptr<GeographicCoordinateSystem>;
#pragma region GeographicCoordinateSystem members

	public:
		auto GetAngularUnit() const -> AngularUnit override;

		auto GetPrimeMeridian() const -> PrimeMeridian override;

		auto CompareTo(std::shared_ptr<IGeographicCoordinateSystem> const& obj) const -> int override;
#pragma endregion
#pragma region IBaseInfo members
		using CoordinateSystem::GetWkt;
		[[nodiscard]] auto GetWkt(CrsContext const& context) const -> std::string override;
#pragma endregion
	};
}
