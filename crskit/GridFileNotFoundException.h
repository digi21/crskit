#pragma once

#include <format>
#include <string>
#include <utility>

#include "OpenGisException.h"

namespace CrsKit
{
	///	<summary>
	///		Thrown when a grid file referenced by a coordinate operation (geoid, NTv2, NADCON...) cannot be
	///		found in the data directory. Carries enough information for the caller to locate the file
	///		(e.g. to show a dialog or search for it online).
	///	</summary>
	class GridFileNotFoundException : public OpenGisException
	{
	public:
		std::string gridFileName;       // file name as referenced by the EPSG dataset
		std::string searchedPath;       // full path where the file was looked for
		int operationCode{};            // EPSG coordinate operation code (0 if not known at throw site)
		std::string operationName;      // EPSG coordinate operation name
		std::string areaOfUse;          // EPSG area-of-use name
		std::string informationSource;  // EPSG INFORMATION_SOURCE of the operation

		// Basic form, thrown by the grid loader (only the file name and the searched path are known).
		GridFileNotFoundException(std::string gridFileName, std::string searchedPath)
			: OpenGisException{ std::format("Grid file '{}' not found at '{}'.", gridFileName, searchedPath) }
			, gridFileName{ std::move(gridFileName) }
			, searchedPath{ std::move(searchedPath) }
		{
		}

		// Enriched form, rethrown upstream once the EPSG coordinate operation is known.
		GridFileNotFoundException(std::string gridFileName, std::string searchedPath, int operationCode, std::string operationName, std::string areaOfUse, std::string informationSource)
			: OpenGisException{ std::format(
				"Grid file '{}' required by EPSG coordinate operation {} '{}' (area of use: {}) was not found at '{}'. Information source: {}.",
				gridFileName, operationCode, operationName, areaOfUse, searchedPath, informationSource.empty() ? "n/a" : informationSource) }
			, gridFileName{ std::move(gridFileName) }
			, searchedPath{ std::move(searchedPath) }
			, operationCode{ operationCode }
			, operationName{ std::move(operationName) }
			, areaOfUse{ std::move(areaOfUse) }
			, informationSource{ std::move(informationSource) }
		{
		}
	};
}
