#include "pch.h"

using namespace std;

namespace CrsKit::Wkt
{
	TokenWkt::TokenWkt(std::string const& name)
		: _value{name}
	{
		_value = trim(_value);
		_value = trim(_value, '"');
	}

	auto TokenWkt::AddChild(std::unique_ptr<TokenWkt> child) -> TokenWkt*
	{
		_children.push_back(std::move(child));
		return _children.back().get();
	}

	auto TokenWkt::GetEsString() const -> bool
	{
		if (!_children.empty())
			throw OpenGisException{ "A node with children has no scalar value" };

		std::stringstream ss;
		ss << _value;

		double d;
		ss >> d;

		if (ss.fail())
			return false;
		return true;
	}

	auto TokenWkt::HasChild(std::string const& childName) const -> bool
	{
		return std::ranges::any_of(_children, [&](auto const& child)
		{
			try { return 0 == compareNoCase(child->GetName().c_str(), childName.c_str()); }
			catch (...) { return false; }
		});
	}

	auto TokenWkt::GetChildCount() const -> size_t
	{
		return _children.size();
	}

	auto TokenWkt::GetChild(std::string const& childName) const noexcept(false) -> const TokenWkt&
	{
		auto const it = std::ranges::find_if(_children, [&](auto const& child)
		{
			try { return 0 == compareNoCase(child->GetName().c_str(), childName.c_str()); }
			catch (...) { return false; }
		});
		if (it != _children.end())
			return **it;

		throw WktParseException(std::format("The {} node in the Wkt string was not found.", static_cast<const char*>(childName.c_str())));
	}

	auto TokenWkt::GetChild(size_t index) const noexcept(false) -> const TokenWkt&
	{
		if (index >= _children.size())
			throw WktParseException("An attempt was made to access one Wkt node per index and the number of nodes was exceeded.");

		return *_children[index];
	}

	auto ReadAuthority(TokenWkt const& node) -> std::pair<std::string, int>
	{
		if (!node.HasChild("AUTHORITY"))
			return { "", 0 };

		auto const& authority = node.GetChild("AUTHORITY");
		return { authority.GetChild(0).GetString(), authority.GetChild(1).GetInt() };
	}

	auto EpsgCode(TokenWkt const& node) -> int
	{
		auto const [authority, code] = ReadAuthority(node);
		return 0 == compareNoCase(authority.c_str(), "EPSG") ? code : 0;
	}
}
