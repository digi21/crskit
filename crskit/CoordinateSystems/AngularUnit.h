#pragma once

#include <compare>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"

namespace CrsKit::CoordinateSystems
{
	// Value type (cf. AxisInfo): an angular unit of measure. Identifying info carried inline.
	class _modeCrsKit AngularUnit final
	{
		std::string _name;
		std::string _authority;
		int _authorityCode{};
		std::string _alias;
		std::string _remarks;
		double _factorToRadians{};

	public:
		AngularUnit() = default;
		AngularUnit(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, double radiansPerUnit);
		AngularUnit(std::string const& name, double radiansPerUnit);

		auto operator<=>(AngularUnit const&) const = default;

		static auto FromWkt(Wkt::TokenWkt const& tokenUnit) -> AngularUnit;

		[[nodiscard]] auto GetName() const -> std::string { return _name; }
		[[nodiscard]] auto GetAuthority() const -> std::string { return _authority; }
		[[nodiscard]] auto GetAuthorityCode() const -> int { return _authorityCode; }
		[[nodiscard]] auto GetAlias() const -> std::string { return _alias; }
		[[nodiscard]] auto GetRemarks() const -> std::string { return _remarks; }

		[[nodiscard]] auto GetRadiansPerUnit() const noexcept -> double { return _factorToRadians; }

		[[nodiscard]] auto GetWkt() const -> std::string;
	};
}
