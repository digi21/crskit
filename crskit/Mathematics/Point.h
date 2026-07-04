#pragma once

#include "Concepts.h"

namespace CrsKit::Math
{
	template <Arithmetic T>
	class Point2D
	{
	public:
		T x{};
		T y{};

		Point2D() = default;

		Point2D(T x, T y)
			: x{x}
			, y{y}
		{
		}

		auto operator<=>(Point2D const&) const = default;
	};

	template <Arithmetic T>
	class Point3D : public Point2D<T>
	{
	public:
		T z{};

		Point3D() = default;

		Point3D(T x, T y, T z = 0)
			: Point2D<T>{x, y}
			, z{z}
		{
		}

		auto operator<=>(Point3D const&) const = default;
	};
}
