#pragma once

#include <memory>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#include "../CrsKitExport.h"
#include "../Parameter.h"
#include "../TokenWkt.h"
#include "IProjection.h"

namespace CrsKit::CoordinateSystems
{
	// WKT1 parameter names compared without case and without underscores: GDAL and OGC 01-009 write standard_parallel_1,
	// the crskit registry standard_parallel1.
	[[nodiscard]] inline auto ParameterNamesMatch(std::string_view a, std::string_view b) -> bool
	{
		auto const next = [](std::string_view s, size_t& i) { while (i < s.size() && '_' == s[i]) ++i; return i < s.size(); };
		size_t i = 0, j = 0;
		while (true)
		{
			auto const hasA = next(a, i);
			auto const hasB = next(b, j);
			if (!hasA || !hasB)
				return hasA == hasB;
			if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[j])))
				return false;
			++i;
			++j;
		}
	}

	class _modeCrsKit Projection final : public IProjection
	{
		std::string _methodName;
		std::vector<Parameter> _parameters;

	public:
		Projection(std::string const& className, std::vector<Parameter> parameters);
		static auto FromWkt(Wkt::TokenWkt const& token) -> std::shared_ptr<Projection>;
		static auto AddSemiAxes(std::shared_ptr<IProjection> const& projection, double semiMajorAxis, double semiMinorAxis) -> std::shared_ptr<IProjection>;
		static auto ImprimeIProjectionComoMathTransform(std::shared_ptr<IProjection> const& projection) -> std::string;
		static auto ImprimeIProjectionComoProjection(std::shared_ptr<IProjection> const& projection) -> std::string;

#pragma region IProjection members

	public:
		auto GetClassName() const -> std::string override;
		auto SetClassName(std::string const& value) -> void;

		auto GetNumParameters() const -> int override;

		auto GetParameter(int index) const -> Parameter override;
		auto SetParameter(int index, Parameter const& value) -> void;

		auto GetParameter(std::string const& name) const -> Parameter override;
		auto GetParameters() const noexcept -> std::vector<Parameter> const& override { return _parameters; }
#pragma endregion
	};
}
