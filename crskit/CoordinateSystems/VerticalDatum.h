#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../StringUtil.h"
#include "../TokenWkt.h"
#include "Datum.h"
#include "IInfo.h"
#include "IVerticalDatum.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit VerticalDatum final
	: public Datum
	, public IVerticalDatum
	{
	public:
		VerticalDatum(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, int datumType);
		static auto FromWkt(Wkt::TokenWkt const& tokenDatum) -> std::shared_ptr<VerticalDatum>;

#pragma region IInfo members

	public:
		auto GetWkt() const -> std::string override;
#pragma endregion

		auto CompareTo(std::shared_ptr<IVerticalDatum> const& obj) const -> int
		{
			auto const info = std::dynamic_pointer_cast<IInfo>(obj);

			if (GetAuthorityCode() != UnknownAuthorityCode)
			{
				if (GetAuthority() != info->GetAuthority())
					return compareNoCase(GetAuthority().c_str(), info->GetAuthority().c_str());

				if (GetAuthorityCode() != info->GetAuthorityCode())
					return GetAuthorityCode() - info->GetAuthorityCode();

				return 0;
			}

			return compareNoCase(GetName().c_str(), info->GetName().c_str());
		}
	};
}
