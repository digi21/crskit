#include "pch.h"

using namespace std;

namespace CrsKit::Wkt
{
	WktDeserializer::WktDeserializer(std::string const& wktString)
	{
		std::vector<char> text(wktString.begin(), wktString.end());
		text.push_back('\0');

		size_t i = 0, j = 0;
		RellenaArbol(nullptr, i, j, text.data());
	}

	auto WktDeserializer::RellenaArbol(TokenWkt* node, size_t& i, size_t& j, char* text) -> void
	{
		while (true)
		{
			while (text[j] != 0 && text[j] != ']' && text[j] != '[' && text[j] != ',')
				++j;

			if (0 == text[j] || ']' == text[j])
			{
				if (i != j)
				{
					if (nullptr == node)
						throw WktParseException("Not a valid Wkt string");

					text[j] = 0;
					node->AddChild(std::make_unique<TokenWkt>(text + i));
				}

				j++;
				i = j;
				return;
			}

			if ('[' == text[j])
			{
				text[j] = 0;

				// Hemos encontrado un token.
				auto token = std::make_unique<TokenWkt>(text + i);

				TokenWkt* observer;
				if (nullptr == node)
				{
					root = std::move(token);
					observer = root.get();
					node = observer;
				}
				else
				{
					observer = node->AddChild(std::move(token));
				}

				j++;
				i = j;
				RellenaArbol(observer, i, j, text);
				continue;
			}

			// If we have just found "]," i and j will be identical. We must not add any node
			if (i == j)
			{
				j++;
				i = j;
				continue;
			}

			// Era un parameter
			assert(nullptr != node);

			text[j] = 0;
			node->AddChild(std::make_unique<TokenWkt>(text + i));
			j++;
			i = j;
		}
	}

	auto WktDeserializer::GetRootNode() const -> const TokenWkt&
	{
		return *root;
	}
}
