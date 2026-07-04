#pragma once

#include <string>
#include <variant>

#include "AngularUnit.h"
#include "LinearUnit.h"

namespace CrsKit::CoordinateSystems
{
	// A coordinate-system axis unit is either linear or angular. std::variant gives the mixed per-axis
	// unit list value semantics, replacing the former shared_ptr<IUnit> polymorphism.
	using AnyUnit = std::variant<LinearUnit, AngularUnit>;

	// WKT of whichever alternative is held.
	[[nodiscard]] inline auto WktOf(AnyUnit const& unit) -> std::string
	{
		return std::visit([](auto const& u) { return u.GetWkt(); }, unit);
	}
}
