#pragma once

namespace CrsKit::CoordinateTransformations
{
	enum class DomainFlags
	{
		Inside = 1,
		Outside = 2,
		Discontinuous = 4
	};
}
