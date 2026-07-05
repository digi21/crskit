#pragma once

#include <optional>
#include <string>
#include <vector>

namespace CrsKit::CoordinateTransformations
{
	struct CoordinateOperation final
	{
		int Code;
		std::string Name;
		std::string Type;
		std::optional<std::string> Version;
		std::optional<int> Variant;
		std::optional<std::string> Scope;
		std::optional<double> Accuracy;
		int MethodCode;
		std::optional<std::string> Remarks;
		std::optional<std::string> InformationSource;

		// EPSG area-of-use name (the geographic extent where the operation applies), "" if none. Lets a
		// caller choose the right operation by geography instead of only by accuracy.
		std::string AreaOfUse;
		// Grid file(s) the operation needs (EPSG PARAM_VALUE_FILE_REF), empty for non-grid operations.
		// The caller can surface these so the user knows which file to obtain (e.g. an EGM2008 geoid grid).
		std::vector<std::string> GridFiles;
	};

}