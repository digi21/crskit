#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "Datum.h"
#include "Ellipsoid.h"
#include "IHorizontalDatum.h"
#include "Wgs84ConversionInfo.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit HorizontalDatum final
	: public Datum
	, public IHorizontalDatum
	{
		Ellipsoid _ellipsoid;
		// Optional Bursa-Wolf parameters to WGS 84 (the WKT TOWGS84 node); null when absent.
		std::shared_ptr<Wgs84ConversionInfo> _toWgs84;

	public:
		HorizontalDatum(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, int datumType, Ellipsoid const& ellipsoid, std::shared_ptr<Wgs84ConversionInfo> const& toWgs84 = nullptr);
		static auto FromWkt(Wkt::TokenWkt const& tokenDatum) -> std::shared_ptr<HorizontalDatum>;

		[[nodiscard]] auto GetWgs84ConversionInfo() const -> std::shared_ptr<Wgs84ConversionInfo> { return _toWgs84; }

#pragma region IHorizontalDatum members
		auto GetEllipsoid() const -> Ellipsoid override;
		auto CompareTo(std::shared_ptr<IHorizontalDatum> const& obj) const -> int override;
#pragma endregion
#pragma region IInfo members
		auto GetWkt() const -> std::string override;
#pragma endregion
	};
}
