#pragma once

#include <concepts>

namespace CrsKit::Math
{
	// Numeric types (integral or floating-point) accepted by the math templates.
	template <typename T>
	concept Arithmetic = std::integral<T> || std::floating_point<T>;
}
