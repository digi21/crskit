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
		if (!root)
			throw WktParseException("Not a valid Wkt string");
	}

	auto WktDeserializer::RellenaArbol(TokenWkt* node, size_t& i, size_t& j, char* text) -> void
	{
		while (true)
		{
			while (text[j] != 0 && text[j] != ']' && text[j] != '[' && text[j] != ',')
			{
				// Commas and brackets inside a quoted name ("WGS 84 / UPS North (N,E)") are text. A doubled quote is a quote.
				if ('"' == text[j])
				{
					for (++j; text[j] != 0 && !('"' == text[j] && '"' != text[j + 1]); ++j)
						if ('"' == text[j])
							++j;
					if (0 == text[j])
						throw WktParseException("A quoted text of the WKT is not closed.");
				}
				++j;
			}

			if (0 == text[j] || ']' == text[j])
			{
				auto const endOfText = 0 == text[j];
				if (i != j)
				{
					if (nullptr == node)
						throw WktParseException("Not a valid Wkt string");

					text[j] = 0;
					node->AddChild(std::make_unique<TokenWkt>(text + i));
				}

				// At the terminator j stays put, so the enclosing levels stop too instead of reading past the buffer.
				if (!endOfText)
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
			if (nullptr == node)
				throw WktParseException("Not a valid Wkt string");

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
