#include "pch.h"
#include "NadCon.h"
#include "../../CrsContext.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	NadConFile::NadConFile(std::string const& path)
		: _filePath{path}
		, _data{}
	{
		std::string const realFilePath = GetDefaultContext()->dataDirectory + path;

		std::ifstream is(std::filesystem::path(realFilePath), std::ios::binary);
		if (!is.is_open())
			throw CrsKit::GridFileNotFoundException{ path, realFilePath };

		is.read(reinterpret_cast<char*>(&_header), sizeof(_header));

		// Records are always [int (which must always be 0)] [float*header.width]

		// The header itself has the same width as each record (header.width*4), so we are going to skip it
		is.seekg(sizeof(int) + _header.Width * sizeof(float), std::ios::beg);

		_data.resize(static_cast<size_t>(_header.Width) * _header.Height);

		for (auto i = 0; i < _header.Height; i++)
		{
			int enteroQueSiempreEs0;
			is.read(reinterpret_cast<char*>(&enteroQueSiempreEs0), sizeof(enteroQueSiempreEs0));
			assert(0 == enteroQueSiempreEs0);

			is.read(reinterpret_cast<char*>(_data.data() + i * _header.Width), _header.Width * sizeof(float));
		}
	}

	auto NadConFile::GetFilePath() const -> std::string { return _filePath; }

	auto NadConFile::ComputeOffset(double longitude, double latitude) const -> float
	{
		// Grid units: x grows east, y grows north by row index (origin is the SW corner).
		// sampleBilinear handles cell location, bounds, NaN/Inf and the bilinear evaluation.
		auto const value = sampleBilinear(_data, _header.Width, _header.Height,
			(longitude - _header.OriginLongitude) / _header.CellLongitudeWidth,
			(latitude - _header.OriginLatitude) / _header.CellLatitudeWidth);
		if (!value)
		{
			throw CoordinateOutsideDomainException(std::format(
				"The geographical coordinates Longitude={:f}, Latitude={:f} are outside the range covered by the geoid model file: \"{}\".",
				longitude,
				latitude,
				GetFilePath().c_str()));
		}

		return static_cast<float>(*value);
	}


	NadCon::NadCon(std::string const& latitudeDifferenceFile, std::string const& longitudeDifferenceFile)
		: _inverse{ false }
		, _gridLatitud{make_unique<NadConFile>(latitudeDifferenceFile)}
		, _gridLongitude{make_unique<NadConFile>(longitudeDifferenceFile)}
	{
	}

	NadCon::NadCon(std::string const& latitudeDifferenceFile, std::string const& longitudeDifferenceFile, bool inverse)
		: _inverse{inverse}
		, _gridLatitud{make_unique<NadConFile>(latitudeDifferenceFile)}
		, _gridLongitude{make_unique<NadConFile>(longitudeDifferenceFile)}
	{
	}

	NadCon::NadCon(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: _inverse{inverse}
		, _gridLatitud{make_unique<NadConFile>(get<std::string>(parameters->GetParameter("latitude_difference_file").GetValue()))}
		, _gridLongitude{make_unique<NadConFile>(get<std::string>(parameters->GetParameter("longitude_difference_file").GetValue()))}
	{
	}

	NadCon::NadCon(std::shared_ptr<IProjection> const& parameters)
		: _inverse{ false }
		, _gridLatitud{make_unique<NadConFile>(get<std::string>(parameters->GetParameter("latitude_difference_file").GetValue()))}
		, _gridLongitude{make_unique<NadConFile>(get<std::string>(parameters->GetParameter("longitude_difference_file").GetValue()))}
	{
	}

#pragma region IMathTransform members
	auto NadCon::GetWkt() const -> std::string
	{
		if (_inverse)
			return std::format("INVERSE_MT[PARAM_MT[\"nadcon\", PARAMETER[\"latitude_difference_file\",{}], PARAMETER[\"longitude_difference_file\", {}]]]",
				_gridLatitud->GetFilePath().c_str(), _gridLongitude->GetFilePath().c_str());

		return std::format("PARAM_MT[\"nadcon\", PARAMETER[\"latitude_difference_file\",{}], PARAMETER[\"longitude_difference_file\", {}]]",
			_gridLatitud->GetFilePath().c_str(), _gridLongitude->GetFilePath().c_str());
	}

	auto NadCon::GetSourceDimension() const -> int
	{
		return 2;
	}

	auto NadCon::GetTargetDimension() const -> int
	{
		return 2;
	}

	auto NadCon::GetIsIdentity() const -> bool
	{
		return false;
	}

	auto NadCon::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		vector<double> result;
		result.resize(this->GetTargetDimension());

		if (!_inverse)
		{
			auto const longitudeCorrection = _gridLongitude->ComputeOffset(point[0], point[1]);
			result[0] = point[0] + longitudeCorrection / 3600;

			auto const latitudeCorrection = _gridLatitud->ComputeOffset(point[0], point[1]);
			result[1] = point[1] + latitudeCorrection / 3600;
		}
		else
		{
			result[0] = point[0];
			result[1] = point[1];

			auto iter = 0;               //counter de iteraciones
			constexpr auto tol = 0.0001; //we stop at the ten-thousandth of a second

			vector<double> computed;
			computed.resize(2);
			do
			{
				computed[0] = result[0] + _gridLongitude->ComputeOffset(result[0], result[1]) / 3600;
				computed[1] = result[1] + _gridLatitud->ComputeOffset(result[0], result[1]) / 3600;

				result[0] = result[0] - (computed[0] - point[0]);
				result[1] = result[1] - (computed[1] - point[1]);
				iter = iter + 1;
			} while (iter < 15 && (abs(computed[0] - point[0]) > tol || abs(computed[1] - point[1]) > tol));
		}

		return result;
	}

	auto NadCon::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<NadCon>(_gridLatitud->GetFilePath(), _gridLongitude->GetFilePath(), !_inverse);
	}
#pragma endregion
}
