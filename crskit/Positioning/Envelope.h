#pragma once

#include <compare>
#include <vector>

namespace CrsKit::Positioning
{
	struct Envelope
	{
	private:
		std::vector<double> minCP;
		std::vector<double> maxCP;

	public:
		Envelope() = default;

		Envelope(std::vector<double> const& _minCP, std::vector<double> const& _maxCP)
			: minCP{_minCP}
			, maxCP{_maxCP}
		{
		}

		auto operator<=>(Envelope const&) const = default;

		auto GetMinCP() -> std::vector<double>
		{
			return minCP;
		}

		auto GetMaxCP() -> std::vector<double>
		{
			return maxCP;
		}
	};
}
