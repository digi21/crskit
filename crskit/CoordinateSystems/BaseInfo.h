#pragma once

#include <string>

#include "../CrsKitExport.h"
#include "IInfo.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit BaseInfo : virtual public IInfo
	{
	protected:
		std::string _name;
		std::string _authority;
		int _authorityCode{};
		std::string _alias;
		std::string _remarks;

		BaseInfo(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info)
			: _name{name}
			, _authority{authority}
			, _authorityCode{authorityCode}
			, _alias{alias}
			, _remarks{info}
		{
		}

		explicit BaseInfo(std::string const& name)
			: _name{name}
		{
		}

	public:
		auto GetName() const -> std::string override { return _name; }

		auto GetAuthority() const -> std::string override { return _authority; }

		auto GetAuthorityCode() const -> int override { return _authorityCode; }

		auto GetAlias() const -> std::string override { return _alias; }

		auto GetRemarks() const -> std::string override { return _remarks; }
	};
}
