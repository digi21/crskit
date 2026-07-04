#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AngularUnit.h"
#include "AxisInfo.h"
#include "CompoundCoordinateSystem.h"
#include "CoordinateSystem.h"
#include "Ellipsoid.h"
#include "GeocentricCoordinateSystem.h"
#include "GeographicCoordinateSystem.h"
#include "HorizontalCoordinateSystem.h"
#include "IHorizontalDatum.h"
#include "IProjection.h"
#include "IVerticalDatum.h"
#include "LinearUnit.h"
#include "PrimeMeridian.h"
#include "ProjectedCoordinateSystem.h"
#include "VerticalCoordinateSystem.h"

namespace CrsKit::CoordinateSystems
{
	struct CoordinateSystemKind
	{
		static inline const char* Compound = "compound";
		static inline const char* Derived = "derived";
		static inline const char* Engineering = "engineering";
		static inline const char* Geocentric = "geocentric";
		static inline const char* Geographic2D = "geographic 2D";
		static inline const char* Geographic3D = "geographic 3D";
		static inline const char* Projected = "projected";
		static inline const char* Vertical = "vertical";
	};

	struct ICoordinateSystemAuthorityFactory
	{
		///	<summary>
		///		Returns the name of the authority for which this object builds objects.
		///	</summary>
		[[nodiscard]] virtual auto GetAuthority() const -> std::string = 0;

		[[nodiscard]] virtual auto CreateCoordinateSystem(int code) const -> std::shared_ptr<CoordinateSystems::CoordinateSystem> = 0;

		///	<summary>
		///		Returns a ProjectedCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateProjectedCoordinateSystem(int code) const -> std::shared_ptr<ProjectedCoordinateSystem> = 0;
		[[nodiscard]] virtual auto CreateProjectedCoordinateSystem(int code, std::vector<AxisInfo> const& axis) const->std::shared_ptr<ProjectedCoordinateSystem> = 0;
		[[nodiscard]] virtual auto CreateProjectedCoordinateSystem(int projectionCode, int geographicCrsCode) const->std::shared_ptr<ProjectedCoordinateSystem> = 0;

		///	<summary>
		///		Returns a GeographicCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateGeographicCoordinateSystem(int code) const -> std::shared_ptr<GeographicCoordinateSystem> = 0;
		[[nodiscard]] virtual auto CreateGeographicCoordinateSystem(int code, std::vector<AxisInfo> const& axis) const->std::shared_ptr<GeographicCoordinateSystem> = 0;

		[[nodiscard]] virtual auto CreateCompoundCoordinateSystem(int horizontal, int vertical) const -> std::shared_ptr<CoordinateSystems::CompoundCoordinateSystem> = 0;

		///	<summary>
		///		Returns an IHorizontalDatum object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateHorizontalDatum(int code) const -> std::shared_ptr<IHorizontalDatum> = 0;

		///	<summary>
		///		Returns an IEllipsoid object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateEllipsoid(int code) const -> Ellipsoid = 0;

		///	<summary>
		///		Returns a PrimeMeridian object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreatePrimeMeridian(int code) const -> PrimeMeridian = 0;

		///	<summary>
		///		Returns a LinearUnit object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateLinearUnit(int code) const -> LinearUnit = 0;

		///	<summary>
		///		Returns an AngularUnit object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateAngularUnit(int code) const -> AngularUnit = 0;

		///	<summary>
		///		Returns an IVerticalDatum object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateVerticalDatum(int code) const -> std::shared_ptr<IVerticalDatum> = 0;

		///	<summary>
		///		Returns a VerticalCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateVerticalCoordinateSystem(int code) const -> std::shared_ptr<VerticalCoordinateSystem> = 0;

		///	<summary>
		///		Returns a GeocentricCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateGeocentricCoordinateSystem(int code) const -> std::shared_ptr<GeocentricCoordinateSystem> = 0;

		///	<summary>
		///		Returns a HorizontalCoordinateSystem object from an authorityCode.
		///	</summary>
		[[nodiscard]] virtual auto CreateHorizontalCoordinateSystem(int code) const -> std::shared_ptr<HorizontalCoordinateSystem> = 0;

		//

		[[nodiscard]] virtual auto EnumerateCoordinateSystems(std::string const& kind) const -> std::unordered_map<int, std::string> = 0;
		[[nodiscard]] virtual auto GetKindOfCoordinateSystem(int code) const->std::string = 0;

		[[nodiscard]] virtual auto GetDescriptionAreaApplicationCrs(int code) const -> std::string = 0;
		[[nodiscard]] virtual auto GetCodeOfDatumAssociatedWithCrs(int code) const -> int = 0;
		[[nodiscard]] virtual auto GetCodeOfPrimeMeridianAssociatedWithCrs(int code) const -> int = 0;

		[[nodiscard]] virtual auto GetNameOfDatum(int code) const -> std::string = 0;
		[[nodiscard]] virtual auto GetOriginDescriptionOfDatum(int code) const -> std::string = 0;
		[[nodiscard]] virtual auto GetDescriptionAreaApplicationDatum(int code) const -> std::string = 0;
		[[nodiscard]] virtual auto GetCodeOfEllipsoidAssociatedWithDatum(int code) const -> int = 0;

		[[nodiscard]] virtual auto GetNameOfPrimeMeridian(int code) const -> std::string = 0;
		[[nodiscard]] virtual auto GetNameOfEllipsoid(int code) const -> std::string = 0;

		[[nodiscard]] virtual auto GetNameOfAlgorithm(int code) const -> std::string = 0;



		// This should be hidden

		[[nodiscard]] virtual auto GetCodeOfCoordinateSystemAssociatedWithCrs(int code) const -> int = 0;
		[[nodiscard]] virtual auto GetNameOfCoordinateSystem(int code) const->std::string = 0;
		[[nodiscard]] virtual auto GetAxisOfCoordinateSystem(int code) const->std::vector<AxisInfo> = 0;
		[[nodiscard]] virtual auto CreateGeographic2DSystemForGeographic3DSystem(int code, CoordinateSystems::AxisInfo const& axisX, CoordinateSystems::AxisInfo const& axisY) const->std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> = 0;
		[[nodiscard]] virtual auto FindGeographicSystem3D(int code, CoordinateSystems::AxisInfo const& axisX, CoordinateSystems::AxisInfo const& axisY) const->std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> = 0;
		[[nodiscard]] virtual auto CreateProjection(int operationCode, int algorithmCode) const->std::shared_ptr<CoordinateSystems::IProjection> = 0;

	};
}
