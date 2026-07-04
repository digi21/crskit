#pragma once

#include <format>
#include <string>
#include <variant>

#include "StringUtil.h"

namespace CrsKit
{

enum class ParameterType
{
	Length,
	Angular,
	Scalar,
	String
};

class Parameter
{
	std::string _name;
	std::variant<std::string, double> _value;

public:
	Parameter() = default;

	Parameter(std::string const& name, double value)
		: _name{name}
		, _value{value}
	{
	}

	Parameter(std::string const& name, std::string value)
		: _name{name}
		, _value{value}
	{
	}

	Parameter(std::string const& name, std::variant<std::string, double> const& value)
		: _name{name}
		, _value{value}
	{
	}

	Parameter(Parameter const& parameter)
		: _name{parameter.GetName()}
		, _value{parameter.GetValue()}
	{
	}

	auto operator<=>(Parameter const&) const = default;

	auto GetName() const -> std::string { return _name; }

	auto GetValue() const -> std::variant<std::string, double> { return _value; }


	auto GetWkt() const -> std::string
	{
		if (_value.index() == 1)
		{
			return std::format("PARAMETER[\"{}\",{:.16g}]",
						_name.c_str(),
						std::get<double>(_value));
		}

		return std::format("PARAMETER[\"{}\",{}]",
							_name.c_str(),
							std::get<std::string>(_value).c_str());
	}
};

}
