#include "pch.h"
#include "Ntv2.h"
#include "../../CrsContext.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	auto sgn(double x) -> int
	{
		if (0.0 == x)
			return 0;
		if (0.0 > x)
			return -1;
		return 1;
	}

	auto Fix(double x) -> int
	{
		return sgn(x) * static_cast<int>(abs(x));
	}

	auto Ntv2Grid::GetFileName() const -> std::string
	{
		return _fileName;
	}

	Ntv2Grid::Ntv2Grid(std::string const& file)
	{
		_fileName = file;

		std::string const filePath = GetDefaultContext()->dataDirectory + file;

		std::ifstream is(std::filesystem::path(filePath), std::ios::binary);

		if (!is.is_open())
			throw CrsKit::GridFileNotFoundException{ file, filePath };

		if (!is.read(reinterpret_cast<char*>(&_cab), sizeof(GridHeader)))
			throw UnsupportedFormatException(std::format("Could not read the header of the file: {}. Its format may be incorrect.", filePath.c_str()));

		_subGrids.resize(_cab.NUM_FILE);
		for (auto i = 0; i < _cab.NUM_FILE; i++)
		{
			is.read(reinterpret_cast<char*>(&_subGrids[i]), sizeof(SubGridHeader));
			// Round (not truncate) the node count: the increment may not divide the span exactly in
			// floating point (e.g. 119.99999 instead of 120), which would drop a whole row/column.
			_subGrids[i].nLats = 1 + static_cast<int>(std::lround((_subGrids[i].CabSub.N_LAT - _subGrids[i].CabSub.S_LAT) / _subGrids[i].CabSub.LAT_INC));
			_subGrids[i].nLons = 1 + static_cast<int>(std::lround((_subGrids[i].CabSub.W_LONG - _subGrids[i].CabSub.E_LONG) / _subGrids[i].CabSub.LONG_INC));

			_subGrids[i].NodeSet.resize(_subGrids[i].CabSub.GSCOUNT);
			is.read(reinterpret_cast<char*>(&(_subGrids[i].NodeSet[0])), sizeof(Node) * _subGrids[i].CabSub.GSCOUNT);
		}
	}

	auto Ntv2Grid::Direct(vector<Point3D<double>> const& points) const -> vector<Point3D<double>>
	{
		auto const maxPoints = points.size();
		vector<Point3D<double>> result;
		result.reserve(maxPoints);

		for (size_t i = 0; i < maxPoints; i++)
			result.push_back(Direct(points[i]));

		return result;
	}

	auto Ntv2Grid::Direct(Point3D<double> const& coordinate) const -> Point3D<double>
	{
		if (!std::isfinite(coordinate.x) || !std::isfinite(coordinate.y))
			throw CoordinateOutsideDomainException(std::format("Non-finite geographical coordinates passed to the geoid model file: \"{}\".", _fileName.c_str()));

		auto LonF = coordinate.x;
		auto LatF = coordinate.y;

		LonF = LonF * 3600;
		LatF = LatF * 3600;

		LonF = -LonF;   // We change its sign. (Does not modify the original data)
		auto nSel = -1; //Number of the subGrid we will select
		auto menorINC = _subGrids[0].CabSub.LAT_INC;
		//We search across all subGrids for the one containing the point with the smallest LAT_INC
		for (auto i = 0; i < _cab.NUM_FILE; i++)
		{
			if (LatF > _subGrids[i].CabSub.N_LAT)
				continue;
			if (LatF < _subGrids[i].CabSub.S_LAT)
				continue;
			if (LonF > _subGrids[i].CabSub.W_LONG)
				continue;
			if (LonF < _subGrids[i].CabSub.E_LONG)
				continue;
			//The point is inside this grid
			//We see whether it is the one with the smallest increment
			if (_subGrids[i].CabSub.LAT_INC <= menorINC)
			{
				menorINC = _subGrids[i].CabSub.LAT_INC;
				nSel = i;
			}
		}

		if (-1 == nSel) {
			throw CoordinateOutsideDomainException(std::format(
				"The geographical coordinates Longitude={:f}, Latitude={:f} are outside the range covered by the geoid model file: \"{}\".",
				coordinate.x,
				coordinate.y,
				_fileName.c_str()));
		}

		//We compute the row and column where the point is
		auto const row = Fix((LatF - _subGrids[nSel].CabSub.S_LAT) / _subGrids[nSel].CabSub.LAT_INC);
		auto const col = Fix((LonF - _subGrids[nSel].CabSub.E_LONG) / _subGrids[nSel].CabSub.LONG_INC);
		// Clamp the +1 neighbour on BOTH axes. Previously only the row (row+1) was clamped while
		// the column (col+1) was not, so a point on the eastern edge read the first node of the
		// next row instead of the edge node.
		auto const col1 = cellUpperClamped(col, _subGrids[nSel].nLons);
		auto const row1 = cellUpperClamped(row, _subGrids[nSel].nLats);
		int p[4];
		p[0] = _subGrids[nSel].nLons * row + col;
		p[1] = _subGrids[nSel].nLons * row + col1;
		p[2] = _subGrids[nSel].nLons * row1 + col;
		p[3] = _subGrids[nSel].nLons * row1 + col1;

		Node point[4];

		point[0] = _subGrids[nSel].NodeSet[p[0]];
		point[1] = _subGrids[nSel].NodeSet[p[1]];
		point[2] = _subGrids[nSel].NodeSet[p[2]];
		point[3] = _subGrids[nSel].NodeSet[p[3]];

		auto const lata = _subGrids[nSel].CabSub.S_LAT + row * _subGrids[nSel].CabSub.LAT_INC;
		auto const lona = _subGrids[nSel].CabSub.E_LONG + col * _subGrids[nSel].CabSub.LONG_INC;
		auto const y = (LatF - lata) / _subGrids[nSel].CabSub.LAT_INC;
		auto const x = (LonF - lona) / _subGrids[nSel].CabSub.LONG_INC;

		// Bilinear over the cell (x,y already normalised to [0,1]); same single evaluator as the rest.
		auto const ip = Interpolations<double>::BilinearInterpolation(
			point[0].ilat, point[1].ilat, point[2].ilat, point[3].ilat, 0, 1, 0, 1, x, y);
		auto LatT = LatF + ip;

		auto const ib = Interpolations<double>::BilinearInterpolation(
			point[0].ilon, point[1].ilon, point[2].ilon, point[3].ilon, 0, 1, 0, 1, x, y);
		auto LonT = LonF + ib;

		LonT = -LonT;

		LatT = LatT / 3600;
		LonT = LonT / 3600;

		return { LonT, LatT, coordinate.z };
	}

	auto Ntv2Grid::Direct(std::vector<double> const& coordinate) const -> std::vector<double>
	{
		if (!std::isfinite(coordinate[0]) || !std::isfinite(coordinate[1]))
			throw CoordinateOutsideDomainException(std::format("Non-finite geographical coordinates passed to the geoid model file: \"{}\".", _fileName.c_str()));

		auto LonF = coordinate[0];
		auto LatF = coordinate[1];

		LonF = LonF * 3600;
		LatF = LatF * 3600;

		LonF = -LonF;   // We change its sign. (Does not modify the original data)
		auto nSel = -1; //Number of the subGrid we will select
		auto menorINC = _subGrids[0].CabSub.LAT_INC;
		//We search across all subGrids for the one containing the point with the smallest LAT_INC
		for (auto i = 0; i < _cab.NUM_FILE; i++)
		{
			if (LatF > _subGrids[i].CabSub.N_LAT)
				continue;
			if (LatF < _subGrids[i].CabSub.S_LAT)
				continue;
			if (LonF > _subGrids[i].CabSub.W_LONG)
				continue;
			if (LonF < _subGrids[i].CabSub.E_LONG)
				continue;
			//The point is inside this grid
			//We see whether it is the one with the smallest increment
			if (_subGrids[i].CabSub.LAT_INC <= menorINC)
			{
				menorINC = _subGrids[i].CabSub.LAT_INC;
				nSel = i;
			}
		}

		if (-1 == nSel) {
			throw CoordinateOutsideDomainException(std::format(
				"The geographical coordinates Longitude={:f}, Latitude={:f} are outside the range covered by the geoid model file: \"{}\".",
				coordinate[0],
				coordinate[1],
				_fileName.c_str()));
		}

		//We compute the row and column where the point is
		auto const row = Fix((LatF - _subGrids[nSel].CabSub.S_LAT) / _subGrids[nSel].CabSub.LAT_INC);
		auto const col = Fix((LonF - _subGrids[nSel].CabSub.E_LONG) / _subGrids[nSel].CabSub.LONG_INC);
		// Clamp the +1 neighbour on BOTH axes. Previously only the row (row+1) was clamped while
		// the column (col+1) was not, so a point on the eastern edge read the first node of the
		// next row instead of the edge node.
		auto const col1 = cellUpperClamped(col, _subGrids[nSel].nLons);
		auto const row1 = cellUpperClamped(row, _subGrids[nSel].nLats);
		int p[4];
		p[0] = _subGrids[nSel].nLons * row + col;
		p[1] = _subGrids[nSel].nLons * row + col1;
		p[2] = _subGrids[nSel].nLons * row1 + col;
		p[3] = _subGrids[nSel].nLons * row1 + col1;

		Node point[4];

		point[0] = _subGrids[nSel].NodeSet[p[0]];
		point[1] = _subGrids[nSel].NodeSet[p[1]];
		point[2] = _subGrids[nSel].NodeSet[p[2]];
		point[3] = _subGrids[nSel].NodeSet[p[3]];

		auto const lata = _subGrids[nSel].CabSub.S_LAT + row * _subGrids[nSel].CabSub.LAT_INC;
		auto const lona = _subGrids[nSel].CabSub.E_LONG + col * _subGrids[nSel].CabSub.LONG_INC;
		auto const y = (LatF - lata) / _subGrids[nSel].CabSub.LAT_INC;
		auto const x = (LonF - lona) / _subGrids[nSel].CabSub.LONG_INC;

		// Bilinear over the cell (x,y already normalised to [0,1]); same single evaluator as the rest.
		auto const ip = Interpolations<double>::BilinearInterpolation(
			point[0].ilat, point[1].ilat, point[2].ilat, point[3].ilat, 0, 1, 0, 1, x, y);
		auto LatT = LatF + ip;

		auto const ib = Interpolations<double>::BilinearInterpolation(
			point[0].ilon, point[1].ilon, point[2].ilon, point[3].ilon, 0, 1, 0, 1, x, y);
		auto LonT = LonF + ib;

		LonT = -LonT;

		LatT = LatT / 3600;
		LonT = LonT / 3600;

		return { LonT, LatT };
	}


	auto Ntv2Grid::Inverse(vector<Point3D<double>> const& points) const -> vector<Point3D<double>>
	{
		auto const maxPoints = points.size();
		vector<Point3D<double>> result;
		result.resize(maxPoints);

		for (size_t i = 0; i < maxPoints; i++)
		{
			auto const t(points[i]);
			auto f(points[i]);

			auto iter = 0;                                   //counter de iteraciones
			constexpr auto tol = 0.0001 * M_PI / 180 / 3600; //we stop at the ten-thousandth of a second

			Point3D<double> computed;
			do
			{
				computed = Direct(f);

				f.x = f.x - (computed.x - t.x);
				f.y = f.y - (computed.y - t.y);
				iter = iter + 1;
			} while (iter < 15 && (abs(computed.x - t.x) > tol || abs(computed.y - t.y) > tol));

			result[i] = f;
		}

		return result;
	}

	auto Ntv2Grid::Inverse(std::vector<double> const& coordinate) const -> std::vector<double>
	{
		vector<double> t;
		t.resize(2);
		t[0] = coordinate[0];
		t[1] = coordinate[1];

		vector<double> f;
		f.resize(2);
		f[0] = coordinate[0];
		f[1] = coordinate[1];

		auto iter = 0;                                   //counter de iteraciones
		constexpr auto tol = 0.0001 * M_PI / 180 / 3600; //we stop at the ten-thousandth of a second

		std::vector<double> computed;
		do
		{
			computed = Direct(f);

			f[0] = f[0] - (computed[0] - t[0]);
			f[1] = f[1] - (computed[1] - t[1]);
			iter = iter + 1;
		} while (iter < 15 && (abs(computed[0] - t[0]) > tol || abs(computed[1] - t[1]) > tol));

		return f;
	}

	Ntv2::Ntv2(std::string const& latitudeAndLongitudeDifferenceFile)
		: _file{new Ntv2Grid(latitudeAndLongitudeDifferenceFile)}
		, _inverse{ false }
	{
	}

	Ntv2::Ntv2(std::string const& latitudeAndLongitudeDifferenceFile, bool inverse)
		: _file{new Ntv2Grid(latitudeAndLongitudeDifferenceFile)}
		, _inverse{inverse}
	{
	}

	Ntv2::Ntv2(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: _file{new Ntv2Grid(get<std::string>(parameters->GetParameter("Latitude_and_longitude_difference_file").GetValue()))}
		, _inverse{inverse}
	{
	}

	Ntv2::Ntv2(std::shared_ptr<IProjection> const& parameters)
		: _file{new Ntv2Grid(get<std::string>(parameters->GetParameter("Latitude_and_longitude_difference_file").GetValue()))}
		, _inverse{ false }
	{
	}
#pragma endregion

#pragma region IMathTransform members
	auto Ntv2::GetWkt() const -> std::string
	{
		if (_inverse)
			return std::format("INVERSE_MT[PARAM_MT[\"ntv2\", PARAMETER[\"Latitude_and_longitude_difference_file\", {}]]]",
				_file->GetFileName().c_str());

		return std::format("PARAM_MT[\"ntv2\", PARAMETER[\"Latitude_and_longitude_difference_file\", {}]]",
			_file->GetFileName().c_str());
	}

	auto Ntv2::GetSourceDimension() const -> int
	{
		return 2;
	}

	auto Ntv2::GetTargetDimension() const -> int
	{
		return 2;
	}

	auto Ntv2::GetIsIdentity() const -> bool
	{
		return false;
	}

	auto Ntv2::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		if (!_inverse)
			return _file->Direct(point);

		return _file->Inverse(point);
	}

	auto Ntv2::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<Ntv2>(_file->GetFileName(), !_inverse);
	}
#pragma endregion
}
