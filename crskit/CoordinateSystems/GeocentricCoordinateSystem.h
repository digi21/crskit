#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "CoordinateSystem.h"
#include "IGeocentricCoordinateSystem.h"
#include "IHorizontalDatum.h"
#include "LinearUnit.h"
#include "PrimeMeridian.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit GeocentricCoordinateSystem final
	: public CoordinateSystem, virtual public IGeocentricCoordinateSystem
	{
		std::shared_ptr<IHorizontalDatum> _datum;
		LinearUnit _linearUnit;
		PrimeMeridian _primeMeridian;

	public:
		GeocentricCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, LinearUnit const& linearUnit, std::shared_ptr<IHorizontalDatum> const& horizontalDatum, PrimeMeridian const& primeMeridian, AxisInfo const& axis1, AxisInfo const& axis2, AxisInfo const& axis3);
		static auto FromWkt(Wkt::TokenWkt const& token) -> std::shared_ptr<GeocentricCoordinateSystem>;
#pragma region GeocentricCoordinateSystem members

	public:
		auto GetHorizontalDatum() const -> std::shared_ptr<IHorizontalDatum> override;

		auto GetLinearUnit() const -> LinearUnit override;

		auto GetPrimeMeridian() const -> PrimeMeridian override;
#pragma endregion
#pragma region IBaseInfo members
		using CoordinateSystem::GetWkt;
		[[nodiscard]] auto GetWkt(CrsContext const& context) const -> std::string override;
#pragma endregion
	};
}
