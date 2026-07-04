#include "pch.h"

// Bases (the algorithm headers are not self-contained; they expect their base already included).
#include "../CoordinateSystems/Projection.h"
#include "MathTransformBase.h"
#include "Algorithms/MapProjectionBase.h"
#include "Algorithms/Afine.h"

// All built-in algorithms.
#include "Algorithms/AlbersConicEqualArea.h"
#include "Algorithms/AmericanPolyconic.h"
#include "Algorithms/CassiniSoldner.h"
#include "Algorithms/CoordinateFrameRotation.h"
#include "Algorithms/GeocentricToTopocentric.h"
#include "Algorithms/GeocentricTranslations.h"
#include "Algorithms/Geographic2D2Geocentric.h"
#include "Algorithms/Geographic2D2Geographic3D.h"
#include "Algorithms/Geographic3DToGravityRelatedHeightEGM.h"
#include "Algorithms/Geographic3DToGravityRelatedHeightEGM2008.h"
#include "Algorithms/HotineObliqueMercator.h"
#include "Algorithms/LambertAzimuthalEqualArea.h"
#include "Algorithms/LambertConformalConic1SP.h"
#include "Algorithms/LambertConformalConic2SP.h"
#include "Algorithms/LongitudeRotation.h"
#include "Algorithms/Mercator1Sp.h"
#include "Algorithms/Mercator2Sp.h"
#include "Algorithms/MolodenskyBadekas.h"
#include "Algorithms/NadCon.h"
#include "Algorithms/Ntv2.h"
#include "Algorithms/ObliqueMercator.h"
#include "Algorithms/ObliqueStereographic.h"
#include "Algorithms/P6.h"
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

	auto parameter(std::string const& name, ParameterType type) -> std::shared_ptr<Attribute>
	{
		return std::make_shared<WktParameterAttribute>(name, type);
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
			parameter("latitude_of_center", ParameterType::Angular),
			parameter("longitude_of_center", ParameterType::Angular),
			parameter("standard_parallel1", ParameterType::Angular),
			parameter("standard_parallel2", ParameterType::Angular),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::AmericanPolyconic>({
			mathTransform("polyconic"), authorityOperation("EPSG", 9818),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::CassiniSoldner>({
			mathTransform("Cassini_Soldner"), authorityOperation("EPSG", 9806),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::HotineObliqueMercator>({
			mathTransform("hotine_oblique_mercator"), authorityOperation("EPSG", 9812),
			parameter("latitude_of_center", ParameterType::Angular),
			parameter("longitude_of_center", ParameterType::Angular),
			parameter("azimuth", ParameterType::Angular),
			parameter("rectified_grid_angle", ParameterType::Angular),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::LambertAzimuthalEqualArea>({
			mathTransform("Lambert_Azimuthal_Equal_Area"), authorityOperation("EPSG", 9820),
			parameter("latitude_of_center", ParameterType::Angular),
			parameter("longitude_of_center", ParameterType::Angular),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::LambertConformalConic1SP>({
			mathTransform("Lambert_Conformal_Conic_1SP"), authorityOperation("EPSG", 9801),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::LambertConformalConic2SP>({
			mathTransform("Lambert_Conformal_Conic_2SP"), authorityOperation("EPSG", 9802),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("standard_parallel1", ParameterType::Angular),
			parameter("standard_parallel2", ParameterType::Angular),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::Mercator1Sp>({
			mathTransform("Mercator_1SP"), authorityOperation("EPSG", 9804),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::Mercator2Sp>({
			mathTransform("Mercator_2SP"), authorityOperation("EPSG", 9805),
			parameter("standard_parallel_1", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::ObliqueMercator>({
			mathTransform("oblique_mercator"), authorityOperation("EPSG", 9815),
			parameter("latitude_of_center", ParameterType::Angular),
			parameter("longitude_of_center", ParameterType::Angular),
			parameter("azimuth", ParameterType::Angular),
			parameter("rectified_grid_angle", ParameterType::Angular),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::ObliqueStereographic>({
			mathTransform("Oblique_Stereographic"), authorityOperation("EPSG", 9809),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::PopularVisualisationPseudoMercator>({
			mathTransform("Popular_Visualisation_Pseudo_Mercator"), authorityOperation("EPSG", 1024),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::TransverseMercator>({
			mathTransform("Transverse_Mercator"), authorityOperation("EPSG", 9807),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		MathTransformGenerics::Register<MP::TransverseMercatorSouthOrientated>({
			mathTransform("Transverse_Mercator_South_Orientated"), authorityOperation("EPSG", 9808),
			parameter("latitude_of_origin", ParameterType::Angular),
			parameter("central_meridian", ParameterType::Angular),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("false_easting", ParameterType::Length),
			parameter("false_northing", ParameterType::Length) });

		// --- Other algorithms ---
		MathTransformGenerics::Register<A::Affine>({
			mathTransform("Affine") });

		MathTransformGenerics::Register<A::CoordinateFrameRotation>({
			mathTransform("coordinate_frame_rotation"), authorityOperation("EPSG", 1032),
			parameter("x_axis_translation", ParameterType::Length),
			parameter("y_axis_translation", ParameterType::Length),
			parameter("z_axis_translation", ParameterType::Length),
			parameter("x_axis_rotation", ParameterType::Angular),
			parameter("y_axis_rotation", ParameterType::Angular),
			parameter("z_axis_rotation", ParameterType::Angular),
			parameter("scale_difference", ParameterType::Scalar) });

		MathTransformGenerics::Register<A::GeocentricToTopocentric>({
			mathTransform("Geographic_To_Topocentric"), mathTransform("Topocentric_To_Geographic"),
			authorityOperation("EPSG", 9836),
			parameter("semi_major", ParameterType::Length),
			parameter("semi_minor", ParameterType::Length),
			parameter("geocentric_x_of_topocentric_origin", ParameterType::Length),
			parameter("geocentric_y_of_topocentric_origin", ParameterType::Length),
			parameter("geocentric_z_of_topocentric_origin", ParameterType::Length) });

		MathTransformGenerics::Register<A::GeocentricTranslations>({
			mathTransform("geocentric_translations"), authorityOperation("EPSG", 1031),
			parameter("x_axis_translation", ParameterType::Length),
			parameter("y_axis_translation", ParameterType::Length),
			parameter("z_axis_translation", ParameterType::Length) });

		MathTransformGenerics::Register<A::GeographicToGeocentric>({
			mathTransform("Ellipsoid_To_Geocentric"), mathTransform("Geocentric_To_Ellipsoid"),
			authorityOperation("EPSG", 9602),
			parameter("semi_major", ParameterType::Length),
			parameter("semi_minor", ParameterType::Length) });

		MathTransformGenerics::Register<A::Geographic2D2Geographic3D>({
			authorityOperation("EPSG", 9659) });

		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightEGM>({
			mathTransform("Geographic3DToGravityRelatedHeightEGM"), authorityOperation("EPSG", 9661),
			parameter("geoid_model_file", ParameterType::String) });

		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightEGM2008>({
			mathTransform("Geographic3DToGravityRelatedHeightEGM2008"), authorityOperation("EPSG", 1025),
			parameter("geoid_model_file", ParameterType::String) });

		// Same algorithm (apply a geoid undulation), different grid file format: the ".gtx" binary
		// geoid grid. The reader is picked from the file itself (see CreateLoaderFromFileName).
		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightGtx>({
			mathTransform("Geographic3DToGravityRelatedHeightGtx"), authorityOperation("EPSG", 9665),
			parameter("geoid_model_file", ParameterType::String) });

		// Same algorithm again for the ".tif" geodetic GeoTIFF ("gtg") geoid method — the format PROJ
		// standardised on and that cdn.proj.org serves the whole grid catalogue as.
		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightGtg>({
			mathTransform("Geographic3DToGravityRelatedHeightGtg"), authorityOperation("EPSG", 1123),
			parameter("geoid_model_file", ParameterType::String) });

		// AUSGeoid: NTv2 (".gsb") geoid grids carrying the separation N in the first grid field. Two
		// EPSG methods (98 and v2), same reader.
		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightAusGeoid98>({
			mathTransform("Geographic3DToGravityRelatedHeightAUSGeoid98"), authorityOperation("EPSG", 9662),
			parameter("geoid_model_file", ParameterType::String) });
		MathTransformGenerics::Register<A::Geographic3DToGravityRelatedHeightAusGeoidV2>({
			mathTransform("Geographic3DToGravityRelatedHeightAUSGeoidV2"), authorityOperation("EPSG", 1048),
			parameter("geoid_model_file", ParameterType::String) });

		MathTransformGenerics::Register<A::LongitudeRotation>({
			mathTransform("longitude_rotation"), authorityOperation("EPSG", 9601),
			parameter("rotation", ParameterType::Angular) });

		MathTransformGenerics::Register<A::MolodenskyBadekas>({
			mathTransform("molodensky_badekas"), authorityOperation("EPSG", 9636),
			parameter("x_axis_translation", ParameterType::Length),
			parameter("y_axis_translation", ParameterType::Length),
			parameter("z_axis_translation", ParameterType::Length),
			parameter("x_axis_rotation", ParameterType::Angular),
			parameter("y_axis_rotation", ParameterType::Angular),
			parameter("z_axis_rotation", ParameterType::Angular),
			parameter("scale_difference", ParameterType::Scalar),
			parameter("ordinate_1_of_evaluation_point", ParameterType::Length),
			parameter("ordinate_2_of_evaluation_point", ParameterType::Length),
			parameter("ordinate_3_of_evaluation_point", ParameterType::Length) });

		MathTransformGenerics::Register<A::NadCon>({
			mathTransform("nadcon"), authorityOperation("EPSG", 9613),
			parameter("latitude_difference_file", ParameterType::String),
			parameter("longitude_difference_file", ParameterType::String) });

		MathTransformGenerics::Register<A::Ntv2>({
			mathTransform("ntv2"), authorityOperation("EPSG", 9615),
			parameter("Latitude_and_longitude_difference_file", ParameterType::String) });

		MathTransformGenerics::Register<A::P6>({
			mathTransform("p6"), authorityOperation("EPSG", 9666),
			parameter("bin_grid_origin_i", ParameterType::Length),
			parameter("bin_grid_origin_j", ParameterType::Length),
			parameter("bin_grid_origin_easting", ParameterType::Length),
			parameter("bin_grid_origin_northing", ParameterType::Length),
			parameter("scale_factor", ParameterType::Scalar),
			parameter("bin_width_of_i_axis", ParameterType::Length),
			parameter("bin_width_of_j_axis", ParameterType::Length),
			parameter("map_grid_bearing_of_bin_grid_j_axis", ParameterType::Angular),
			parameter("bin_node_increment_on_i_axis", ParameterType::Length),
			parameter("bin_node_increment_on_j_axis", ParameterType::Length) });

		MathTransformGenerics::Register<A::PositionVectorTransformation>({
			authorityOperation("EPSG", 1033),
			parameter("x_axis_translation", ParameterType::Length),
			parameter("y_axis_translation", ParameterType::Length),
			parameter("z_axis_translation", ParameterType::Length),
			parameter("x_axis_rotation", ParameterType::Length),
			parameter("y_axis_rotation", ParameterType::Length),
			parameter("z_axis_rotation", ParameterType::Length),
			parameter("scale_difference", ParameterType::Scalar) });

		MathTransformGenerics::Register<A::VerticalOffset>({
			mathTransform("vertical_offset"), authorityOperation("EPSG", 9616),
			parameter("vertical_offset", ParameterType::Length) });
	}
}
