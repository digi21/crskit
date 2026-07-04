#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../../Mathematics/Point.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
#pragma region Internal structures
	struct GridHeader
	{
		char eti1[8];
		std::int32_t NUM_OREC;    // As Long  'number of records in the header
		std::int32_t padding1;    // As Long
		char eti2[8];     // As System::std::string * 8
		std::int32_t NUM_SREC;    // As Long 'number of records containing the grid headers
		std::int32_t padding2;    // As Long
		char eti3[8];     // As System::std::string * 8
		std::int32_t NUM_FILE;    // As Long 'number de grids
		std::int32_t padding3;    // As Long
		char eti4[8];     // As System::std::string * 8
		char GS_TYPE[8];  // As System::std::string * 8 'units of each node value, generally seconds
		char eti5[8];     // As System::std::string * 8
		char VERSION[8];  // As System::std::string * 8
		char eti6[8];     // As System::std::string * 8
		char SYSTEM_F[8]; // As System::std::string * 8 'source ellipsoid name
		char eti7[8];     // As System::std::string * 8
		char SYSTEM_T[8]; // As System::std::string * 8 'target ellipsoid name
		char eti8[8];     // As System::std::string * 8
		double MAJOR_F;   // As Double 'semiAxis mayor output
		char eti9[8];     // As System::std::string * 8
		double MINOR_F;   // As Double 'semiAxis menor output
		char eti10[8];    // As System::std::string * 8
		double MAJOR_T;   // As Double 'semiAxis mayor llegada
		char eti11[8];    // As System::std::string * 8
		double MINOR_T;   // As Double 'semiAxis menor llegada
	};

	struct SubGridHeader
	{
		char eti1[8];     // As System::std::string * 8
		char SUB_NAME[8]; // As System::std::string * 8 'grid name
		char eti2[8];     // As System::std::string * 8
		char PARENT[8];   // As System::std::string * 8 'name of the containing grid
		char eti3[8];     // As System::std::string * 8
		char CREATED[8];  // As System::std::string * 8 'fecha
		char eti4[8];     // As System::std::string * 8
		char UPDATED[8];  // As System::std::string * 8 'fecha
		char eti5[8];     // As System::std::string * 8
		double S_LAT;     // As Double     'latitude inferior  (S)
		char eti6[8];     // As System::std::string * 8
		double N_LAT;     // As Double     'latitude superior  (N)
		char eti7[8];     // As System::std::string * 8
		double E_LONG;    // As Double    'easternmost longitude
		char eti8[8];     // As System::std::string * 8
		double W_LONG;    // As Double    'westernmost longitude
		char eti9[8];     // As System::std::string * 8
		double LAT_INC;   // As Double   'grid increment in latitude
		char eti10[8];    // As System::std::string * 8
		double LONG_INC;  // As Double  'grid increment in longitude
		char eti11[8];    // As System::std::string * 8
		std::int32_t GSCOUNT;     // As Long     'number of nodes in the grid
		std::int32_t padding;     // As Long
	};

	struct Node
	{
		float ilat; // As Single  'correction de latitude
		float ilon; // As Single  'correction de longitude
		float plat; // As Single  'precision of the latitude correction
		float plon; // As Single  'precision of the longitude correction
	};

	struct SubGrid
	{
		SubGridHeader CabSub;
		std::vector<Node> NodeSet;
		int nLats; // number de points en latitude
		int nLons; // number of points in longitude. The product of both must be GSCOUNT
	};
#pragma endregion
	class Ntv2Grid
	{
#pragma region Private fields
		std::string _fileName;
		GridHeader _cab;
		std::vector<SubGrid> _subGrids;
#pragma endregion

#pragma region Implementation

	public:
		Ntv2Grid(std::string const& filePath);
#pragma endregion
		auto GetFileName() const -> std::string;

		auto Direct(std::vector<double> const& coordinate) const -> std::vector<double>;
		auto Inverse(std::vector<double> const& coordinate) const -> std::vector<double>;

		auto Direct(std::vector<Math::Point3D<double>> const& points) const -> std::vector<Math::Point3D<double>>;
		auto Direct(Math::Point3D<double> const& point) const -> Math::Point3D<double>;
		auto Inverse(std::vector<Math::Point3D<double>> const& points) const -> std::vector<Math::Point3D<double>>;
	};

	class Ntv2 final
	: public IMathTransform
	{

#pragma region Private fields
		std::unique_ptr<Ntv2Grid> _file;
		bool _inverse;
#pragma endregion
#pragma region Constructor

	public:
		explicit Ntv2(std::string const& latitudeAndLongitudeDifferenceFile);
		Ntv2(std::string const& latitudeAndLongitudeDifferenceFile, bool inverse);
		Ntv2(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		explicit Ntv2(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
#pragma region IMathTransform members

	public:
		auto GetWkt() const -> std::string override;

		auto GetSourceDimension() const -> int override;

		auto GetTargetDimension() const -> int override;

		auto GetIsIdentity() const -> bool override;

		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;

		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
