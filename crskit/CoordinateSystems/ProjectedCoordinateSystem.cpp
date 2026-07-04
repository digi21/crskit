#include "pch.h"
#include "BaseInfo.h"
#include "CoordinateSystem.h"
#include "HorizontalCoordinateSystem.h"
#include "GeographicCoordinateSystem.h"
#include "Projection.h"
#include "LinearUnit.h"
#include "ProjectedCoordinateSystem.h"
#include "CoordinateSystemAuthorityFactory.h"
#include "Wkt2.h"
#include "../CrsContext.h"
#include "../EsriProjectionEngine.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	ProjectedCoordinateSystem::ProjectedCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::shared_ptr<GeographicCoordinateSystem> const& _geographicCoordinateSystem, std::shared_ptr<IProjection> const& projection, LinearUnit const& _linearUnit, AxisInfo const& ejes0, AxisInfo const& ejes1)
		: HorizontalCoordinateSystem{name, authority, authorityCode, alias, info, nullptr, _linearUnit, ejes0, ejes1}
		, _geographicCoordinateSystem{_geographicCoordinateSystem}
		, _linearUnit{_linearUnit}
	{
		_projection = Projection::AddSemiAxes(
			projection,
			_geographicCoordinateSystem->GetHorizontalDatum()->GetEllipsoid().GetLinearUnit().GetMetersPerUnit() * _geographicCoordinateSystem->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(),
			_geographicCoordinateSystem->GetHorizontalDatum()->GetEllipsoid().GetLinearUnit().GetMetersPerUnit() * _geographicCoordinateSystem->GetHorizontalDatum()->GetEllipsoid().GetSemiMinorAxis());
	}

	auto ProjectedCoordinateSystem::FromWkt(Wkt::TokenWkt const& tokenPROJCS) -> std::shared_ptr<ProjectedCoordinateSystem>
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenPROJCS);

		auto axis = AxisInfo::FromWkt(tokenPROJCS);
		if (0 == axis.size())
		{
			axis.resize(2);
			axis[0] = AxisInfo("X", AxisOrientationEnum::East);
			axis[1] = AxisInfo("Y", AxisOrientationEnum::North);
		}

		if (!authority.empty())
		{
			if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
				return dynamic_pointer_cast<ProjectedCoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(authorityCode, axis));
		}
		else if (CatalogWins())
		{
			// No AUTHORITY node: we try to recognise the CRS by name and use the catalogue version.
			// BUT a datum carrying explicit TOWGS84 parameters is taken at face value: snapping it to a
			// same-named catalogue entry would drop the Bursa-Wolf parameters the caller supplied, so we
			// keep the WKT definition as-is. The datum sits at PROJCS -> GEOGCS -> DATUM.
			if (!tokenPROJCS.GetChild(1).GetChild(1).HasChild("TOWGS84"))
			{
				// We check whether it is of type EsriProjectionEngine
				auto const epsgCode = EsriProjectionEngine::FindHorizontalEpsgCodeByName(tokenPROJCS.GetChild(0).GetString());

				if (0 != epsgCode)
					return GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(epsgCode, axis);
			}
		}

		// Position 1 of a PROJCS must be GEOGCS
		if (compareNoCase(tokenPROJCS.GetChild(1).GetName().c_str(), "GEOGCS"))
			throw WktParseException("GEOGCS");

		// Position 2 of a PROJCS must be PROJECTION
		if (compareNoCase(tokenPROJCS.GetChild(2).GetName().c_str(), "PROJECTION"))
			throw WktParseException("PROJECTION");


		return make_shared<ProjectedCoordinateSystem>(
			tokenPROJCS.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			GeographicCoordinateSystem::FromWkt(tokenPROJCS.GetChild(1)),
			Projection::FromWkt(tokenPROJCS),
			LinearUnit::FromWkt(tokenPROJCS.GetChild("UNIT")),
			axis[0],
			axis[1]);
	}

#pragma region GeographicCoordinateSystem members
	auto ProjectedCoordinateSystem::GetGeographicCoordinateSystem() const -> std::shared_ptr<IGeographicCoordinateSystem>
	{
		return _geographicCoordinateSystem;
	}

	auto ProjectedCoordinateSystem::GetLinearUnit() const -> LinearUnit
	{
		return _linearUnit;
	}

	auto ProjectedCoordinateSystem::GetProjection() const -> std::shared_ptr<IProjection>
	{
		return _projection;
	}
#pragma endregion
#pragma region IBaseInfo members
	auto ProjectedCoordinateSystem::GetWkt(CrsContext const& context) const -> std::string
	{
		auto const version = context.wktVersion;

		if (IsWkt2(version))
			return Wkt::ToWkt2(*this, version);

		// If the geographic-to-projected transformation is not covered by the OpenGis standard, we return
		// nada.
		std::string text;

		if (!_authority.empty() && _authorityCode && !IsEsri(version))
		{
			text = std::format("PROJCS[\"{}\",{},{},{},{},{},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				_geographicCoordinateSystem->GetWkt(context).c_str(),
				Projection::ImprimeIProjectionComoProjection(_projection).c_str(),
				_linearUnit.GetWkt().c_str(),
				_axes[0].GetWkt().c_str(),
				_axes[1].GetWkt().c_str(),
				_authority.c_str(),
				_authorityCode);
		}
		else
		{
			text = std::format("PROJCS[\"{}\",{},{},{},{},{}]",
				_name.c_str(),
				_geographicCoordinateSystem->GetWkt(context).c_str(),
				Projection::ImprimeIProjectionComoProjection(_projection).c_str(),
				_linearUnit.GetWkt().c_str(),
				_axes[0].GetWkt().c_str(),
				_axes[1].GetWkt().c_str());
		}

		if (!_authority.empty() && _authorityCode && IsEsri(version))
			text = EsriProjectionEngine::HorizontalWkt(_authorityCode, text, IsEsriWithEpsgCode(version));

		return text;
	}
#pragma endregion
}
