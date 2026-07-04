#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "../Positioning/Envelope.h"
#include "AxisInfo.h"
#include "CoordinateSystem.h"
#include "IHorizontalCoordinateSystem.h"
#include "IHorizontalDatum.h"
#include "Unit.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit HorizontalCoordinateSystem
	: public CoordinateSystem, virtual public IHorizontalCoordinateSystem
	{
	protected:
		std::shared_ptr<IHorizontalDatum> _horizontalDatum;

	public:
		HorizontalCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units, Positioning::Envelope const& areaUso, std::shared_ptr<IHorizontalDatum> const& datum);
		HorizontalCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units, std::shared_ptr<IHorizontalDatum> const& datum);
		HorizontalCoordinateSystem(std::string const& name, std::shared_ptr<IHorizontalDatum> const& datum, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units);
		HorizontalCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::shared_ptr<IHorizontalDatum> const& datum, AnyUnit const& unit, AxisInfo const& axis1, AxisInfo const& axis2);

#pragma region HorizontalCoordinateSystem members
		auto GetHorizontalDatum() const -> std::shared_ptr<IHorizontalDatum> override;
#pragma endregion
	};
}
