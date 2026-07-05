#pragma once

#include "GridFileNotFoundException.h"
#include "CrsKitExport.h"

namespace CrsKit::CoordinateTransformations
{
	struct CoordinateOperation;
}

namespace CrsKit
{
	constexpr auto LOCAL_COORDINATE_SYSTEM = 0;
	constexpr auto MEASURE_UNIT_METRE = 9001;
	constexpr auto MEASURE_UNIT_DEGREES = 9122;
	constexpr auto ANGULAR_UNIT_SEXAGESIMAL_DMS = 9110;
}

#include <vector>
#include <variant>
#include <optional>
#include <functional>
#include <stdexcept>
#include <memory>

#include "StringUtil.h"


#include "TokenWkt.h"
#include "WktDeserializer.h"	
#include "Positioning/Envelope.h"
#include "Parameter.h"

// MSVC: silence library-design warnings that also fire when CONSUMERS include these headers
// (Prueba, tests, Python binding, WinUI 3, Digi3D.NET). Scoped with push/pop so the consumer's
// own code keeps its warning settings.
//   4250: 'inherits via dominance' (pure-interface CRS hierarchy with virtual inheritance of IInfo).
//   4251/4275: STL data members / non-dll-interface bases crossing the DLL boundary; benign in an
//              all-MSVC ecosystem, and the concrete classes are intentionally exported (built from
//              components by tests/Python/WinUI), so they cannot be made internal.
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4250 4251 4275)
#endif

// CoordinateSystems
#include "CoordinateSystems/IInfo.h"
#include "CoordinateSystems/AxisOrientationEnum.h"
#include "CoordinateSystems/AxisInfo.h"
#include "CoordinateSystems/BaseInfo.h"

// Value types (units / ellipsoid / prime meridian): plain data, no longer polymorphic interfaces.
// Defined before the CRS interfaces below, which now return them by value.
#include "CoordinateSystems/LinearUnit.h"
#include "CoordinateSystems/AngularUnit.h"
#include "CoordinateSystems/Unit.h"
#include "CoordinateSystems/Ellipsoid.h"
#include "CoordinateSystems/PrimeMeridian.h"

#include "CoordinateSystems/ICoordinateSystem.h"
#include "CoordinateSystems/CoordinateSystem.h"
#include "CoordinateSystems/DatumType.h"
#include "CoordinateSystems/IDatum.h"
#include "CoordinateSystems/Wgs84ConversionInfo.h"
#include "CoordinateSystems/IHorizontalDatum.h"
#include "CoordinateSystems/IHorizontalCoordinateSystem.h"
#include "CoordinateSystems/HorizontalCoordinateSystem.h"
#include "CoordinateSystems/IGeographicCoordinateSystem.h"
#include "CoordinateSystems/GeographicCoordinateSystem.h"
#include "CoordinateSystems/IProjection.h"
#include "CoordinateSystems/IProjectedCoordinateSystem.h"
#include "CoordinateSystems/ProjectedCoordinateSystem.h"
#include "CoordinateSystems/IVerticalDatum.h"
#include "CoordinateSystems/IVerticalCoordinateSystem.h"
#include "CoordinateSystems/VerticalCoordinateSystem.h"
#include "CoordinateSystems/IGeocentricCoordinateSystem.h"
#include "CoordinateSystems/GeocentricCoordinateSystem.h"
#include "CoordinateSystems/ICompoundCoordinateSystem.h"
#include "CoordinateSystems/CompoundCoordinateSystem.h"
#include "CoordinateSystems/ILocalDatum.h"
#include "CoordinateSystems/ILocalCoordinateSystem.h"
#include "CoordinateSystems/LocalCoordinateSystem.h"

// Concrete datums/projection exposed to build coordinate systems from
// components (not only from WKT): GIGS tests, Python binding, WinUI 3.
#include "CoordinateSystems/Datum.h"
#include "CoordinateSystems/HorizontalDatum.h"
#include "CoordinateSystems/VerticalDatum.h"
#include "CoordinateSystems/LocalDatum.h"
#include "CoordinateSystems/Projection.h"

