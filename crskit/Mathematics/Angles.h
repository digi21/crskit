#pragma once

#include <cmath>
#include <concepts>

#define EPSILON 1e-10

// Some <cmath> headers define these as macros; we redefine them as constexpr.
#undef M_E
#undef M_PI
#undef M_PI_2

namespace CrsKit::Math
{
constexpr auto M_E = 2.71828182845904523536;
constexpr auto M_PI = 3.14159265358979323846;
constexpr auto M_PI_2 = M_PI / 2.0;

template <std::convertible_to<double> T>
constexpr auto DEGREES_TO_RADIANS(T degrees) -> double { return static_cast<double>(degrees) * M_PI / 180.0; }

template <std::convertible_to<double> T>
constexpr auto RADIANS_TO_DEGREES(T x) -> double { return static_cast<double>(x) * 180.0 / M_PI; }

template <std::convertible_to<double> T>
constexpr auto GRADS_TO_RADIANS(T degrees) -> double { return static_cast<double>(degrees) * M_PI / 200.0; }

template <std::convertible_to<double> T>
constexpr auto RADIANS_TO_GRADS(T x) -> double { return static_cast<double>(x) * 200.0 / M_PI; }

template <std::convertible_to<double> T>
constexpr auto DEGREES_TO_GRADS(T x) -> double { return x * 200.0 / 180.0; }

template <std::convertible_to<double> T>
constexpr auto GRADS_TO_DEGREES(T x) -> double { return x * 180.0 / 200.0; }

template <std::convertible_to<double> T>
constexpr auto SIGN(T x) -> double { return (x > 0 ? 1.0 : -1.0); }
}
