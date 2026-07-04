#pragma once

#include <string>

namespace CrsKit::CoordinateSystems
{
	struct IInfo
	{
		virtual ~IInfo() = default;

		[[nodiscard]] virtual auto GetName() const -> std::string = 0;

		[[nodiscard]] virtual auto GetAuthority() const -> std::string = 0;

		[[nodiscard]] virtual auto GetAuthorityCode() const -> int = 0;

		[[nodiscard]] virtual auto GetAlias() const -> std::string = 0;

		[[nodiscard]] virtual auto GetRemarks() const -> std::string = 0;

		[[nodiscard]] virtual auto GetWkt() const -> std::string = 0;
	};

	// Sentinel returned by GetAuthorityCode() when an object carries no authority code
	// (e.g. a CRS/datum parsed from WKT without an AUTHORITY node, or a synthetic one).
	constexpr int UnknownAuthorityCode{0};
}
