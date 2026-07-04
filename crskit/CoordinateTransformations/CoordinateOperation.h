#pragma once

#include <optional>
#include <string>

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
	};

}