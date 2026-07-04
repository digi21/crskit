#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "../Parameter.h"
#include "../TokenWkt.h"
#include "IProjection.h"

namespace CrsKit::CoordinateSystems
{
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
