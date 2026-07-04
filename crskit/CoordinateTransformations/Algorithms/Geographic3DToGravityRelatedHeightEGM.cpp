#include "pch.h"
#include "Geographic3DToGravityRelatedHeightEGM.h"
#include "../../CrsContext.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	Geographic3DToGravityRelatedHeightFile::Geographic3DToGravityRelatedHeightFile(std::string const& path)
		: _filePath{path}
		, _data{}
	{
		std::string const realFilePath = GetDefaultContext()->dataDirectory + path;

		std::ifstream is{std::filesystem::path(realFilePath)};
		if (!is.is_open())
			throw CrsKit::GridFileNotFoundException{ path, realFilePath };

		size_t index = 0;

		std::string text;
		std::getline(is, text);

		auto words = split(text, " \t\r\n,=");

		_header.OriginLatitude = static_cast<float>(parse_double(words[1].c_str()));
		_header.OriginLongitude = static_cast<float>(parse_double(words[2].c_str()));
		_header.CellLongitudeWidth = static_cast<float>(parse_double(words[4].c_str()));
		_header.CellLatitudeWidth = static_cast<float>(parse_double(words[5].c_str()));
		_header.Width = static_cast<int>((parse_double(words[3].c_str()) - _header.OriginLongitude) / _header.CellLongitudeWidth + 1); // The +1 comes from the egm96 readme.txt file
		_header.Height = static_cast<int>((_header.OriginLatitude - parse_double(words[0].c_str())) / _header.CellLatitudeWidth + 1);    // The +1 comes from the egm96 readme.txt file
		_data.resize(static_cast<size_t>(_header.Width) * _header.Height);

		while (std::getline(is, text))
		{
			words = split(text, " \t\r\n,=");
			auto const wordCount = static_cast<int>(words.size());

			for (auto i = 0; i < wordCount; i++)
				_data[index++] = static_cast<float>(parse_double(words[i].c_str()));
		}
	}

	auto Geographic3DToGravityRelatedHeightFile::GetFilePath() const -> std::string
	{
		return _filePath;
	}

	auto Geographic3DToGravityRelatedHeightFile::ComputeOffset(double longitude, double latitude) const -> float
	{
		if (longitude < 0)
			longitude += 360.0;

		if (longitude < _header.OriginLongitude || longitude >(_header.OriginLongitude + static_cast<float>(_header.Width) * _header.CellLongitudeWidth) ||
			latitude > _header.OriginLatitude || latitude < (_header.OriginLatitude - static_cast<float>(_header.Height) * _header.CellLatitudeWidth))
		{
			throw CoordinateOutsideDomainException(std::format("The geographical coordinates Longitude={:f}, Latitude={:f} are outside the range covered by the geoid model file: \"{}\".",
				longitude,
				latitude,
				GetFilePath().c_str()));
		}

		auto const x = (longitude - _header.OriginLongitude) / _header.CellLongitudeWidth;
		auto const xmin = static_cast<int>(x);
		auto const xmax = xmin + 1;

		auto const y = (_header.OriginLatitude - latitude) / _header.CellLatitudeWidth;
		auto const ymin = static_cast<int>(y);
		auto const ymax = ymin + 1;

		auto const a = _data[_header.Width * ymin + xmin];
		auto const b = _data[_header.Width * ymin + xmax];

		auto const c = _data[_header.Width * ymax + xmin];
		auto const d = _data[_header.Width * ymax + xmax];

		return static_cast<float>(Math::Interpolations<double>::BilinearInterpolation(a, b, c, d, xmin, xmax, ymin, ymax, x, y));
	}


	Geographic3DToGravityRelatedHeightEGM::Geographic3DToGravityRelatedHeightEGM(std::string const& geoidModelFile)
		: _inverse{ false }
		, _grid{new Geographic3DToGravityRelatedHeightFile(geoidModelFile)}
	{
	}

	Geographic3DToGravityRelatedHeightEGM::Geographic3DToGravityRelatedHeightEGM(std::string const& geoidModelFile, bool inverse)
		: _inverse{inverse}
		, _grid{new Geographic3DToGravityRelatedHeightFile(geoidModelFile)}
	{
	}

	Geographic3DToGravityRelatedHeightEGM::Geographic3DToGravityRelatedHeightEGM(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: _inverse{inverse}
		, _grid{new Geographic3DToGravityRelatedHeightFile(get<std::string>(parameters->GetParameter("geoid_model_file").GetValue()))}
	{
	}

	Geographic3DToGravityRelatedHeightEGM::Geographic3DToGravityRelatedHeightEGM(std::shared_ptr<IProjection> const& _parameters)
		: _inverse{ false }
		, _grid{new Geographic3DToGravityRelatedHeightFile(get<std::string>(_parameters->GetParameter("geoid_model_file").GetValue()))}
	{
	}

#pragma region IMathTransform members
	auto Geographic3DToGravityRelatedHeightEGM::GetWkt() const -> std::string
	{
		if (_inverse)
		{
			return std::format("INVERSE_MT[PARAM_MT[\"Geographic3DToGravityRelatedHeightEGM\", PARAMETER[\"geoid_model_file\",{}]]]",
				_grid->GetFilePath().c_str());
		}

		return std::format("PARAM_MT[\"Geographic3DToGravityRelatedHeightEGM\", PARAMETER[\"geoid_model_file\",{}]]",
			_grid->GetFilePath().c_str());
	}

	auto Geographic3DToGravityRelatedHeightEGM::GetSourceDimension() const -> int
	{
		return 3;
	}

	auto Geographic3DToGravityRelatedHeightEGM::GetTargetDimension() const -> int
	{
		return 1;
	}

	auto Geographic3DToGravityRelatedHeightEGM::GetIsIdentity() const -> bool
	{
		return false;
	}

	auto Geographic3DToGravityRelatedHeightEGM::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", GetSourceDimension()));

		vector<double> result;
		result.resize(this->GetTargetDimension());

		auto const correction = _grid->ComputeOffset(point[0], point[1]);

		if (_inverse)
			result[0] = point[2] + correction;
		else
			result[0] = point[2] - correction;

		return result;
	}

	auto Geographic3DToGravityRelatedHeightEGM::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<Geographic3DToGravityRelatedHeightEGM>(_grid->GetFilePath(), !_inverse);
	}
#pragma endregion
}
