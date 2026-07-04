#include "pch.h"
#include "../Geographic3DToGravityRelatedHeightEGM2008.h"
#include "GridEGM2008.h"
#include "../../../CrsContext.h"

constexpr auto TILE_CACHE_WIDTH = 100;

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::GravityRelatedHeightEGM2008GridFiles
{
	GridEgm2008::GridEgm2008(std::string const& fileName, int rows, int columns)
		: _rows{rows}
		, _columns{columns}
		, _fileName{fileName}
	{
		auto const filePath = GetDefaultContext()->dataDirectory + fileName;

		_file.open(std::filesystem::path(filePath), std::ios::binary);
		if (!_file.is_open())
			throw CrsKit::GridFileNotFoundException{ fileName, filePath };
	}

	auto GridEgm2008::ComputeUndulation(Point2D<double> const& geoSexa) -> float
	{
		if (!std::isfinite(geoSexa.x) || !std::isfinite(geoSexa.y))
			throw CoordinateOutsideDomainException(std::format(
				"Non-finite geographical coordinates passed to the geoid model file: \"{}\".",
				GetFilePath().c_str()));

		auto longitude = static_cast<float>(geoSexa.x);
		auto latitude = static_cast<float>(geoSexa.y);

		// The longitude must be between 0 and 360
		if (longitude < 0)
			longitude += 360.0;

		// The file is built top to bottom, from 90 to -90, so we compute the latitude relative to the top
		latitude = 90 - latitude;

		if (latitude < -0 || latitude >= 180 || longitude < 0 || longitude >= 360)
		{
			throw CoordinateOutsideDomainException(std::format(
				"The geographical coordinates Longitude={:f}, Latitude={:f} are outside the range covered by the geoid model file: \"{}\".",
				geoSexa.x,
				geoSexa.y,
				GetFilePath().c_str()));
		}

		// Grid computations
		auto const gridCellWidth = _columns / 360.0f;
		auto const row = static_cast<float>(latitude * gridCellWidth);
		auto const column = static_cast<float>(longitude * gridCellWidth);

		// In columns we may need to interpolate between the last cell and the first, for example with the coordinate 
		// 39.490655 -0.000278 with the 1-minute file, since the coordinate is -1 second. In this case, when extracting
		// we are going to take the values "a", "b", "c" and "d" from the column as integers; however the interpolation is going to be
		// assuming that instead of column 0 we are interpolating with columns, i.e. we simulate a continuum.
		auto const minColumn = (static_cast<int>(column)) % _columns;
		auto const maxColumn = (static_cast<int>(column) + 1) % _columns;

		// For Y we do not interpolate at all, because the world does not wrap around in the vertical.
		auto const minRow = std::min(_rows-1, (int)row);
		auto const maxRow = std::min(_rows-1, (int)row+1);

		std::lock_guard<std::mutex> lock(_sc);

		if (!(_maxminCache && Point2D<long>(minColumn, minRow)))
			LoadCache(minColumn, minRow);
		auto const a = _cache[TILE_CACHE_WIDTH * (minRow - _maxminCache.Ymin) + (minColumn - _maxminCache.Xmin)];

		if (!(_maxminCache && Point2D<long>(minColumn, maxRow)))
			LoadCache(minColumn, maxRow);
		auto const c = _cache[TILE_CACHE_WIDTH * (maxRow - _maxminCache.Ymin) + (minColumn - _maxminCache.Xmin)];

		if (!(_maxminCache && Point2D<long>(maxColumn, minRow)))
			LoadCache(maxColumn, minRow);
		auto const b = _cache[TILE_CACHE_WIDTH * (minRow - _maxminCache.Ymin) + (maxColumn - _maxminCache.Xmin)];

		if (!(_maxminCache && Point2D<long>(maxColumn, maxRow)))
			LoadCache(maxColumn, maxRow);
		auto const d = _cache[TILE_CACHE_WIDTH * (maxRow - _maxminCache.Ymin) + (maxColumn - _maxminCache.Xmin)];

		auto const f = Interpolations<float>::BilinearInterpolation(
			a,
			b,
			c,
			d,
			static_cast<int>(column) / gridCellWidth,
			(static_cast<int>(column) + 1) / gridCellWidth,
			minRow / gridCellWidth,
			maxRow / gridCellWidth,
			longitude,
			latitude);

		return f;
	}

	auto GridEgm2008::LoadCache(long column, long row) -> void
	{
		_maxminCache = Point2D<long>(column, row);
		_maxminCache.Inflate(50, 50);

		if (_maxminCache.Xmin < 0)
			_maxminCache.Offset(-_maxminCache.Xmin, 0);
		if (_maxminCache.Ymin < 0)
			_maxminCache.Offset(0, -_maxminCache.Ymin);
		if (_maxminCache.Xmax > _columns)
			_maxminCache.Offset(_columns - _maxminCache.Xmax, 0);
		if (_maxminCache.Ymax > _rows)
			_maxminCache.Offset(0, _rows - _maxminCache.Ymax);

		if (_cache.empty())
			_cache.resize(static_cast<size_t>(TILE_CACHE_WIDTH) * TILE_CACHE_WIDTH);

		auto temp = _cache.data();
		// Taken from the help of the Fortran "open" function:
		//
		// http://docs.oracle.com/cd/E19957-01/805-4939/6j4m0vnaf/index.html
		//
		// ##If FORM='UNFORMATTED', each record is preceded and terminated with an INTEGER*4 count, making each record 8 characters longer than normal. This convention is not shared with other languages, so it is useful only for communicating between FORTRAN programs.
		// Which means each record actually has 2 extra floats, one at the start and one at the end that must not be read

		for (int i = _maxminCache.Ymin; i < _maxminCache.Ymax; i++)
		{
			_file.seekg(i * (_columns + 2) * sizeof(float) + 1 * sizeof(float) + _maxminCache.Xmin * sizeof(float), std::ios::beg);
			_file.read(reinterpret_cast<char*>(temp), TILE_CACHE_WIDTH * sizeof(float));
			temp += TILE_CACHE_WIDTH;
		}
	}
}
