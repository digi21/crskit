#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../CrsContext.h"
#include "../CrsKitExport.h"
#include "../Positioning/Envelope.h"
#include "AxisInfo.h"
#include "BaseInfo.h"
#include "ICoordinateSystem.h"
#include "Unit.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit CoordinateSystem : public BaseInfo, virtual public ICoordinateSystem, public std::enable_shared_from_this<CoordinateSystem>
	{
	protected:
		std::vector<AxisInfo> _axes;
		std::vector<AnyUnit> _units;
		Positioning::Envelope _areaUso;

	public:
		CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info);
		CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units, Positioning::Envelope const& areaUso);
		CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units);
		CoordinateSystem(std::string const& name, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units);
		CoordinateSystem(std::string const& name, std::vector<AxisInfo> const& axis, AnyUnit const& unit);
		CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AnyUnit const& unit, AxisInfo const& axis1, AxisInfo const& axis2);
		CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AnyUnit const& unit, AxisInfo const& axis1, AxisInfo const& axis2, AxisInfo const& axis3);
		CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AnyUnit const& unit, AxisInfo const& axis1);

		auto ExtractHorizontalVertical() -> std::pair<std::shared_ptr<CoordinateSystem>, std::shared_ptr<CoordinateSystem>>;

		// WKT emission. The CrsContext overload is the primary, virtual one: it drives emission entirely
		// from the passed context (output version + compound-local handling) with no global reads, so it is
		// pure and thread-safe. The two conveniences forward to it: the no-argument GetWkt() (from IInfo)
		// uses the default context, and the WktVersion overload uses the default context with just the
		// output version overridden. Each concrete CRS implements only the CrsContext overload.
		[[nodiscard]] auto GetWkt() const -> std::string override;
		[[nodiscard]] auto GetWkt(WktVersion version) const -> std::string;
		[[nodiscard]] virtual auto GetWkt(CrsContext const& context) const -> std::string = 0;

#pragma region CoordinateSystem members
		auto GetDimension() const -> int override;
		auto GetDefaultEnvelope() const -> Positioning::Envelope override;
		auto GetAxis(int dimension) const -> AxisInfo override;
		auto GetUnits(int dimension) const -> AnyUnit override;
		auto GetUnits() const -> std::vector<AnyUnit> const& override { return _units; }
#pragma endregion

#pragma region Implementation
		auto SetAxis(int axis, AxisInfo const& value) -> void { _axes[axis] = value; }
#pragma endregion
	};

	// True when the current parse policy (CrsContext::parsePolicy) snaps a WKT carrying an EPSG AUTHORITY to
	// the catalogue instead of building it from the pieces. Shared by the FromWkt readers.
	[[nodiscard]] auto CatalogWins() -> bool;

	// Graded structural similarity of two CRS, 0..100. Walks the same mathematical components as
	// AreEquivalent (datum, prime meridian, projection + parameters, units), but scores how many of them
	// match instead of returning a yes/no; 100 means fully equivalent, lower values a partial match.
	// Comparing CRS of different kind (or dimension) yields 0. Powers IdentifyEpsg's confidence ranking.
	[[nodiscard]] _modeCrsKit auto CompareCrs(std::shared_ptr<ICoordinateSystem> const& a, std::shared_ptr<ICoordinateSystem> const& b) -> int;

	// Structural equivalence: same mathematical definition (datum, prime meridian, projection +
	// parameters, units), ignoring name, authority and axis order/orientation. Answers
	// "are these the same CRS for transformation purposes?". Equivalent to CompareCrs(a, b) == 100.
	[[nodiscard]] _modeCrsKit auto AreEquivalent(std::shared_ptr<ICoordinateSystem> const& a, std::shared_ptr<ICoordinateSystem> const& b) -> bool;
}
