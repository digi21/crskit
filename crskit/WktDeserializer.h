#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "TokenWkt.h"

namespace CrsKit::Wkt
{
	class WktDeserializer
	{
		std::unique_ptr<TokenWkt> root;

	public:
		explicit WktDeserializer(std::string const& wktString) noexcept(false);

		auto GetRootNode() const -> const TokenWkt&;

	private:
		// node: observer to the current parent (ownership lives in the parent / in 'root').
		auto RellenaArbol(TokenWkt* node, size_t& i, size_t& j, char* text) -> void;
	};
}