// CoordinateTransformations
#include "CoordinateTransformations/CoordinateOperation.h"
#include "CoordinateTransformations/DomainFlags.h"
#include "CoordinateTransformations/IMathTransform.h"
#include "CoordinateTransformations/TransformType.h"
#include "CoordinateTransformations/ICoordinateTransformation.h"
#include "CoordinateTransformations/CoordinateTransformation.h"
#include "CoordinateTransformations/MathTransformBase.h"
#include "CoordinateTransformations/ConcatenatedTransform.h"
#include "CoordinateTransformations/Passthrough.h"

#include "CoordinateTransformationOptions.h"

#include "CoordinateTransformations/IMathTransformFactory.h"
#include "CoordinateSystems/ICoordinateSystemAuthorityFactory.h"
#include "CoordinateSystems/CoordinateSystemFactory.h"
#include "CoordinateTransformations/ICoordinateTransformationFactory.h"
#include "CoordinateTransformations/ICoordinateTransformationAuthorityFactory.h"
#include "IAuthorityProvider.h"

// Factories concretas e authority factories (construction/instantiation direct
// from external consumers: Python/WinUI 3).
#include "CoordinateTransformations/MathTransformFactory.h"
#include "CoordinateTransformations/CoordinateTransformationFactory.h"
#include "CoordinateTransformations/CoordinateTransformationAuthorityFactory.h"
#include "CoordinateSystems/CoordinateSystemAuthorityFactory.h"

#include "EsriProjectionEngine.h"

#include "CrsContext.h"
#include "IUtilities.h"

namespace CrsKit
{
	// Holds all of the library's mutable state (the default CrsContext, EPSG provider and the factories)
	// in a single object, created by Initialize(). Replaces the former scattered file-static singletons;
	// it can be swapped as a unit -- for tests or multiple contexts at the boundary -- via
	// SetCurrentEnvironment(). Deep code that cannot receive a context per call (grid loaders, the
	// no-arg GetWkt()) reads the default context through the Get* facade below; entry points accept an
	// explicit CrsContext to override it per call.
	struct Environment
	{
		std::shared_ptr<CrsKit::CrsContext> DefaultContext;
		std::shared_ptr<Epsg::IAuthorityProvider> Provider;
		std::shared_ptr<CoordinateSystems::ICoordinateSystemFactory> CoordinateSystemFactory;
		std::shared_ptr<CoordinateTransformations::IMathTransformFactory> MathTransformFactory;
		std::shared_ptr<CoordinateSystems::ICoordinateSystemAuthorityFactory> CoordinateSystemAuthorityFactory;
		std::shared_ptr<CoordinateTransformations::ICoordinateTransformationAuthorityFactory> CoordinateTransformationAuthorityFactory;
		std::shared_ptr<CoordinateTransformations::ICoordinateTransformationFactory> CoordinateTransformationFactory;
		std::shared_ptr<IUtilities> Utilities;
	};

	_modeCrsKit std::shared_ptr<Environment> GetCurrentEnvironment();
	_modeCrsKit void SetCurrentEnvironment(std::shared_ptr<Environment> const& environment);

	_modeCrsKit std::shared_ptr<CrsContext> GetDefaultContext();
	_modeCrsKit bool Initialize(std::shared_ptr<Epsg::IAuthorityProvider> const& authorityProvider);
	_modeCrsKit std::shared_ptr<CoordinateSystems::ICoordinateSystemFactory> GetCoordinateSystemFactory();
	_modeCrsKit std::shared_ptr<CoordinateTransformations::IMathTransformFactory> GetMathTransformFactory();

	_modeCrsKit std::shared_ptr<CoordinateSystems::ICoordinateSystemAuthorityFactory> GetCoordinateSystemAuthorityFactory();
	_modeCrsKit std::shared_ptr<CoordinateTransformations::ICoordinateTransformationAuthorityFactory> GetCoordinateTransformationAuthorityFactory();
	_modeCrsKit std::shared_ptr<CoordinateTransformations::ICoordinateTransformationFactory> GetCoordinateTransformationFactory();

	_modeCrsKit std::shared_ptr<IUtilities> GetUtilities();

	// The EPSG Geodetic Parameter Dataset version of the active database (e.g. "12.057"), "" if unknown.
	_modeCrsKit std::string GetEpsgVersion();
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif