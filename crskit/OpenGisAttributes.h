#pragma once

#include <string>
#include <memory>
#include <utility>

#include "Factory.h"		// Attribute
#include "Parameter.h"		// ParameterType

namespace CrsKit
{

// Metadata attributes attached to each math-transform when registered (see
// RegisterBuiltInMathTransforms in MathTransformRegistry.cpp).
struct OpenGisMathTransformAttribute : Attribute
{
	std::string ClassificationName;

	OpenGisMathTransformAttribute(std::string name)
		: ClassificationName{std::move(name)}
	{
	}
};

struct AuthorityOperationAttribute : Attribute
{
	std::string Authority;
	int AuthorityCode;

	AuthorityOperationAttribute(std::string const& authority, int authorityCode)
		: Authority{ authority }
		, AuthorityCode{ authorityCode }
	{
	}
};

struct WktParameterAttribute : Attribute
{
	std::string Name;
	ParameterType Type;
	int EpsgCode;	// EPSG parameter code; 0 when the parameter is not an EPSG one (e.g. semi_major).

	WktParameterAttribute(std::string const& name, ParameterType const& type, int epsgCode = 0)
		: Name{ name }
		, Type{ type }
		, EpsgCode{ epsgCode }
	{
	}
};

}
