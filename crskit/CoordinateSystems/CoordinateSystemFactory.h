#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "AngularUnit.h"
#include "AxisInfo.h"
#include "CompoundCoordinateSystem.h"
#include "CoordinateSystem.h"
#include "Ellipsoid.h"
#include "GeographicCoordinateSystem.h"
#include "IHorizontalDatum.h"
#include "ILocalDatum.h"
#include "IProjection.h"
#include "IVerticalDatum.h"
#include "LinearUnit.h"
#include "LocalCoordinateSystem.h"
#include "PrimeMeridian.h"
#include "ProjectedCoordinateSystem.h"
#include "Unit.h"
#include "Wgs84ConversionInfo.h"

namespace CrsKit::CoordinateSystems
{
	///	<summary>
	///		Contruye objecos complejos a partir de objetos simples o values.
	///	</summary>
	///	<remarks>
	///		ICoordinateSystemFactory lets applications create coordinate systems that cannot be created by an ICoordinateSystemAuthorityFactory. This factory is very
	///		flexible, whereas authority factories are easier to use.
	///
	///		Therefore ICoordinateSystemAuthorityFactory can be used to create "standard" coordinate systems and ICoordinateSystemFactory can be used to
	///		create "special" coordinate systems.
	///
	///		For example, the EPSG authority has codes for "USA state plane" coordinate systems using the NAD83 datum, but those coordinate systems always use metres.
	///		EPSG has no codes for "NAD83 state plane" coordinate systems that use the foot as unit. This factory lets an application create that system of
	///		hybrid coordinates.
	///	</remarks>
	struct ICoordinateSystemFactory
	{
		[[nodiscard]] virtual auto CreateFromWkt(std::string const& wellKnownText) -> std::shared_ptr<CoordinateSystem> = 0;

		[[nodiscard]] virtual auto CreateCompoundCoordinateSystem(std::string const& name, std::shared_ptr<CoordinateSystem> const& head, std::shared_ptr<CoordinateSystem> const& tail) -> std::shared_ptr<CompoundCoordinateSystem> = 0;

		///	<summary>
		///		Creates an ellipsoid from the values of its radii.
		///	</summary>
		[[nodiscard]] virtual auto CreateEllipsoid(std::string const& name, double semiMajorAxis, double semiMinorAxis, LinearUnit const& linearUnit) -> Ellipsoid = 0;

		///	<summary>
		///		Creates an ellipsoid from its radius and its inverse flattening.
		///	</summary>
		[[nodiscard]] virtual auto CreateFlattenedSphere(std::string const& name, double semiMajorAxis, double inverseFlattening, LinearUnit const& linearUnit) -> Ellipsoid = 0;

		///	<summary>
		///		Creates a projected coordinate system using a projection object.
		///	</summary>
		[[nodiscard]] virtual auto CreateProjectedCoordinateSystem(std::string const& name, std::shared_ptr<GeographicCoordinateSystem> const& gcs, std::shared_ptr<IProjection> const& projection, LinearUnit const& linearUnit, AxisInfo axis0, AxisInfo axis1) -> std::shared_ptr<ProjectedCoordinateSystem> = 0;

		///	<summary>
		///		Creates a horizontal datum from its ellipsoid and optional toWGS84 Bursa-Wolf parameters. The parameters are stored on the datum and
		///		emitted in its WKT (TOWGS84[...]); driving coordinate transformations from them is not yet implemented. Pass nullptr when you have no
		///		Bursa-Wolf parameters.
		///	</summary>
		[[nodiscard]] virtual auto CreateHorizontalDatum(std::string const& name, int datumType, Ellipsoid const& ellipsoid, std::shared_ptr<Wgs84ConversionInfo> const& toWGS84) -> std::shared_ptr<IHorizontalDatum> = 0;

		[[nodiscard]] virtual auto CreateLocalCoordinateSystem(std::string const& name, std::shared_ptr<ILocalDatum> const& datum, AnyUnit const& unit, std::vector<AxisInfo> const& arExes) -> std::shared_ptr<LocalCoordinateSystem> = 0;
		[[nodiscard]] virtual auto CreateLocalDatum(std::string const& name, int datumType) -> std::shared_ptr<ILocalDatum> = 0;

		/// <summary>
		///		Create un meridian primario, relativo a Greenwich.
		///	</summary>
		[[nodiscard]] virtual auto CreatePrimeMeridian(std::string const& name, AngularUnit const& angularUnit, double longitude) -> PrimeMeridian = 0;

		///	<summary>
		///		Creates a geographic coordinate system that can be Lat/Lon or Lon/Lat.
		///	</summary>
		[[nodiscard]] virtual auto CreateGeographicCoordinateSystem(std::string const& name, AngularUnit const& angularUnit, std::shared_ptr<IHorizontalDatum> const& horizontalDatum, PrimeMeridian const& primeMeridian, AxisInfo axis0, AxisInfo axis1) -> std::shared_ptr<GeographicCoordinateSystem> = 0;

