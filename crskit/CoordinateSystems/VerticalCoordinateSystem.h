#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "CoordinateSystem.h"
#include "IVerticalCoordinateSystem.h"
#include "IVerticalDatum.h"
#include "LinearUnit.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit VerticalCoordinateSystem final : public CoordinateSystem, virtual public IVerticalCoordinateSystem
	{
		LinearUnit _verticalUnit;
		std::shared_ptr<IVerticalDatum> _verticalDatum;

	public:
		VerticalCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, LinearUnit const& linearUnit, std::shared_ptr<IVerticalDatum> const& verticalDatum, AxisInfo const& axis1);
		static auto FromWkt(Wkt::TokenWkt const& token) -> std::shared_ptr<VerticalCoordinateSystem>;

#pragma region VerticalCoordinateSystem members

	public:
		auto GetVerticalDatum() const -> std::shared_ptr<IVerticalDatum> override;

		auto GetVerticalUnit() const -> LinearUnit override;
#pragma endregion
#pragma region IBaseInfo members
		using CoordinateSystem::GetWkt;
		[[nodiscard]] auto GetWkt(CrsContext const& context) const -> std::string override;
#pragma endregion
	};
}
