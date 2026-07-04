#pragma once

#include <string>

#include "../CrsKitExport.h"
#include "BaseInfo.h"
#include "IDatum.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit Datum
	: public BaseInfo
	, public IDatum
	{
	public:
		Datum(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, int datumType);
	};
}
