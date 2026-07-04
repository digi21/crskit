#pragma once

#include <compare>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"

namespace CrsKit::CoordinateSystems
{
	// Value type (cf. AxisInfo): a linear unit of measure. Carries its identifying info inline; it is
	// no longer a polymorphic IInfo/ILinearUnit. Copied by value and compared with == / <=>.
	class _modeCrsKit LinearUnit final
	{
		std::string _name;
		std::string _authority;
		int _authorityCode{};
		std::string _alias;
		std::string _remarks;
		double _factorToMeters{};

	public:
		LinearUnit() = default;
		LinearUnit(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, double metersPerUnit);
		LinearUnit(std::string const& name, double metersPerUnit);

		auto operator<=>(LinearUnit const&) const = default;

		static auto FromWkt(Wkt::TokenWkt const& tokenUnit) -> LinearUnit;

		[[nodiscard]] auto GetName() const -> std::string { return _name; }
		[[nodiscard]] auto GetAuthority() const -> std::string { return _authority; }
		[[nodiscard]] auto GetAuthorityCode() const -> int { return _authorityCode; }
		[[nodiscard]] auto GetAlias() const -> std::string { return _alias; }
		[[nodiscard]] auto GetRemarks() const -> std::string { return _remarks; }

		[[nodiscard]] auto GetMetersPerUnit() const -> double { return _factorToMeters; }

		[[nodiscard]] auto GetWkt() const -> std::string;
	};
}
