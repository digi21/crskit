#include "pch.h"

// Bases (the algorithm headers are not self-contained; they expect their base already included).
#include "../CoordinateSystems/Projection.h"
#include "MathTransformBase.h"
#include "Algorithms/MapProjectionBase.h"
#include "Algorithms/Afine.h"

// All built-in algorithms.
#include "Algorithms/AlbersConicEqualArea.h"
#include "Algorithms/AmericanPolyconic.h"
#include "Algorithms/AzimuthalProjections.h"
#include "Algorithms/CassiniSoldner.h"
#include "Algorithms/CoordinateFrameRotation.h"
#include "Algorithms/CylindricalProjections.h"
#include "Algorithms/GeocentricToTopocentric.h"
#include "Algorithms/GeocentricTranslations.h"
#include "Algorithms/Geographic2D2Geocentric.h"
#include "Algorithms/Geographic2D2Geographic3D.h"
#include "Algorithms/Geographic3DToGravityRelatedHeightEGM.h"
#include "Algorithms/Geographic3DToGravityRelatedHeightEGM2008.h"
#include "Algorithms/HotineObliqueMercator.h"
#include "Algorithms/Krovak.h"
#include "Algorithms/LambertAzimuthalEqualArea.h"
#include "Algorithms/LambertConformalConic1SP.h"
#include "Algorithms/LambertConformalConic2SP.h"
#include "Algorithms/LambertConicConformalVariants.h"
#include "Algorithms/LongitudeRotation.h"
#include "Algorithms/Mercator1Sp.h"
#include "Algorithms/Mercator2Sp.h"
#include "Algorithms/MolodenskyBadekas.h"
#include "Algorithms/NadCon.h"
#include "Algorithms/Ntv2.h"
#include "Algorithms/ObliqueMercator.h"
#include "Algorithms/ObliqueStereographic.h"
#include "Algorithms/OtherProjections.h"
#include "Algorithms/P6.h"
#include "Algorithms/PolarStereographic.h"
#include "Algorithms/PopularVisualisationPseudoMercator.h"
#include "Algorithms/PositionVectorTransformation.h"
#include "Algorithms/TransverseMercator.h"
#include "Algorithms/TransverseMercatorSouthOrientated.h"
#include "Algorithms/VerticalOffset.h"

#include "MathTransformRegistry.h"

using namespace std;
using namespace CrsKit;

namespace
{
	// Plain-function replacements for the old MATH_TRANSFORM/AUTHORITY_OPERATION/PARAMETER macros.
	auto mathTransform(std::string name) -> std::shared_ptr<Attribute>
	{
		return std::make_shared<OpenGisMathTransformAttribute>(std::move(name));
	}

	auto authorityOperation(std::string const& authority, int code) -> std::shared_ptr<Attribute>
	{
		return std::make_shared<AuthorityOperationAttribute>(authority, code);
	}

	auto parameter(std::string const& name, ParameterType type, int epsgCode) -> std::shared_ptr<Attribute>
	{
		return std::make_shared<WktParameterAttribute>(name, type, epsgCode);
	}
}

