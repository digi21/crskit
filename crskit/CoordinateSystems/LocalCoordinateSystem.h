#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "CoordinateSystem.h"
#include "ILocalCoordinateSystem.h"
#include "ILocalDatum.h"
#include "Unit.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit LocalCoordinateSystem final : public CoordinateSystem, virtual public ILocalCoordinateSystem
	{
		AnyUnit _unit;
		std::shared_ptr<ILocalDatum> _datum;

	public:
		LocalCoordinateSystem(std::string const& name, std::shared_ptr<ILocalDatum> const& datum, AnyUnit const& linearUnit, std::vector<AxisInfo> const& axis);
		static auto FromWkt(Wkt::TokenWkt const& token) -> std::shared_ptr<LocalCoordinateSystem>;

#pragma region LocalCoordinateSystem members

	public:
		auto GetLocalDatum() const -> std::shared_ptr<ILocalDatum> override;
#pragma endregion
#pragma region IBaseInfo members
		using CoordinateSystem::GetWkt;
		[[nodiscard]] auto GetWkt(CrsContext const& context) const -> std::string override;
#pragma endregion
	};
}
