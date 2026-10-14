#include "pch.h"
#include "CrsAliases.h"

#include <stdexcept>

namespace CrsKit
{
	namespace
	{
		constexpr auto kEpsg = "EPSG";

		auto IsAsciiSpace(unsigned char c) -> bool
		{
			return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
		}

		auto UpperAscii(std::string_view text) -> std::string
		{
			std::string out{ text };
			for (auto& c : out)
				if (c >= 'a' && c <= 'z')
					c = static_cast<char>(c - 'a' + 'A');
			return out;
		}

		auto ThrowIfInvalidTarget(int targetCode) -> void
		{
			if (targetCode <= 0)
				throw std::invalid_argument(std::format("An alias target must be a positive EPSG code, not {}.", targetCode));
		}
	}

	auto CrsAliases::NormalizeName(std::string_view name) -> std::string
	{
		std::string out;
		out.reserve(name.size());
		auto pendingSpace = false;
		for (auto const ch : name)
		{
			auto const c = static_cast<unsigned char>(ch);
			if (c == '_' || IsAsciiSpace(c))
			{
				pendingSpace = !out.empty();
				continue;
			}
			if (pendingSpace)
			{
				out += ' ';
				pendingSpace = false;
			}
			out += (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : ch;
		}
		return out;
	}

	auto CrsAliases::IsCrsTarget(int code) const -> bool
	{
		if (!_tables)
			return false;
		auto const isCode = [code](auto const& entry) { return entry.second == code; };
		return std::ranges::any_of(_tables->crsCodes, isCode) || std::ranges::any_of(_tables->crsNames, isCode);
	}

	auto CrsAliases::ThrowIfCrsTargetIsSource(int targetCode) const -> void
	{
		if (auto const existing = FindCrs(kEpsg, targetCode); existing && *existing != targetCode)
			throw std::invalid_argument(std::format("EPSG:{} is already an alias of EPSG:{}; aliases cannot be chained.", targetCode, *existing));
	}

	auto CrsAliases::AddCrs(int code, int targetCode) -> void
	{
		AddCrs(kEpsg, code, targetCode);
	}

	auto CrsAliases::AddCrs(std::string_view authority, int code, int targetCode) -> void
	{
		ThrowIfInvalidTarget(targetCode);
		auto key = std::pair{ UpperAscii(authority), code };
		if (key.first.empty())
			throw std::invalid_argument("A CRS alias needs an authority.");

		ThrowIfCrsTargetIsSource(targetCode);
		if (key.first == kEpsg && code != targetCode && IsCrsTarget(code))
			throw std::invalid_argument(std::format("EPSG:{} is the target of another alias; aliases cannot be chained.", code));

		auto tables = _tables ? std::make_shared<Tables>(*_tables) : std::make_shared<Tables>();
		tables->crsCodes.insert_or_assign(std::move(key), targetCode);
		_tables = std::move(tables);
	}

	auto CrsAliases::AddCrsName(std::string_view name, int targetCode) -> void
	{
		ThrowIfInvalidTarget(targetCode);
		auto normalized = NormalizeName(name);
		if (normalized.empty())
			throw std::invalid_argument("A CRS name alias needs a non-empty name.");

		ThrowIfCrsTargetIsSource(targetCode);

		auto tables = _tables ? std::make_shared<Tables>(*_tables) : std::make_shared<Tables>();
		tables->crsNames.insert_or_assign(std::move(normalized), targetCode);
		_tables = std::move(tables);
	}

	auto CrsAliases::AddOperation(int code, int targetCode) -> void
	{
		ThrowIfInvalidTarget(targetCode);
		if (auto const existing = FindOperation(targetCode); existing && *existing != targetCode)
			throw std::invalid_argument(std::format("Operation {} is already an alias of operation {}; aliases cannot be chained.", targetCode, *existing));
		if (code != targetCode && _tables && std::ranges::any_of(_tables->operations, [code](auto const& entry) { return entry.second == code; }))
			throw std::invalid_argument(std::format("Operation {} is the target of another alias; aliases cannot be chained.", code));

		auto tables = _tables ? std::make_shared<Tables>(*_tables) : std::make_shared<Tables>();
		tables->operations.insert_or_assign(code, targetCode);
		_tables = std::move(tables);
	}

	auto CrsAliases::Clear() -> void
	{
		_tables.reset();
	}

	auto CrsAliases::IsEmpty() const -> bool
	{
		return !_tables || (_tables->crsCodes.empty() && _tables->crsNames.empty() && _tables->operations.empty());
	}

	auto CrsAliases::FindCrs(std::string_view authority, int code) const -> std::optional<int>
	{
		if (!_tables || _tables->crsCodes.empty() || authority.empty())
			return std::nullopt;
		auto const it = _tables->crsCodes.find(std::pair{ UpperAscii(authority), code });
		if (it == _tables->crsCodes.end())
			return std::nullopt;
		return it->second;
	}

	auto CrsAliases::FindCrsByName(std::string_view name) const -> std::optional<int>
	{
		if (!_tables || _tables->crsNames.empty())
			return std::nullopt;
		auto const it = _tables->crsNames.find(NormalizeName(name));
		if (it == _tables->crsNames.end())
			return std::nullopt;
		return it->second;
	}

	auto CrsAliases::FindOperation(int code) const -> std::optional<int>
	{
		if (!_tables)
			return std::nullopt;
		auto const it = _tables->operations.find(code);
		if (it == _tables->operations.end())
			return std::nullopt;
		return it->second;
	}

	auto CrsAliases::ResolveCrs(int code) const -> int
	{
		return FindCrs(kEpsg, code).value_or(code);
	}

	auto CrsAliases::ResolveOperation(int code) const -> int
	{
		return FindOperation(code).value_or(code);
	}
}
