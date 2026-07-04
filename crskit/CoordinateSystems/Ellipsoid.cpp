#include "pch.h"
using namespace CrsKit::Math;
#include "Ellipsoid.h"
#include "CoordinateSystemAuthorityFactory.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	Ellipsoid::Ellipsoid(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, LinearUnit const& linearUnit)
		: _name{ name }
		, _authority{ authority }
		, _authorityCode{ authorityCode }
		, _alias{ alias }
		, _remarks{ info }
		, _linearUnit{ linearUnit }
	{
	}

	auto Ellipsoid::FromWkt(Wkt::TokenWkt const& tokenSpheroid) -> Ellipsoid
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenSpheroid);

		if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
			return GetCoordinateSystemAuthorityFactory()->CreateEllipsoid(authorityCode);

		// Built from the pieces; the authority (EPSG or otherwise), when present, is kept as metadata.
		return CreateFromSemiMajorAxisInverseFlattening(
			tokenSpheroid.GetChild(0).GetString(),
			authority,
			authorityCode,
			tokenSpheroid.GetChild(1).GetDouble(),
			tokenSpheroid.GetChild(2).GetDouble(),
			GetCoordinateSystemAuthorityFactory()->CreateLinearUnit(MEASURE_UNIT_METRE));
	}

	auto Ellipsoid::CreateFromSemiMajorAxisSemiMinorAxis(std::string const& name, std::string const& authority, int code, double semiMajorAxis, double semiMinorAxis, LinearUnit const& linearUnit) -> Ellipsoid
	{
		Ellipsoid ellipsoid{ name, authority, code, "", "", linearUnit };

		ellipsoid._semiMajorAxis = semiMajorAxis;
		ellipsoid._inverseFlattening = semiMajorAxis / (semiMajorAxis - semiMinorAxis);
		ellipsoid._computedInverseFlattening = true;

		return ellipsoid;
	}

	auto Ellipsoid::CreateFromSemiMajorAxisInverseFlattening(std::string const& name, std::string const& authority, int code, double semiMajorAxis, double inverseFlattening, LinearUnit const& linearUnit) -> Ellipsoid
	{
		Ellipsoid ellipsoid{ name, authority, code, "", "", linearUnit };

		ellipsoid._semiMajorAxis = semiMajorAxis;
		ellipsoid._inverseFlattening = inverseFlattening;
		ellipsoid._computedInverseFlattening = false;

		return ellipsoid;
	}

	auto Ellipsoid::GetSemiMinorAxis() const -> double
	{
		return _semiMajorAxis - _semiMajorAxis / _inverseFlattening;
	}

	auto Ellipsoid::GetFlattening() const -> double
	{
		return 1.0 / _inverseFlattening;
	}

	auto Ellipsoid::GetEccentricity() const -> double
	{
		return sqrt(2.0 * GetFlattening() - pow(GetFlattening(), 2.0));
	}

	auto Ellipsoid::GetSecondEccentricity() const -> double
	{
		auto const e2 = pow(GetEccentricity(), 2.0);
		return sqrt(e2 / (1.0 - e2));
	}

	auto Ellipsoid::GetRadiusOfCurvatureInTheMeridian(double phi) const -> double
	{
		auto const e2 = pow(GetEccentricity(), 2.0);
		return _semiMajorAxis * (1.0 - e2) / pow(1.0 - e2 * pow(sin(DEGREES_TO_RADIANS(phi)), 2.0), 3.0 / 2.0);
	}

	auto Ellipsoid::GetRadiusOfCurvatureInThePrimeVertical(double phi) const -> double
	{
		auto const e2 = pow(GetEccentricity(), 2.0);
		return _semiMajorAxis / sqrt(1.0 - e2 * pow(sin(DEGREES_TO_RADIANS(phi)), 2.0));
	}

	auto Ellipsoid::GetRadiusOfAuthalicSphere() const -> double
	{
		auto const e = GetEccentricity();
		auto const e2 = pow(e, 2.0);
		return _semiMajorAxis * sqrt((1.0 - ((1.0 - e2) / (2 * e)) * (log((1.0 - e) / (1.0 + e)))) * 0.5);
	}

	auto Ellipsoid::GetRadiusOfConformalSphere(double phi) const -> double
	{
		auto const e2 = pow(GetEccentricity(), 2.0);
		return _semiMajorAxis * sqrt(1.0 - e2) / (1.0 - e2 * pow(sin(DEGREES_TO_RADIANS(phi)), 2.0));
	}

	auto Ellipsoid::GetWkt() const -> std::string
	{
		if (_authorityCode && !_authority.empty())
			return std::format("SPHEROID[\"{}\",{:.16g},{:.16g},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				_linearUnit.GetMetersPerUnit() * _semiMajorAxis,
				_inverseFlattening,
				_authority.c_str(),
				_authorityCode);

		return std::format("SPHEROID[\"{}\", {:.16g}, {:.16g}]",
			_name.c_str(),
			_linearUnit.GetMetersPerUnit() * _semiMajorAxis,
			_inverseFlattening);
	}
}
