#pragma once

#include <limits>
#include "Point.h"
#include "Concepts.h"

namespace CrsKit::Math
{
	template <Arithmetic T>
	class Window2D
	{
	public:
		T Xmin{ std::numeric_limits<T>::max() };
		T Ymin{ std::numeric_limits<T>::max() };
		T Xmax{ -std::numeric_limits<T>::max() };
		T Ymax{ -std::numeric_limits<T>::max() };

		Window2D() = default;

		auto operator<=>(Window2D const&) const = default;

		auto operator&&(Point2D<T> const& point) const -> bool
		{
			return Xmin <= point.x && Ymin <= point.y && Xmax >= point.x && Ymax >= point.y;
		}

		auto operator=(Point2D<T> const& point) -> const Window2D<T>&
		{
			Xmin = Xmax = point.x;
			Ymin = Ymax = point.y;
			return *this;
		}

		auto Inflate(T dx, T dy) -> void
		{
			Xmin -= dx;
			Xmax += dx;
			Ymin -= dy;
			Ymax += dy;
		}

		auto Offset(T dx, T dy) -> void
		{
			Xmin += dx;
			Xmax += dx;
			Ymin += dy;
			Ymax += dy;
		}
	};
}