namespace CrsKit
{
	void RegisterBuiltInMathTransforms()
	{
		namespace A = CrsKit::CoordinateTransformations::Algorithms;
		namespace MP = CrsKit::CoordinateTransformations::Algorithms::MapProjections;

		// --- Map projections ---
		MathTransformGenerics::Register<MP::AlbersConicEqualArea>({
			mathTransform("Albers_Conic_Equal_Area"), authorityOperation("EPSG", 9822),
			parameter("latitude_of_center", ParameterType::Angular, 8821),
			parameter("longitude_of_center", ParameterType::Angular, 8822),
			parameter("standard_parallel1", ParameterType::Angular, 8823),
			parameter("standard_parallel2", ParameterType::Angular, 8824),
			parameter("false_easting", ParameterType::Length, 8826),
			parameter("false_northing", ParameterType::Length, 8827) });

		MathTransformGenerics::Register<MP::AmericanPolyconic>({
			mathTransform("polyconic"), authorityOperation("EPSG", 9818),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::ColombiaUrban>({
			mathTransform("Colombia_Urban"), authorityOperation("EPSG", 1052),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807),
			parameter("projection_plane_origin_height", ParameterType::Length, 1039) });

		MathTransformGenerics::Register<MP::AzimuthalEquidistant>({
			mathTransform("Azimuthal_Equidistant"), authorityOperation("EPSG", 1125),
			parameter("latitude_of_center", ParameterType::Angular, 8801),
			parameter("longitude_of_center", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::ModifiedAzimuthalEquidistant>({
			mathTransform("Modified_Azimuthal_Equidistant"), authorityOperation("EPSG", 9832),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::GuamProjection>({
			mathTransform("Guam_Projection"), authorityOperation("EPSG", 9831),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::LocalOrthographic>({
			mathTransform("Local_Orthographic"), authorityOperation("EPSG", 1130),
			parameter("latitude_of_center", ParameterType::Angular, 8811),
			parameter("longitude_of_center", ParameterType::Angular, 8812),
			parameter("azimuth", ParameterType::Angular, 8813),
			parameter("scale_factor", ParameterType::Scalar, 8815),
			parameter("false_easting", ParameterType::Length, 8816),
			parameter("false_northing", ParameterType::Length, 8817) });

		MathTransformGenerics::Register<MP::BonneSouthOrientated>({
			mathTransform("Bonne_South_Orientated"), authorityOperation("EPSG", 9828),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::HyperbolicCassiniSoldner>({
			mathTransform("Hyperbolic_Cassini_Soldner"), authorityOperation("EPSG", 9833),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::LambertConicNearConformal>({
			mathTransform("Lambert_Conic_Near_Conformal"), authorityOperation("EPSG", 9817),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::TransverseMercatorZonedGrid>({
			mathTransform("Transverse_Mercator_Zoned_Grid_System"), authorityOperation("EPSG", 9824),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("initial_longitude", ParameterType::Angular, 8830),
			parameter("zone_width", ParameterType::Angular, 8831),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::NewZealandMapGrid>({
			mathTransform("New_Zealand_Map_Grid"), authorityOperation("EPSG", 9811),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::TunisiaMiningGrid>({
			mathTransform("Tunisia_Mining_Grid"), authorityOperation("EPSG", 9816),
			parameter("latitude_of_origin", ParameterType::Angular, 8821),
			parameter("central_meridian", ParameterType::Angular, 8822),
			parameter("false_easting", ParameterType::Length, 8826),
			parameter("false_northing", ParameterType::Length, 8827) });

		MathTransformGenerics::Register<MP::CassiniSoldner>({
			mathTransform("Cassini_Soldner"), authorityOperation("EPSG", 9806),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::HotineObliqueMercator>({
			mathTransform("hotine_oblique_mercator"), authorityOperation("EPSG", 9812),
			parameter("latitude_of_center", ParameterType::Angular, 8811),
			parameter("longitude_of_center", ParameterType::Angular, 8812),
			parameter("azimuth", ParameterType::Angular, 8813),
			parameter("rectified_grid_angle", ParameterType::Angular, 8814),
			parameter("scale_factor", ParameterType::Scalar, 8815),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		// Krovak and Krovak (North Orientated) share the WKT1 name and the formulas, as do the two modified methods: the CRS
		// axes tell them apart.
		MathTransformGenerics::Register<MP::Krovak>({
			mathTransform("Krovak"), authorityOperation("EPSG", 9819),
			parameter("latitude_of_center", ParameterType::Angular, 8811),
			parameter("longitude_of_center", ParameterType::Angular, 8833),
			parameter("azimuth", ParameterType::Angular, 1036),
			parameter("pseudo_standard_parallel_1", ParameterType::Angular, 8818),
			parameter("scale_factor", ParameterType::Scalar, 8819),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::KrovakNorthOrientated>({
			mathTransform("Krovak"), authorityOperation("EPSG", 1041),
			parameter("latitude_of_center", ParameterType::Angular, 8811),
			parameter("longitude_of_center", ParameterType::Angular, 8833),
			parameter("azimuth", ParameterType::Angular, 1036),
			parameter("pseudo_standard_parallel_1", ParameterType::Angular, 8818),
			parameter("scale_factor", ParameterType::Scalar, 8819),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::KrovakModified>({
			mathTransform("Krovak_Modified"), authorityOperation("EPSG", 1042),
			parameter("latitude_of_center", ParameterType::Angular, 8811),
			parameter("longitude_of_center", ParameterType::Angular, 8833),
			parameter("azimuth", ParameterType::Angular, 1036),
			parameter("pseudo_standard_parallel_1", ParameterType::Angular, 8818),
			parameter("scale_factor", ParameterType::Scalar, 8819),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807),
			parameter("evaluation_point_ordinate_1", ParameterType::Length, 8617),
			parameter("evaluation_point_ordinate_2", ParameterType::Length, 8618),
			parameter("C1", ParameterType::Scalar, 1026),
			parameter("C2", ParameterType::Scalar, 1027),
			parameter("C3", ParameterType::Scalar, 1028),
			parameter("C4", ParameterType::Scalar, 1029),
			parameter("C5", ParameterType::Scalar, 1030),
			parameter("C6", ParameterType::Scalar, 1031),
			parameter("C7", ParameterType::Scalar, 1032),
			parameter("C8", ParameterType::Scalar, 1033),
			parameter("C9", ParameterType::Scalar, 1034),
			parameter("C10", ParameterType::Scalar, 1035) });

		MathTransformGenerics::Register<MP::KrovakModifiedNorthOrientated>({
			mathTransform("Krovak_Modified"), authorityOperation("EPSG", 1043),
			parameter("latitude_of_center", ParameterType::Angular, 8811),
			parameter("longitude_of_center", ParameterType::Angular, 8833),
			parameter("azimuth", ParameterType::Angular, 1036),
			parameter("pseudo_standard_parallel_1", ParameterType::Angular, 8818),
			parameter("scale_factor", ParameterType::Scalar, 8819),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807),
			parameter("evaluation_point_ordinate_1", ParameterType::Length, 8617),
			parameter("evaluation_point_ordinate_2", ParameterType::Length, 8618),
			parameter("C1", ParameterType::Scalar, 1026),
			parameter("C2", ParameterType::Scalar, 1027),
			parameter("C3", ParameterType::Scalar, 1028),
			parameter("C4", ParameterType::Scalar, 1029),
			parameter("C5", ParameterType::Scalar, 1030),
			parameter("C6", ParameterType::Scalar, 1031),
			parameter("C7", ParameterType::Scalar, 1032),
			parameter("C8", ParameterType::Scalar, 1033),
			parameter("C9", ParameterType::Scalar, 1034),
			parameter("C10", ParameterType::Scalar, 1035) });

		MathTransformGenerics::Register<MP::LambertAzimuthalEqualArea>({
			mathTransform("Lambert_Azimuthal_Equal_Area"), authorityOperation("EPSG", 9820),
			parameter("latitude_of_center", ParameterType::Angular, 8801),
			parameter("longitude_of_center", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::LambertConformalConic1SP>({
			mathTransform("Lambert_Conformal_Conic_1SP"), authorityOperation("EPSG", 9801),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::LambertConformalConic2SP>({
			mathTransform("Lambert_Conformal_Conic_2SP"), authorityOperation("EPSG", 9802),
			parameter("latitude_of_origin", ParameterType::Angular, 8821),
			parameter("central_meridian", ParameterType::Angular, 8822),
			parameter("standard_parallel1", ParameterType::Angular, 8823),
			parameter("standard_parallel2", ParameterType::Angular, 8824),
			parameter("false_easting", ParameterType::Length, 8826),
			parameter("false_northing", ParameterType::Length, 8827) });

		MathTransformGenerics::Register<MP::LambertConicConformalWestOrientated>({
			mathTransform("Lambert_Conformal_Conic_West_Orientated"), authorityOperation("EPSG", 9826),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::LambertConicConformal1SPVariantB>({
			mathTransform("Lambert_Conformal_Conic_1SP_Variant_B"), authorityOperation("EPSG", 1102),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("latitude_of_false_origin", ParameterType::Angular, 8821),
			parameter("central_meridian", ParameterType::Angular, 8822),
			parameter("false_easting", ParameterType::Length, 8826),
			parameter("false_northing", ParameterType::Length, 8827) });

		MathTransformGenerics::Register<MP::LambertConicConformal2SPMichigan>({
			mathTransform("Lambert_Conformal_Conic_2SP_Michigan"), authorityOperation("EPSG", 1051),
			parameter("latitude_of_origin", ParameterType::Angular, 8821),
			parameter("central_meridian", ParameterType::Angular, 8822),
			parameter("standard_parallel1", ParameterType::Angular, 8823),
			parameter("standard_parallel2", ParameterType::Angular, 8824),
			parameter("false_easting", ParameterType::Length, 8826),
			parameter("false_northing", ParameterType::Length, 8827),
			parameter("ellipsoid_scaling_factor", ParameterType::Scalar, 1038) });

		MathTransformGenerics::Register<MP::LambertConicConformal2SPBelgium>({
			mathTransform("Lambert_Conformal_Conic_2SP_Belgium"), authorityOperation("EPSG", 9803),
			parameter("latitude_of_origin", ParameterType::Angular, 8821),
			parameter("central_meridian", ParameterType::Angular, 8822),
			parameter("standard_parallel1", ParameterType::Angular, 8823),
			parameter("standard_parallel2", ParameterType::Angular, 8824),
			parameter("false_easting", ParameterType::Length, 8826),
			parameter("false_northing", ParameterType::Length, 8827) });

		MathTransformGenerics::Register<MP::MercatorSpherical>({
			mathTransform("Mercator_1SP_Spherical"), authorityOperation("EPSG", 9841),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::EqualEarth>({
			mathTransform("Equal_Earth"), authorityOperation("EPSG", 1078),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::LambertCylindricalEqualArea>({
			mathTransform("Cylindrical_Equal_Area"), authorityOperation("EPSG", 9835),
			parameter("standard_parallel1", ParameterType::Angular, 8823),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::LambertCylindricalEqualAreaSpherical>({
			mathTransform("Cylindrical_Equal_Area_Spherical"), authorityOperation("EPSG", 9834),
			parameter("standard_parallel1", ParameterType::Angular, 8823),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		// 1028 and its deprecated code 9842 share the WKT1 name and the computation, as do 1029 and 9823.
		MathTransformGenerics::Register<MP::EquidistantCylindrical>({
			mathTransform("Equirectangular"), authorityOperation("EPSG", 1028),
			parameter("standard_parallel1", ParameterType::Angular, 8823),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::EquidistantCylindricalDeprecated>({
			mathTransform("Equirectangular"), authorityOperation("EPSG", 9842),
			parameter("standard_parallel1", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8822),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::EquidistantCylindricalSpherical>({
			mathTransform("Equirectangular_Spherical"), authorityOperation("EPSG", 1029),
			parameter("standard_parallel1", ParameterType::Angular, 8823),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::EquidistantCylindricalSphericalDeprecated>({
			mathTransform("Equirectangular_Spherical"), authorityOperation("EPSG", 9823),
			parameter("standard_parallel1", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::LambertAzimuthalEqualAreaSpherical>({
			mathTransform("Lambert_Azimuthal_Equal_Area_Spherical"), authorityOperation("EPSG", 1027),
			parameter("latitude_of_center", ParameterType::Angular, 8801),
			parameter("longitude_of_center", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::Mercator1Sp>({
			mathTransform("Mercator_1SP"), authorityOperation("EPSG", 9804),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::Mercator2Sp>({
			mathTransform("Mercator_2SP"), authorityOperation("EPSG", 9805),
			parameter("standard_parallel_1", ParameterType::Angular, 8823),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::ObliqueMercator>({
			mathTransform("oblique_mercator"), authorityOperation("EPSG", 9815),
			parameter("latitude_of_center", ParameterType::Angular, 8811),
			parameter("longitude_of_center", ParameterType::Angular, 8812),
			parameter("azimuth", ParameterType::Angular, 8813),
			parameter("rectified_grid_angle", ParameterType::Angular, 8814),
			parameter("scale_factor", ParameterType::Scalar, 8815),
			parameter("false_easting", ParameterType::Length, 8816),
			parameter("false_northing", ParameterType::Length, 8817) });

		MathTransformGenerics::Register<MP::ObliqueStereographic>({
			mathTransform("Oblique_Stereographic"), authorityOperation("EPSG", 9809),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		// Variants A and B share the WKT1 name Polar_Stereographic, as GDAL writes them. Whichever of the two types the name
		// resolves to, the variant is chosen from the parameters.
		MathTransformGenerics::Register<MP::PolarStereographic>({
			mathTransform("Polar_Stereographic"), authorityOperation("EPSG", 9810),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::PolarStereographicVariantB>({
			mathTransform("Polar_Stereographic"), authorityOperation("EPSG", 9829),
			parameter("latitude_of_origin", ParameterType::Angular, 8832),
			parameter("central_meridian", ParameterType::Angular, 8833),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::PolarStereographicVariantC>({
			mathTransform("Polar_Stereographic_Variant_C"), authorityOperation("EPSG", 9830),
			parameter("latitude_of_origin", ParameterType::Angular, 8832),
			parameter("central_meridian", ParameterType::Angular, 8833),
			parameter("false_easting", ParameterType::Length, 8826),
			parameter("false_northing", ParameterType::Length, 8827) });

		MathTransformGenerics::Register<MP::PopularVisualisationPseudoMercator>({
			mathTransform("Popular_Visualisation_Pseudo_Mercator"), authorityOperation("EPSG", 1024),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::TransverseMercator>({
			mathTransform("Transverse_Mercator"), authorityOperation("EPSG", 9807),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		MathTransformGenerics::Register<MP::TransverseMercatorSouthOrientated>({
			mathTransform("Transverse_Mercator_South_Orientated"), authorityOperation("EPSG", 9808),
			parameter("latitude_of_origin", ParameterType::Angular, 8801),
			parameter("central_meridian", ParameterType::Angular, 8802),
			parameter("scale_factor", ParameterType::Scalar, 8805),
			parameter("false_easting", ParameterType::Length, 8806),
			parameter("false_northing", ParameterType::Length, 8807) });

		// --- Other algorithms ---
		MathTransformGenerics::Register<A::Affine>({
			mathTransform("Affine") });

		MathTransformGenerics::Register<A::CoordinateFrameRotation>({
			mathTransform("coordinate_frame_rotation"), authorityOperation("EPSG", 1032),
			parameter("x_axis_translation", ParameterType::Length, 8605),
			parameter("y_axis_translation", ParameterType::Length, 8606),
			parameter("z_axis_translation", ParameterType::Length, 8607),
			parameter("x_axis_rotation", ParameterType::Angular, 8608),
			parameter("y_axis_rotation", ParameterType::Angular, 8609),
			parameter("z_axis_rotation", ParameterType::Angular, 8610),
			parameter("scale_difference", ParameterType::Scalar, 8611) });

		MathTransformGenerics::Register<A::GeocentricToTopocentric>({
			mathTransform("Geographic_To_Topocentric"), mathTransform("Topocentric_To_Geographic"),
			authorityOperation("EPSG", 9836),
			parameter("semi_major", ParameterType::Length, 0),
			parameter("semi_minor", ParameterType::Length, 0),
			parameter("geocentric_x_of_topocentric_origin", ParameterType::Length, 8837),
			parameter("geocentric_y_of_topocentric_origin", ParameterType::Length, 8838),
			parameter("geocentric_z_of_topocentric_origin", ParameterType::Length, 8839) });

		MathTransformGenerics::Register<A::GeocentricTranslations>({
			mathTransform("geocentric_translations"), authorityOperation("EPSG", 1031),
			parameter("x_axis_translation", ParameterType::Length, 8605),
			parameter("y_axis_translation", ParameterType::Length, 8606),
			parameter("z_axis_translation", ParameterType::Length, 8607) });

		MathTransformGenerics::Register<A::GeographicToGeocentric>({
			mathTransform("Ellipsoid_To_Geocentric"), mathTransform("Geocentric_To_Ellipsoid"),
			authorityOperation("EPSG", 9602),
			parameter("semi_major", ParameterType::Length, 0),
			parameter("semi_minor", ParameterType::Length, 0) });

		MathTransformGenerics::Register<A::Geographic2D2Geographic3D>({
			authorityOperation("EPSG", 9659) });

		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightEGM>({
			mathTransform("Geographic3DToGravityRelatedHeightEGM"), authorityOperation("EPSG", 9661),
			parameter("geoid_model_file", ParameterType::String, 8666) });

		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightEGM2008>({
			mathTransform("Geographic3DToGravityRelatedHeightEGM2008"), authorityOperation("EPSG", 1025),
			parameter("geoid_model_file", ParameterType::String, 8666) });

		// Same algorithm (apply a geoid undulation), different grid file format: the ".gtx" binary
		// geoid grid. The reader is picked from the file itself (see CreateLoaderFromFileName).
		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightGtx>({
			mathTransform("Geographic3DToGravityRelatedHeightGtx"), authorityOperation("EPSG", 9665),
			parameter("geoid_model_file", ParameterType::String, 8666) });

		// Same algorithm again for the ".tif" geodetic GeoTIFF ("gtg") geoid method — the format PROJ
		// standardised on and that cdn.proj.org serves the whole grid catalogue as.
		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightGtg>({
			mathTransform("Geographic3DToGravityRelatedHeightGtg"), authorityOperation("EPSG", 1123),
			parameter("geoid_model_file", ParameterType::String, 8666) });

		// AUSGeoid: NTv2 (".gsb") geoid grids carrying the separation N in the first grid field. Two
		// EPSG methods (98 and v2), same reader.
		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightAusGeoid98>({
			mathTransform("Geographic3DToGravityRelatedHeightAUSGeoid98"), authorityOperation("EPSG", 9662),
			parameter("geoid_model_file", ParameterType::String, 8666) });
		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightAusGeoidV2>({
			mathTransform("Geographic3DToGravityRelatedHeightAUSGeoidV2"), authorityOperation("EPSG", 1048),
			parameter("geoid_model_file", ParameterType::String, 8666) });

		MathTransformGenerics::Register<A::LongitudeRotation>({
			mathTransform("longitude_rotation"), authorityOperation("EPSG", 9601),
			parameter("rotation", ParameterType::Angular, 8602) });

		MathTransformGenerics::Register<A::MolodenskyBadekas>({
			mathTransform("molodensky_badekas"), authorityOperation("EPSG", 9636),
			parameter("x_axis_translation", ParameterType::Length, 8605),
			parameter("y_axis_translation", ParameterType::Length, 8606),
			parameter("z_axis_translation", ParameterType::Length, 8607),
			parameter("x_axis_rotation", ParameterType::Angular, 8608),
			parameter("y_axis_rotation", ParameterType::Angular, 8609),
			parameter("z_axis_rotation", ParameterType::Angular, 8610),
			parameter("scale_difference", ParameterType::Scalar, 8611),
			parameter("ordinate_1_of_evaluation_point", ParameterType::Length, 8617),
			parameter("ordinate_2_of_evaluation_point", ParameterType::Length, 8618),
			parameter("ordinate_3_of_evaluation_point", ParameterType::Length, 8667) });

		MathTransformGenerics::Register<A::NadCon>({
			mathTransform("nadcon"), authorityOperation("EPSG", 9613),
			parameter("latitude_difference_file", ParameterType::String, 8657),
			parameter("longitude_difference_file", ParameterType::String, 8658) });

		MathTransformGenerics::Register<A::Ntv2>({
			mathTransform("ntv2"), authorityOperation("EPSG", 9615),
			parameter("Latitude_and_longitude_difference_file", ParameterType::String, 8656) });

		MathTransformGenerics::Register<A::P6>({
			mathTransform("p6"), authorityOperation("EPSG", 9666),
			parameter("bin_grid_origin_i", ParameterType::Length, 8733),
			parameter("bin_grid_origin_j", ParameterType::Length, 8734),
			parameter("bin_grid_origin_easting", ParameterType::Length, 8735),
			parameter("bin_grid_origin_northing", ParameterType::Length, 8736),
			parameter("scale_factor", ParameterType::Scalar, 8737),
			parameter("bin_width_of_i_axis", ParameterType::Length, 8738),
			parameter("bin_width_of_j_axis", ParameterType::Length, 8739),
			parameter("map_grid_bearing_of_bin_grid_j_axis", ParameterType::Angular, 8740),
			parameter("bin_node_increment_on_i_axis", ParameterType::Length, 8741),
			parameter("bin_node_increment_on_j_axis", ParameterType::Length, 8742) });

		MathTransformGenerics::Register<A::PositionVectorTransformation>({
			authorityOperation("EPSG", 1033),
			parameter("x_axis_translation", ParameterType::Length, 8605),
			parameter("y_axis_translation", ParameterType::Length, 8606),
			parameter("z_axis_translation", ParameterType::Length, 8607),
			parameter("x_axis_rotation", ParameterType::Length, 8608),
			parameter("y_axis_rotation", ParameterType::Length, 8609),
			parameter("z_axis_rotation", ParameterType::Length, 8610),
			parameter("scale_difference", ParameterType::Scalar, 8611) });

		MathTransformGenerics::Register<A::VerticalOffset>({
			mathTransform("vertical_offset"), authorityOperation("EPSG", 9616),
			parameter("vertical_offset", ParameterType::Length, 8603) });
	}
}
