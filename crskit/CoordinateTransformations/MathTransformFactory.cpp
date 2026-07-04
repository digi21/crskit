#include "pch.h"
#include "MathTransformFactory.h"
#include "ConcatenatedTransform.h"
#include "Passthrough.h"
#include "../CoordinateSystems/Projection.h"
#include "../ExtensionManager.h"
#include "Algorithms/Afine.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations::Algorithms;
using namespace std;

namespace CrsKit::CoordinateTransformations
{
#pragma region IMathTransformFactory members
	auto MathTransformFactory::CreateConcatenatedTransform(std::shared_ptr<IMathTransform> const& transform1, std::shared_ptr<IMathTransform> const& transform2) -> std::shared_ptr<IMathTransform>
	{
		return make_shared<ConcatenatedTransform>(transform1, transform2);
	}

	auto MathTransformFactory::CreateConcatenatedTransform(vector<shared_ptr<IMathTransform>> const& transformations) -> std::shared_ptr<IMathTransform>
	{
		return make_shared<ConcatenatedTransform>(transformations);
	}

	bool isOnlyDouble(const char* str)
	{
		char* endptr{};
		strtold(str, &endptr);

		if (*endptr != '\0' || endptr == str)
			return false;
		return true;
	}

	auto MathTransformFactory::CreateFromWkt(Wkt::TokenWkt const& node, bool inverse) -> std::shared_ptr<IMathTransform>
	{
		if (node.GetName() == "INVERSE_MT")
			return CreateFromWkt(node.GetChild(0), true);

		if (node.GetName() == "CONCAT_MT")
		{
			vector<shared_ptr<IMathTransform>> transformations;
			transformations.resize(node.GetChildCount());
			for (auto i = 0u; i < node.GetChildCount(); i++)
				transformations[i] = CreateFromWkt(node.GetChild(i), inverse);
			return make_shared<ConcatenatedTransform>(transformations);
		}

		if (node.GetName() == "PASSTHROUGH_MT")
		{
			return make_shared<PassThrough>(
				node.GetChild(0).GetInt(),
				CreateFromWkt(node.GetChild(1), false));
		}

		if (node.GetName() == "PARAM_MT")
		{
			auto const operationName = node.GetChild(0).GetString();

			vector<Parameter> parameters;

			for (auto i = 1u; i < node.GetChildCount(); i++)
			{
				auto const valueString = node.GetChild(i).GetChild(1).GetString();

				if(isOnlyDouble(valueString.c_str()))
				{
					parameters.push_back(Parameter(node.GetChild(i).GetChild(0).GetString(), std::stod(valueString)));
				}
				else
				{
					parameters.push_back(Parameter(node.GetChild(i).GetChild(0).GetString(), valueString));
				}
			}

			return CreateParameterizedTransform(operationName, parameters, inverse);
		}

		throw WktParseException("A parameterized transformation could not be created from the provided Wkt string.");
	}

	auto MathTransformFactory::CreateFromWkt(std::string const& wkt) -> std::shared_ptr<IMathTransform>
	{
		Wkt::WktDeserializer const deserializadorWkt(wkt);
		return CreateFromWkt(deserializadorWkt.GetRootNode(), false);
	}

	auto MathTransformFactory::CreateParameterizedTransform(std::string const& classification, vector<Parameter> const& parameters) -> std::shared_ptr<IMathTransform>
	{
		auto const projection = make_shared<Projection>(classification, parameters);
		return ExtensionManager::CreateMathTransform(projection, false);
	}

	auto MathTransformFactory::CreateParameterizedTransform(std::string const& classification, vector<Parameter> const& parameters, bool inverse) -> std::shared_ptr<IMathTransform>
	{
		auto const projection = make_shared<Projection>(classification, parameters);
		return ExtensionManager::CreateMathTransform(projection, inverse);
	}

	auto MathTransformFactory::CreatePassThroughTransform(int firstAffectedOrdinate, std::shared_ptr<IMathTransform> const& mathTransform) -> std::shared_ptr<IMathTransform>
	{
		return make_shared<PassThrough>(firstAffectedOrdinate, mathTransform);
	}
#pragma endregion
}