		///	<summary>
		///		Creates a vertical coordinate system that can be Lat/Lon or Lon/Lat.
		///	</summary>
		//std::shared_ptr<VerticalCoordinateSystem> CreateVerticalCoordinateSystem(std::string name);

		///	<summary>
		///		Creates a vertical datum.
		///	</summary>
		[[nodiscard]] virtual auto CreateVerticalDatum(std::string const& name, int datumType) -> std::shared_ptr<IVerticalDatum> = 0;

		virtual auto ModifyWithAxisNorthEast(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem> = 0;
		virtual auto ModifyWithAxisEastNorth(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem> = 0;
		[[nodiscard]] virtual auto CreateCompoundUnknown() const -> std::shared_ptr<CompoundCoordinateSystem> = 0;
	};

	class _modeCrsKit CoordinateSystemFactory final
	: public ICoordinateSystemFactory
	{
	public:
		[[nodiscard]] auto CreateFromWkt(std::string const& wellKnownText) -> std::shared_ptr<CoordinateSystem> override;
		[[nodiscard]] auto CreateCompoundCoordinateSystem(std::string const& name, std::shared_ptr<CoordinateSystem> const& head, std::shared_ptr<CoordinateSystem> const& tail) -> std::shared_ptr<CompoundCoordinateSystem> override;
		[[nodiscard]] auto CreateEllipsoid(std::string const& name, double semiMajorAxis, double semiMinorAxis, LinearUnit const& linearUnit) -> Ellipsoid override;
		[[nodiscard]] auto CreateFlattenedSphere(std::string const& name, double semiMajorAxis, double inverseFlattening, LinearUnit const& linearUnit) -> Ellipsoid override;
		[[nodiscard]] auto CreatePrimeMeridian(std::string const& name, AngularUnit const& angularUnit, double longitude) -> PrimeMeridian override;
		[[nodiscard]] auto CreateHorizontalDatum(std::string const& name, int datumType, Ellipsoid const& ellipsoid, std::shared_ptr<Wgs84ConversionInfo> const& toWGS84) -> std::shared_ptr<IHorizontalDatum> override;
		[[nodiscard]] auto CreateLocalCoordinateSystem(std::string const& name, std::shared_ptr<ILocalDatum> const& datum, AnyUnit const& unit, std::vector<AxisInfo> const& arExes) -> std::shared_ptr<LocalCoordinateSystem> override;
		[[nodiscard]] auto CreateLocalDatum(std::string const& name, int datumType) -> std::shared_ptr<ILocalDatum> override;
		[[nodiscard]] auto CreateGeographicCoordinateSystem(std::string const& name, AngularUnit const& angularUnit, std::shared_ptr<IHorizontalDatum> const& horizontalDatum, PrimeMeridian const& primeMeridian, AxisInfo axis0, AxisInfo axis1) -> std::shared_ptr<GeographicCoordinateSystem> override;
		[[nodiscard]] auto CreateProjectedCoordinateSystem(std::string const& name, std::shared_ptr<GeographicCoordinateSystem> const& gcs, std::shared_ptr<IProjection> const& projection, LinearUnit const& linearUnit, AxisInfo axis0, AxisInfo axis1) -> std::shared_ptr<ProjectedCoordinateSystem> override;
		//virtual std::shared_ptr<VerticalCoordinateSystem> CreateVerticalCoordinateSystem(std::string name) ;
		[[nodiscard]] auto CreateVerticalDatum(std::string const& name, int datumType) -> std::shared_ptr<IVerticalDatum> override;
		auto ModifyWithAxisNorthEast(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem> override;
		auto ModifyWithAxisEastNorth(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem> override;
		[[nodiscard]] auto CreateCompoundUnknown() const->std::shared_ptr<CompoundCoordinateSystem> override;

		[[nodiscard]] static auto CreateFrom(Wkt::TokenWkt const& token) -> std::shared_ptr<CoordinateSystem>;

		/// <summary>
		///		Returns either the coordinate system passed as parameters or a copy with swapped axes, ensuring the axes are (East,North).
		///	</summary>
		static auto WithAxesEastNorth(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem>;
		static auto WithAxesNorthEast(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem>;
		[[nodiscard]] static auto Create2DSystemFor3DSystem(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem>;
		[[nodiscard]] static auto FindGeographicSystem3D(std::shared_ptr<GeographicCoordinateSystem> const& coordinateSystem) -> std::shared_ptr<GeographicCoordinateSystem>;
		[[nodiscard]] static auto CreateUnknownCompound() -> std::shared_ptr<CompoundCoordinateSystem>;
	};
}
