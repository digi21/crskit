#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "CrsKitExport.h"

namespace CrsKit
{
	// Caller-supplied equivalences that resolve a CRS or a coordinate operation the catalogue does not hold
	// (or that the caller wants replaced) as an official catalogue entry. Three tables:
	//
	//   CRS code -> CRS code          the source carries an authority ("EPSG" unless given); the target is EPSG.
	//   CRS name -> CRS code          the target is EPSG; names are compared after NormalizeName().
	//   operation code -> operation code   both EPSG.
	//
	// An alias is an explicit request of the caller, so it is applied BEFORE the catalogue is consulted and
	// under both parse policies: a CRS resolved through an alias is the official target CRS built entirely
	// from the catalogue (name, datum, unit, code), and GetWkt() emits that CRS. The text of a WKT that
	// matches an alias is discarded except for its axes (WKT1 GEOGCS/PROJCS with AXIS keep them, as under
	// WktParsePolicy::CatalogWins), because the stored coordinates follow them.
	//
	// Where aliases apply: every CRS created by code through the EPSG CoordinateSystemAuthorityFactory
	// (including the base CRS it looks up internally), every WKT1/WKT2 CRS node (GEOGCS, PROJCS, VERT_CS,
	// GEOCCS, GEOGCRS, PROJCRS, VERTCRS, GEODCRS; by AUTHORITY/ID, and by name only when the node carries no
	// AUTHORITY/ID, so a WKT with a code that has no alias keeps that code whatever its name) and the components
	// of a compound CRS; every operation code the EPSG CoordinateTransformationAuthorityFactory receives,
	// including the one returned by CoordinateTransformationOptions::selectOperation; and the CRS codes it
	// uses to look up operations between two CRS. A compound CRS code is not aliased: a compound is
	// resolved component by component, and loses its own AUTHORITY/ID when a component is replaced.
	//
	// Chains are rejected with std::invalid_argument (an alias target cannot be the source of another alias),
	// so resolving twice gives the same result as resolving once. Adding an existing source replaces its
	// target, whether the old or the new target is the source itself (code -> code). Copies share the
	// tables; Add* and Clear replace them, so a copy taken earlier is not changed.
	class _modeCrsKit CrsAliases final
	{
	public:
		// EPSG:code resolves as EPSG:targetCode.
		auto AddCrs(int code, int targetCode) -> void;
		// authority:code resolves as EPSG:targetCode. The authority is compared without case.
		auto AddCrs(std::string_view authority, int code, int targetCode) -> void;
		// A CRS named 'name' (after NormalizeName) whose WKT has no AUTHORITY/ID resolves as EPSG:targetCode.
		auto AddCrsName(std::string_view name, int targetCode) -> void;
		// The EPSG operation 'code' resolves as the EPSG operation targetCode.
		auto AddOperation(int code, int targetCode) -> void;

		auto Clear() -> void;
		[[nodiscard]] auto IsEmpty() const -> bool;

		[[nodiscard]] auto FindCrs(std::string_view authority, int code) const -> std::optional<int>;
		[[nodiscard]] auto FindCrsByName(std::string_view name) const -> std::optional<int>;
		[[nodiscard]] auto FindOperation(int code) const -> std::optional<int>;

		// The EPSG code a CRS code resolves to: the alias target, or 'code' itself when there is no alias.
		[[nodiscard]] auto ResolveCrs(int code) const -> int;
		// The EPSG code an operation code resolves to: the alias target, or 'code' itself.
		[[nodiscard]] auto ResolveOperation(int code) const -> int;

		// The form names are compared in: ASCII letters lowered, '_' and ASCII white space (space, tab, CR, LF,
		// VT, FF) taken as a space, runs of spaces collapsed to one, leading and trailing spaces removed. Bytes
		// >= 0x80 are kept as they are: UTF-8 accented letters must match exactly (no case folding, no accent
		// removal), and text in another encoding (e.g. Windows-1252) is not recognised as its UTF-8 equivalent.
		[[nodiscard]] static auto NormalizeName(std::string_view name) -> std::string;

	private:
		struct Tables
		{
			std::map<std::pair<std::string, int>, int> crsCodes{};
			std::map<std::string, int, std::less<>> crsNames{};
			std::map<int, int> operations{};
		};

		[[nodiscard]] auto IsCrsTarget(int code) const -> bool;
		auto ThrowIfCrsTargetIsSource(int targetCode) const -> void;

		std::shared_ptr<Tables const> _tables{};
	};
}
