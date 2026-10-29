#include "pch.h"
#include "CoordinateTransformations/Algorithms/Geographic3DToGravityRelatedHeightEGM2008.h"

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace CrsKit
{
	namespace
	{
		// Method of the European Petroleum Survey Group (EPSG) catalogue whose PARAM_VALUE_FILE_REF values are names of
		// coordinate operations.
		constexpr auto kNorwayOffshoreInterpolationMethodCode = 9620;
	}

	bool IsGridFileInstalled(std::string const& gridFileName)
	{
		auto const dataDirectory = GetDefaultContext()->dataDirectory;
		for (auto const& candidateGridFileName : CoordinateTransformations::Algorithms::GetGridFileSearchNames(gridFileName))
		{
			std::error_code gridFileStatusError;
			if (std::filesystem::is_regular_file(std::filesystem::path(dataDirectory + candidateGridFileName), gridFileStatusError))
				return true;
		}
		return false;
	}

	std::vector<std::string> GetMissingGridFiles(CoordinateTransformations::CoordinateOperation const& operation)
	{
		if (operation.MethodCode == kNorwayOffshoreInterpolationMethodCode)
			return {};

		std::vector<std::string> missingGridFileNames;
		for (auto const& gridFileName : operation.GridFiles)
			if (!IsGridFileInstalled(gridFileName))
				missingGridFileNames.push_back(gridFileName);
		return missingGridFileNames;
	}
}
