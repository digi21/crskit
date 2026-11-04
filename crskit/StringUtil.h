#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <cstdlib>
#include <cctype>

// String utilities (UTF-8 std::string). The library is UTF-8 throughout the
// model (CRS names, WKT, parameters): does not use wchar_t/std::wstring.

namespace CrsKit
{

[[nodiscard]] inline int compareNoCase(const char* a, const char* b) noexcept
{
	for (;; ++a, ++b)
	{
		auto const ca = std::tolower(static_cast<unsigned char>(*a));
		auto const cb = std::tolower(static_cast<unsigned char>(*b));
		if (ca != cb)
			return ca < cb ? -1 : 1;
		if (*a == '\0')
			return 0;
	}
}

[[nodiscard]] inline int parse_int(const char* s) noexcept { return static_cast<int>(std::strtol(s, nullptr, 10)); }
[[nodiscard]] inline double parse_double(const char* s) noexcept { return std::strtod(s, nullptr); }

// Internal helpers (not part of the public API). Every file path crskit receives or returns as a
// std::string is UTF-8. std::filesystem::path(std::string) decodes with the ANSI code page on
// Windows, so paths are built from std::u8string and converted back with u8string().
// On Linux both conversions copy the bytes unchanged.
[[nodiscard]] inline std::filesystem::path CreatePathFromUtf8(std::string const& utf8Path)
{
	return std::filesystem::path{ std::u8string{ reinterpret_cast<char8_t const*>(utf8Path.data()), utf8Path.size() } };
}

[[nodiscard]] inline std::string ConvertPathToUtf8String(std::filesystem::path const& path)
{
	auto const utf8Path = path.u8string();
	return std::string{ reinterpret_cast<char const*>(utf8Path.data()), utf8Path.size() };
}

// string_format (snprintf) removed: use std::format directly (C++20/<format>).

// Adapted from https://www.educative.io/courses/cpp-17-in-detail-a-deep-dive/7nWg1RXRNxB
[[nodiscard]] inline std::vector<std::string>
split(std::string_view strv, std::string_view delims = " ")
{
	std::vector<std::string> output; auto first = strv.begin();
	while (first != strv.end()) {
		const auto second = std::find_first_of(first, std::cend(strv), std::cbegin(delims), std::cend(delims));

		if (first != second)
			output.emplace_back(strv.substr(std::distance(strv.begin(), first), std::distance(first, second)));

		if (second == strv.end())
			break;

		first = std::next(second);
	}
	return output;
}


[[nodiscard]] inline std::string trim(std::string_view str) {
	auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
		return std::isspace(ch);
		});
	auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
		return std::isspace(ch);
		}).base();
	return (start < end) ? std::string(start, end) : std::string();
}

[[nodiscard]] inline std::string trim(std::string_view str, char chTarget) {
	auto start = std::find_if_not(str.begin(), str.end(), [chTarget](char ch) {
		return ch == chTarget;
		});
	auto end = std::find_if_not(str.rbegin(), str.rend(), [chTarget](char ch) {
		return ch == chTarget;
		}).base();
	return (start < end) ? std::string(start, end) : std::string();
}

}
