#include "pch.h"
#include "GridEGM2008Rednap.h"
#include "../../../CrsContext.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
#pragma region Constructor
	GridEGM2008Rednap::GridEGM2008Rednap(const char* file)
	{
		_filePath = GetDefaultContext()->dataDirectory + file;
		if (!std::filesystem::exists(std::filesystem::path(_filePath)))
			throw CrsKit::GridFileNotFoundException{ file, _filePath };
		Load(_filePath.c_str());
	}
#pragma endregion

#pragma region GridInterpolation members
	auto GridEGM2008Rednap::Load(const char* gridFilePath) -> bool
	{
		std::ifstream is{std::filesystem::path(gridFilePath)};

		if (!is.is_open())
			return false;

		std::string text;
		if (!std::getline(is, text))
			return false;

		auto words{split(text, " \t\r\n,=")};

		if (words.size() < 6)
			return false;

		_origin.y = parse_double(words[0].c_str());
		_origin.x = parse_double(words[1].c_str());
		if (_origin.x > 180)
			_origin.x -= 360;

		_cellSize.y = parse_double(words[2].c_str()) / 60.0; // pasamos a fraction decimal de grado
		_cellSize.x = parse_double(words[3].c_str()) / 60.0; // pasamos a fraction decimal de grado
		_verticalCellCount = parse_int(words[4].c_str());
		_horizontalCellCount = parse_int(words[5].c_str());

		CreateCorrectionsArray();

		auto wordCount = 0;
		auto wordIndex = 0;
		for (auto row = 0; row < _verticalCellCount; row++)
		{
			for (auto column = 0; column < _horizontalCellCount; column++)
			{
				if (0 == wordCount)
				{
					if (!std::getline(is, text))
						return false;

					wordIndex = 0;
					words = split(text, " \t\r\n,=");
					wordCount = static_cast<int>(words.size());
					if (0 == wordCount)
						return false;
				}

				correction(row, column) = static_cast<float>(parse_double(words[wordIndex++].c_str()));
				wordCount--;
			}
		}

		return true;
	}
#pragma endregion

#pragma region IUndulationCalculator members
	auto GridEGM2008Rednap::ComputeUndulation(Point2D<double> const& geoSexa) -> float
	{
		try
		{
			return ComputeBilinearCorrections(geoSexa);
		}
		catch (CrsKit::OpenGisException const&)
		{
			// A point outside the geoid grid cannot be converted; surface it as an error instead of
			// silently returning undulation 0 (which would corrupt the resulting height by the geoid
			// undulation, ~tens of metres). This matches the binary EGM2008 grid, which also throws.
			throw CoordinateOutsideDomainException(std::format(
				"The geographical coordinates Longitude={:f}, Latitude={:f} are outside the range covered by the geoid model file: \"{}\".",
				geoSexa.x,
				geoSexa.y,
				GetFilePath().c_str()));
		}
	}
#pragma endregion
}
