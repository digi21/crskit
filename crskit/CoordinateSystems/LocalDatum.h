#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../StringUtil.h"
#include "../TokenWkt.h"
#include "Datum.h"
#include "IInfo.h"
#include "ILocalDatum.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit LocalDatum final
	: public Datum
	, public ILocalDatum
	{
	public:
		LocalDatum(std::string const& name, int datumType);
		LocalDatum(std::string const& name, int datumType, std::string const& authority, int authorityCode);
		static auto FromWkt(Wkt::TokenWkt const& tokenDatum) -> std::shared_ptr<LocalDatum>;

#pragma region IInfo members

	public:
		auto GetWkt() const -> std::string override;
#pragma endregion

#pragma region System::IComparable<ILocalDatum*> members
		auto CompareTo(std::shared_ptr<ILocalDatum> const& obj) const -> int
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
#pragma endregion
	};
}
