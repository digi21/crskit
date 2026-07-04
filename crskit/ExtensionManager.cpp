#include "pch.h"
#include "ExtensionManager.h"
#include "CoordinateSystems/Projection.h"

#include <cctype>
#include <unordered_map>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;
using namespace std;

namespace
{
	auto toLower(std::string s) -> std::string
	{
		for (auto& c : s)
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		return s;
	}

	// Composite key "authority:code" (e.g. "EPSG:9807") so the code index can be a hashed map too.
	auto codeKey(std::string const& authority, int code) -> std::string
	{
		return authority + ':' + std::to_string(code);
	}

	// O(1) indexes over the registered math-transform metadata, built once on first use (after
	// RegisterBuiltInMathTransforms() has run from Initialize()). Replaces the previous linear scans.
	std::unordered_map<std::string, std::type_index> gNameIndex; // lowercased WKT classification name -> type
	std::unordered_map<std::string, std::type_index> gCodeIndex; // "authority:code" -> type

	void ensureIndexBuilt()
	{
		// Built exactly once even under concurrent first use.
		static std::once_flag once;
		std::call_once(once, []
		{
			for (auto const type : MathTransformGenerics::RegisteredTypes())
			{
				for (auto const& attribute : MathTransformGenerics::GetFilteredMetadata<OpenGisMathTransformAttribute>(type))
					gNameIndex.emplace(toLower(attribute->ClassificationName), type);

				for (auto const& attribute : MathTransformGenerics::GetFilteredMetadata<AuthorityOperationAttribute>(type))
					gCodeIndex.emplace(codeKey(attribute->Authority, attribute->AuthorityCode), type);
			}
		});
	}
}

namespace CrsKit
{
	auto ExtensionManager::FindMathematicalOperation(std::string const& authority, int code) -> std::type_index
	{
		ensureIndexBuilt();

		auto const it = gCodeIndex.find(codeKey(authority, code));
		if (it == gCodeIndex.end())
			throw std::runtime_error("Object not found");

		return it->second;
	}

	auto ExtensionManager::GetMathOperationParameters(std::type_index const& operation) -> std::vector<std::shared_ptr<WktParameterAttribute>>
	{
		return MathTransformGenerics::GetFilteredMetadata<WktParameterAttribute>(operation);
	}

	auto ExtensionManager::GetClassificationName(std::type_index const& operation) -> std::string
	{
		auto const mathTransformAttribute = MathTransformGenerics::GetFilteredMetadata<OpenGisMathTransformAttribute>(operation);
		if (mathTransformAttribute.empty())
			return {};

		return mathTransformAttribute[0]->ClassificationName;
	}

	auto ExtensionManager::CreateMathTransform(std::shared_ptr<IProjection> const& projectionParameters, bool inverse) -> std::shared_ptr<IMathTransform>
	{
		ensureIndexBuilt();

		auto const it = gNameIndex.find(toLower(projectionParameters->GetClassName()));
		if (it == gNameIndex.end())
			throw std::runtime_error("Object not found");
		auto const clase = it->second;

		vector<Parameter> parameters;
		parameters.resize(projectionParameters->GetNumParameters());
		for (auto i = 0; i < projectionParameters->GetNumParameters(); i++)
			parameters[i] = projectionParameters->GetParameter(i);

		auto const projection = std::make_shared<Projection>(projectionParameters->GetClassName(), parameters);
		return MathTransformGenerics::Create(clase, projection, inverse);
	}
}
