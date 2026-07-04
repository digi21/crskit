#pragma once

#include <compare>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "AngularUnit.h"

namespace CrsKit::CoordinateSystems
{
	// Value type (cf. AxisInfo): a prime meridian. Holds its angular unit by value.
	class _modeCrsKit PrimeMeridian final
	{
		std::string _name;
		std::string _authority;
		int _authorityCode{};
		std::string _alias;
		std::string _remarks;
		AngularUnit _angularUnit;
		double _longitude{};

	public:
		PrimeMeridian() = default;
		PrimeMeridian(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AngularUnit const& angularUnit, double longitude);

		auto operator<=>(PrimeMeridian const&) const = default;

		static auto FromWkt(Wkt::TokenWkt const& tokenUnit) -> PrimeMeridian;

		[[nodiscard]] auto GetName() const -> std::string { return _name; }
		[[nodiscard]] auto GetAuthority() const -> std::string { return _authority; }
		[[nodiscard]] auto GetAuthorityCode() const -> int { return _authorityCode; }
		[[nodiscard]] auto GetAlias() const -> std::string { return _alias; }
		[[nodiscard]] auto GetRemarks() const -> std::string { return _remarks; }

		[[nodiscard]] auto GetLongitude() const -> double { return _longitude; }
		[[nodiscard]] auto GetAngularUnit() const -> AngularUnit const& { return _angularUnit; }

		[[nodiscard]] auto GetWkt() const -> std::string;
	};
}
