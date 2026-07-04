#include "pch.h"
#include "Projection.h"

namespace CrsKit::CoordinateSystems
{
	Projection::Projection(std::string const& className, std::vector<Parameter> parameters)
		: _methodName{className}
		, _parameters{std::move(parameters)}
	{
	}

	auto Projection::FromWkt(Wkt::TokenWkt const& tokenPROJCS) -> std::shared_ptr<Projection>
	{
		std::vector<Parameter> _parameters;
		for (auto i = 0; i < tokenPROJCS.GetChildCount(); i++)
		{
			try
			{
				if (0 == compareNoCase(tokenPROJCS.GetChild(i).GetName().c_str(), "PARAMETER"))
					_parameters.push_back(Parameter{tokenPROJCS.GetChild(i).GetChild(0).GetString(), tokenPROJCS.GetChild(i).GetChild(1).GetDouble()});
			}
			catch (...)
			{
			}
		}

		return std::make_shared<Projection>(tokenPROJCS.GetChild("PROJECTION").GetChild(0).GetString(), _parameters);
	}

#pragma region IProjection members
	auto Projection::GetClassName() const -> std::string
	{
		return _methodName;
	}

	auto Projection::SetClassName(std::string const& value) -> void
	{
		_methodName = value;
	}

	auto Projection::GetNumParameters() const -> int
	{
		return static_cast<int>(_parameters.size());
	}

	auto Projection::GetParameter(int index) const -> Parameter
	{
		return _parameters[index];
	}

	auto Projection::SetParameter(int index, Parameter const& value) -> void
	{
		_parameters[index] = value;
	}

	auto Projection::GetParameter(std::string const& name) const -> Parameter
	{
		auto const it = std::ranges::find_if(_parameters,
			[&](auto const& parameter) { return 0 == compareNoCase(name.c_str(), parameter.GetName().c_str()); });
		if (it != _parameters.end())
			return *it;

		throw WktParseException("Parameter not found");
	}
#pragma endregion

	auto Projection::AddSemiAxes(std::shared_ptr<IProjection> const& projection, double semiMajorAxis, double semiMinorAxis) -> std::shared_ptr<IProjection>
	{
		auto hasSemiMajorAxis = false;
		auto hasSemiMinorAxis = false;
		for (auto i = 0; i < projection->GetNumParameters(); i++)
		{
			if (0 == compareNoCase(projection->GetParameter(i).GetName().c_str(), "semi_major"))
				hasSemiMajorAxis = true;

			if (0 == compareNoCase(projection->GetParameter(i).GetName().c_str(), "semi_minor"))
				hasSemiMinorAxis = true;
		}

		if (hasSemiMajorAxis && hasSemiMinorAxis)
			return projection;

		std::vector<Parameter> newParameters;
		for (auto i = 0; i < projection->GetNumParameters(); i++)
			newParameters.push_back(projection->GetParameter(i));

		if (!hasSemiMajorAxis)
			newParameters.push_back(Parameter("semi_major", semiMajorAxis));

		if (!hasSemiMinorAxis)
			newParameters.push_back(Parameter("semi_minor", semiMinorAxis));

		return std::make_shared<Projection>(projection->GetClassName(), newParameters);
	}


	auto Projection::ImprimeIProjectionComoMathTransform(std::shared_ptr<IProjection> const& projection) -> std::string
	{
		auto result = std::format("PARAM_MT[\"{}\"", projection->GetClassName().c_str());

		for (auto i = 0; i < projection->GetNumParameters(); i++)
			result += std::format(",{}", projection->GetParameter(i).GetWkt().c_str());

		result += "]";

		return result;
	}

	auto Projection::ImprimeIProjectionComoProjection(std::shared_ptr<IProjection> const& projection) -> std::string
	{
		auto result = std::format("PROJECTION[\"{}\"]", projection->GetClassName().c_str());

		for (auto i = 0; i < projection->GetNumParameters(); i++)
		{
			result += std::format(",{}", projection->GetParameter(i).GetWkt().c_str());;
		}

		return result;
	}
}
