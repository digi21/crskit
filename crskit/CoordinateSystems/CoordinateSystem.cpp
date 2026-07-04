#include "pch.h"
#include "BaseInfo.h"
#include "CoordinateSystem.h"
#include "CompoundCoordinateSystem.h"
#include "CoordinateSystemFactory.h"

namespace CrsKit::CoordinateSystems
{
	// IInfo::GetWkt() with no argument emits using the current Environment's default context; the
	// emission itself lives in each concrete CRS's GetWkt(CrsContext const&) override.
	auto CoordinateSystem::GetWkt() const -> std::string
	{
		return GetWkt(*GetDefaultContext());
	}

	// Convenience: emit picking just the output version, taking every other emission setting
	// (e.g. compound-local handling) from the default context.
	auto CoordinateSystem::GetWkt(WktVersion version) const -> std::string
	{
		auto context = *GetDefaultContext();
		context.wktVersion = version;
		return GetWkt(context);
	}

	CoordinateSystem::CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info)
		: BaseInfo{name, authority, authorityCode, alias, info}
	{
	}

	CoordinateSystem::CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units, Positioning::Envelope const& areaUso)
		: BaseInfo{name, authority, authorityCode, alias, info}
		, _axes{axis}
		, _units{units}
		, _areaUso{areaUso}
	{
	}

	CoordinateSystem::CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units)
		: BaseInfo{name, authority, authorityCode, alias, info}
		, _axes{axis}
		, _units{units}
	{
	}

	CoordinateSystem::CoordinateSystem(std::string const& name, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units)
		: BaseInfo{name}
		, _axes{axis}
		, _units{units}
	{
	}

	CoordinateSystem::CoordinateSystem(std::string const& name, std::vector<AxisInfo> const& axis, AnyUnit const& unit)
		: BaseInfo{name}
		, _axes{axis}
	{
		_units.resize(_axes.size());

		for (auto i = 0; i < _axes.size(); i++)
			_units[i] = unit;
	}

	CoordinateSystem::CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AnyUnit const& unit, AxisInfo const& axis1, AxisInfo const& axis2, AxisInfo const& axis3)
		: BaseInfo{name, authority, authorityCode, alias, info}
	{
		_axes.resize(3);
		_axes[0] = axis1;
		_axes[1] = axis2;
		_axes[2] = axis3;

		_units.resize(3);
		_units[0] = unit;
		_units[1] = unit;
		_units[2] = unit;
	}

	CoordinateSystem::CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AnyUnit const& unit, AxisInfo const& axis1, AxisInfo const& axis2)
		: BaseInfo{name, authority, authorityCode, alias, info}
	{
		_axes.resize(2);
		_axes[0] = axis1;
		_axes[1] = axis2;

		_units.resize(2);
		_units[0] = unit;
		_units[1] = unit;
	}

	CoordinateSystem::CoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AnyUnit const& unit, AxisInfo const& axis1)
		: BaseInfo{name, authority, authorityCode, alias, info}
	{
		_axes.resize(1);
		_axes[0] = axis1;

		_units.resize(1);
		_units[0] = unit;
	}

	auto CoordinateSystem::GetDimension() const -> int
	{
		return static_cast<int>(_axes.size());
	}

	auto CoordinateSystem::GetDefaultEnvelope() const -> Positioning::Envelope
	{
		return _areaUso;
	}

	auto CoordinateSystem::GetAxis(int dimension) const -> AxisInfo
	{
		return _axes[dimension];
	}

	auto CoordinateSystem::GetUnits(int dimension) const -> AnyUnit
	{
		return _units[dimension];
	}

	auto CoordinateSystem::ExtractHorizontalVertical() -> std::pair<std::shared_ptr<CoordinateSystem>, std::shared_ptr<CoordinateSystem>>
	{
		CoordinateSystemFactory factory;
		auto horizontal = factory.CreateFromWkt("LOCAL_CS[\"Local or unknown horizontal coordinate system\", LOCAL_DATUM[\"Local or unknown horizontal datuml\", 32767],UNIT[\"unknown\",1.0000000000],AXIS[\"X\", EAST],AXIS[\"Y\",NORTH]]");
		auto vertical = factory.CreateFromWkt("LOCAL_CS[\"Local or unknown vertical coordinate system\", LOCAL_DATUM[\"Local or unknown vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]");

		auto const compound = dynamic_cast<CompoundCoordinateSystem*>(this);
		if (nullptr == compound)
		{
			if (this->GetDimension() == 3)
			{
				horizontal = CoordinateSystemFactory::Create2DSystemFor3DSystem(shared_from_this());
				vertical = shared_from_this();
				return { horizontal, vertical };
			}

			if (nullptr != dynamic_cast<HorizontalCoordinateSystem*>(this))
				horizontal = shared_from_this();
			else if (nullptr != dynamic_cast<VerticalCoordinateSystem*>(this))
				vertical = shared_from_this();
			else if (nullptr != dynamic_cast<LocalCoordinateSystem*>(this))
				vertical = horizontal = shared_from_this();

			return { horizontal, vertical };
		}

		if (nullptr != dynamic_pointer_cast<HorizontalCoordinateSystem>(compound->GetHeadCS()) ||
			nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(compound->GetHeadCS()))
			horizontal = dynamic_pointer_cast<CoordinateSystem>(compound->GetHeadCS());

		if (nullptr != dynamic_pointer_cast<VerticalCoordinateSystem>(compound->GetTailCS()) ||
			nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(compound->GetTailCS()))
			vertical = dynamic_pointer_cast<CoordinateSystem>(compound->GetTailCS());

		return { horizontal, vertical };
	}
}
