#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace CrsKit::Wkt
{
	///	<summary>
	///		Represents a token of a Well Known Text
	///	</summary>
	///	<remarks>
	///		If the token is a datum, it will have no children and accessing its value (GetString/GetInt/GetDouble) returns an empty/zero value.
	///		If the token is not a datum, it will have a name and accessing GetName returns its name (value accessors return empty/zero).
	///	</remarks>
	class TokenWkt
	{
		std::string _value;
		std::vector<std::unique_ptr<TokenWkt>> _children;

	public:
		explicit TokenWkt(std::string const& name);

		// Takes ownership of the child and returns an observer to it (to build the tree).
		auto AddChild(std::unique_ptr<TokenWkt> child) -> TokenWkt*;

		auto GetName() const -> std::string
		{
			if (_children.empty())
				return "";
			return _value;
		}

		auto GetString() const -> std::string
		{
			if (!_children.empty())
				return "";
			return _value;
		}

		auto GetInt() const -> int
		{
			if (!_children.empty())
				return 0;
			return std::stoi(_value);
		}

		auto GetDouble() const -> double
		{
			if (!_children.empty())
				return 0.0;
			return std::stod(_value);
		}

		auto GetEsString() const -> bool;

		auto HasChild(std::string const& childName) const -> bool;

		auto GetChildCount() const -> size_t;

		auto GetChild(std::string const& childName) const noexcept(false) -> const TokenWkt&;
		auto GetChild(size_t index) const noexcept(false) -> const TokenWkt&;
	};

	// The (authority name, code) carried by a node's AUTHORITY child, or {"", 0} when there is none.
	[[nodiscard]] auto ReadAuthority(TokenWkt const& node) -> std::pair<std::string, int>;

	// The EPSG code carried by a node's AUTHORITY child, or 0 when there is none or the authority is not EPSG.
	[[nodiscard]] auto EpsgCode(TokenWkt const& node) -> int;
}
