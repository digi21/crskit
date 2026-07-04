#pragma once

#include <compare>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "LinearUnit.h"

namespace CrsKit::CoordinateSystems
{
	// Value type (cf. AxisInfo): a reference ellipsoid. Holds its linear unit by value.
	class _modeCrsKit Ellipsoid final
	{
		std::string _name;
		std::string _authority;
		int _authorityCode{};
		std::string _alias;
		std::string _remarks;
		double _semiMajorAxis{};
		double _inverseFlattening{};
		bool _computedInverseFlattening{};
		LinearUnit _linearUnit;

	public:
		Ellipsoid() = default;
		Ellipsoid(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, LinearUnit const& linearUnit);

		auto operator<=>(Ellipsoid const&) const = default;

		static auto FromWkt(Wkt::TokenWkt const& ellipsoidToken) -> Ellipsoid;
		static auto CreateFromSemiMajorAxisSemiMinorAxis(std::string const& name, std::string const& authority, int code, double semiMajorAxis, double semiMinorAxis, LinearUnit const& linearUnit) -> Ellipsoid;
		static auto CreateFromSemiMajorAxisInverseFlattening(std::string const& name, std::string const& authority, int code, double semiMajorAxis, double inverseFlattening, LinearUnit const& linearUnit) -> Ellipsoid;

		[[nodiscard]] auto GetName() const -> std::string { return _name; }
		[[nodiscard]] auto GetAuthority() const -> std::string { return _authority; }
		[[nodiscard]] auto GetAuthorityCode() const -> int { return _authorityCode; }
		[[nodiscard]] auto GetAlias() const -> std::string { return _alias; }
		[[nodiscard]] auto GetRemarks() const -> std::string { return _remarks; }

		[[nodiscard]] auto GetSemiMajorAxis() const noexcept -> double { return _semiMajorAxis; }
		[[nodiscard]] auto GetSemiMinorAxis() const -> double;
		[[nodiscard]] auto GetInverseFlattening() const noexcept -> double { return _inverseFlattening; }
		[[nodiscard]] auto GetIvfDefinitive() const noexcept -> bool { return !_computedInverseFlattening; }
		[[nodiscard]] auto GetLinearUnit() const noexcept -> LinearUnit const& { return _linearUnit; }
		[[nodiscard]] auto GetFlattening() const -> double;
		[[nodiscard]] auto GetEccentricity() const -> double;
		[[nodiscard]] auto GetSecondEccentricity() const -> double;

		// Radius of curvature in the meridian plane at latitude phi: rho = a(1-e2)/(1-e2 sin2 phi)^3/2
		[[nodiscard]] auto GetRadiusOfCurvatureInTheMeridian(double phi) const -> double;
		// Radius of curvature perpendicular to the meridian at phi: nu = a/(1-e2 sin2 phi)^1/2
		[[nodiscard]] auto GetRadiusOfCurvatureInThePrimeVertical(double phi) const -> double;
		// Radius of the sphere with the same surface as the ellipsoid.
		[[nodiscard]] auto GetRadiusOfAuthalicSphere() const -> double;
		// Conformal-sphere radius (function of latitude; use phi0/phi1, or RA for equal-area).
		[[nodiscard]] auto GetRadiusOfConformalSphere(double phi) const -> double;

		[[nodiscard]] auto GetWkt() const -> std::string;
	};
}
