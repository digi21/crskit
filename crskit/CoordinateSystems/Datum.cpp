#include "pch.h"
#include "BaseInfo.h"
#include "Datum.h"

namespace CrsKit::CoordinateSystems
{
	Datum::Datum(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, int datumType)
		: BaseInfo{name, authority, authorityCode, alias, info}
		, IDatum{datumType}
	{
	}
}
