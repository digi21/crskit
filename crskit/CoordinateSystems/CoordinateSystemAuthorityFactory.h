#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4146)
#pragma warning(disable: 4305)
#pragma warning(disable: 4309)
#endif

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../CrsKitExport.h"
#include "AngularUnit.h"
#include "ICoordinateSystemAuthorityFactory.h"
#include "Projection.h"
#include "Wgs84ConversionInfo.h"
#include "../IAuthorityProvider.h"

namespace CrsKit::Epsg
{
	class _modeCrsKit CoordinateSystemAuthorityFactory final
		: public CoordinateSystems::ICoordinateSystemAuthorityFactory
	{
		std::shared_ptr<Epsg::IAuthorityProvider> _provider;
	public:
		CoordinateSystemAuthorityFactory(std::shared_ptr<Epsg::IAuthorityProvider> const& authorityProvider);

		///	<summary>
		///		Returns the name of the authority for which this object builds objects.
		///	</summary>
		[[nodiscard]] auto GetAuthority() const->std::string override;

		[[nodiscard]] auto CreateCoordinateSystem(int code) const->std::shared_ptr<CoordinateSystems::CoordinateSystem> override;

		///	<summary>
		///		Returns a ProjectedCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateProjectedCoordinateSystem(int code) const -> std::shared_ptr<CoordinateSystems::ProjectedCoordinateSystem> override;
		[[nodiscard]] auto CreateProjectedCoordinateSystem(int code, std::vector<CoordinateSystems::AxisInfo> const& axis) const -> std::shared_ptr<CoordinateSystems::ProjectedCoordinateSystem> override;
		[[nodiscard]] auto CreateProjectedCoordinateSystem(int projectionCode, int geographicCrsCode) const -> std::shared_ptr<CoordinateSystems::ProjectedCoordinateSystem> override;

		///	<summary>
		///		Returns a GeographicCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateGeographicCoordinateSystem(int code) const -> std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> override;
		[[nodiscard]] auto CreateGeographicCoordinateSystem(int code, std::vector<CoordinateSystems::AxisInfo> const& axis) const -> std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> override;

		[[nodiscard]] auto CreateCompoundCoordinateSystem(int horizontal, int vertical) const -> std::shared_ptr<CoordinateSystems::CompoundCoordinateSystem> override;

		///	<summary>
		///		Returns an IHorizontalDatum object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateHorizontalDatum(int code) const -> std::shared_ptr<CoordinateSystems::IHorizontalDatum> override;

		///	<summary>
		///		Returns an IEllipsoid object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateEllipsoid(int code) const -> CoordinateSystems::Ellipsoid override;

		///	<summary>
		///		Returns an IPrimeMeridian object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreatePrimeMeridian(int code) const -> CoordinateSystems::PrimeMeridian override;

		///	<summary>
		///		Returns an ILinearUnit object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateLinearUnit(int code) const -> CoordinateSystems::LinearUnit override;

		///	<summary>
		///		Returns an IAngularUnit object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateAngularUnit(int code) const -> CoordinateSystems::AngularUnit override;

		///	<summary>
		///		Returns an IVerticalDatum object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateVerticalDatum(int code) const -> std::shared_ptr<CoordinateSystems::IVerticalDatum> override;

		///	<summary>
		///		Returns a VerticalCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateVerticalCoordinateSystem(int code) const -> std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> override;

		[[nodiscard]] auto CreateGeocentricCoordinateSystem(int code) const -> std::shared_ptr<CoordinateSystems::GeocentricCoordinateSystem> override;

		///	<summary>
		///		Returns a CompoundCoordinateSystem object from an authorityCode.
		///	</summary>
		//auto CreateCompoundCoordinateSystem(int code) const -> std::shared_ptr<CoordinateSystems::CompoundCoordinateSystem> override;

		///	<summary>
		///		Returns a HorizontalCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] auto CreateHorizontalCoordinateSystem(int code) const -> std::shared_ptr<CoordinateSystems::HorizontalCoordinateSystem> override;

		[[nodiscard]] auto EnumerateCoordinateSystems(std::string const& kind) const -> std::unordered_map<int, std::string> override;
		[[nodiscard]] auto GetKindOfCoordinateSystem(int code) const->std::string override;
		[[nodiscard]] auto GetDescriptionAreaApplicationCrs(int code) const -> std::string override;
		[[nodiscard]] auto GetCodeOfCoordinateSystemAssociatedWithCrs(int code) const -> int override;
		[[nodiscard]] auto GetNameOfCoordinateSystem(int code) const -> std::string override;
		[[nodiscard]] auto GetAxisOfCoordinateSystem(int code) const -> std::vector <CoordinateSystems::AxisInfo> override;

		[[nodiscard]] auto GetCodeOfDatumAssociatedWithCrs(int code) const -> int override;
		[[nodiscard]] auto GetNameOfDatum(int code) const -> std::string override;
		[[nodiscard]] auto GetOriginDescriptionOfDatum(int code) const -> std::string override;
		[[nodiscard]] auto GetDescriptionAreaApplicationDatum(int code) const -> std::string override;
		[[nodiscard]] auto GetCodeOfEllipsoidAssociatedWithDatum(int code) const -> int override;

		[[nodiscard]] auto GetCodeOfPrimeMeridianAssociatedWithCrs(int code) const -> int override;
		[[nodiscard]] auto GetNameOfPrimeMeridian(int code) const -> std::string override;

		[[nodiscard]] auto GetNameOfEllipsoid(int code) const->std::string override;
		[[nodiscard]] auto GetNameOfAlgorithm(int code) const->std::string override;
		[[nodiscard]] auto ToSexagesimal(int angularUnitCode, double value) const -> double;
		[[nodiscard]] auto CreateProjection(int operationCode, int algorithmCode) const -> std::shared_ptr<CoordinateSystems::IProjection> override;
		[[nodiscard]] auto CreateUnitsInformation(int code) const -> std::vector<CoordinateSystems::AnyUnit>;
		[[nodiscard]] auto CreateUnit(int code) const -> CoordinateSystems::AnyUnit;
		[[nodiscard]] auto CreateAxisInformation(int code) const -> std::vector<CoordinateSystems::AxisInfo>;
		[[nodiscard]] auto CreateGeographic2DSystemForGeographic3DSystem(int code, CoordinateSystems::AxisInfo const& axisX, CoordinateSystems::AxisInfo const& axisY) const -> std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> override;
		[[nodiscard]] auto FindGeographicSystem3D(int code, CoordinateSystems::AxisInfo const& axisX, CoordinateSystems::AxisInfo const& axisY) const -> std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> override;

	private:
		///	<summary>
		///		Resolves the datum's Bursa-Wolf parameters to WGS 84 (for emitting TOWGS84 in WKT) from the
		///		most accurate non-deprecated EPSG transformation expressible as a 7-parameter Helmert
		///		(methods 9603, 9606, 9607). Returns null when there is none (e.g. WGS 84 itself, or only
		///		grid/Molodensky-based transformations exist). Always expressed in the Position Vector
		///		convention, as WKT 1 TOWGS84 is interpreted by GDAL/PROJ.
		///	</summary>
		[[nodiscard]] auto ResolveWgs84ConversionInfo(int datumCode) const -> std::shared_ptr<CoordinateSystems::Wgs84ConversionInfo>;
	};
}
