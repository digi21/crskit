#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace CrsKit
{

struct Attribute
{
	virtual ~Attribute() = default;
};

template <typename BaseClass, typename... _AdditionalParameters>
class Factory final
{
	using FactoryMethod = std::function<std::shared_ptr<BaseClass>(_AdditionalParameters...)>;
	using MetadataContainer = std::vector<std::shared_ptr<Attribute>>;
	using CreateDelegate = std::function<bool(MetadataContainer const&)>;

	// Meyers singletons: lazy init -> no static initialization order fiasco
	// (static Registrators call Register in any link order).
	static auto _factories() -> std::unordered_map<std::type_index, FactoryMethod>&
	{
		static std::unordered_map<std::type_index, FactoryMethod> instance;
		return instance;
	}
	static auto _metadata() -> std::unordered_map<std::type_index, MetadataContainer>&
	{
		static std::unordered_map<std::type_index, MetadataContainer> instance;
		return instance;
	}

public:
	template <typename T>
		requires std::derived_from<T, BaseClass>
	static auto Register(std::vector<std::shared_ptr<Attribute>> metadata) -> void
	{
		_factories()[typeid(T)] = [](_AdditionalParameters&&... args) { return std::make_shared<T>(std::forward<_AdditionalParameters>(args)...); };
		_metadata()[typeid(T)] = std::move(metadata);
	}

	static auto Find(CreateDelegate const& delegate) -> std::type_index
	{
		auto& factories = _factories();
		auto const it = std::ranges::find_if(factories,
			[&](auto const& entry) { return delegate(entry.first, _metadata()[entry.first]); });
		if (it != factories.end())
			return it->first;

		throw std::runtime_error("Object not found");
	}

	template <typename T>
		requires std::derived_from<T, Attribute>
	static auto Find(std::function<bool(T const&)> const& delegate) -> std::type_index
	{
		auto& factories = _factories();
		auto const it = std::ranges::find_if(factories, [&](auto const& entry) {
			return std::ranges::any_of(_metadata()[entry.first], [&](auto const& metadataAttribute) {
				auto const* searchedAttribute = dynamic_cast<T const*>(metadataAttribute.get());
				return searchedAttribute && delegate(*searchedAttribute);
			});
		});
		if (it != factories.end())
			return it->first;

		throw std::runtime_error("Object not found");
	}

	static auto Create(std::type_index object, _AdditionalParameters&&... _Args) -> std::shared_ptr<BaseClass>
	{
		return _factories()[object](std::forward<_AdditionalParameters>(_Args)...);
	}

	template <typename T>
		requires std::derived_from<T, BaseClass>
	static auto Create(_AdditionalParameters&&... _Args) -> std::shared_ptr<BaseClass>
	{
		return _factories()[std::type_index{typeid(T)}](std::forward<_AdditionalParameters>(_Args)...);
	}

	static auto GetMetadata(std::type_index const& object) -> MetadataContainer
	{
		return _metadata()[object];
	}

	template <typename T>
		requires std::derived_from<T, BaseClass>
	static auto GetMetadata() -> MetadataContainer
	{
		return _metadata()[std::type_index{typeid(T)}];
	}


	template <typename Filter>
		requires std::derived_from<Filter, Attribute>
	static auto GetFilteredMetadata(std::type_index const& object) -> std::vector<std::shared_ptr<Filter>>
	{
		auto casted = _metadata()[object]
			| std::views::transform([](auto const& metadata) { return std::dynamic_pointer_cast<Filter>(metadata); })
			| std::views::filter([](auto const& include) { return include != nullptr; });

		std::vector<std::shared_ptr<Filter>> filtered;
		std::ranges::copy(casted, std::back_inserter(filtered));
		return filtered;
	}

	// Snapshot of all registered type keys, so callers can build their own O(1) indexes
	// over the metadata instead of scanning linearly.
	static auto RegisteredTypes() -> std::vector<std::type_index>
	{
		std::vector<std::type_index> keys;
		keys.reserve(_factories().size());
		for (auto const& entry : _factories())
			keys.push_back(entry.first);
		return keys;
	}
};

}
