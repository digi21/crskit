#include "pch.h"
#include "BaseInfo.h"
#include "CoordinateSystem.h"
#include "CompoundCoordinateSystem.h"
#include "CoordinateSystemAuthorityFactory.h"
#include "CoordinateSystemFactory.h"
#include "Wkt2.h"
#include "../EsriProjectionEngine.h"
#include "../CrsContext.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	CompoundCoordinateSystem::CompoundCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::shared_ptr<CoordinateSystem> const& head, std::shared_ptr<CoordinateSystem> const& tail)
		: CoordinateSystem{name, authority, authorityCode, alias, info}
		, _head{head}
		, _tail{tail}
	{
	}

	CompoundCoordinateSystem::CompoundCoordinateSystem(std::string const& name, std::shared_ptr<CoordinateSystem> const& head, std::shared_ptr<CoordinateSystem> const& tail)
		: CoordinateSystem{name, "", 0, "", ""}
		, _head{head}
		, _tail{tail}
	{
	}

	auto CompoundCoordinateSystem::FromWkt(Wkt::TokenWkt const& token) -> std::shared_ptr<CompoundCoordinateSystem>
	{
		// Preserve the compound CRS authority if the WKT declares one.
		auto const [authority, authorityCode] = Wkt::ReadAuthority(token);

		auto const head = CoordinateSystemFactory::CreateFrom(token.GetChild(1));
		auto const tail = CoordinateSystemFactory::CreateFrom(token.GetChild(2));

		return make_shared<CompoundCoordinateSystem>(
			head->GetName() + " + " + tail->GetName(),
			authority,
			authorityCode,
			"",
			"",
			head,
			tail);
	}


	auto CompoundCoordinateSystem::GetHeadCS() const -> std::shared_ptr<ICoordinateSystem>
	{
		return _head;
	}

	auto CompoundCoordinateSystem::GetTailCS() const -> std::shared_ptr<ICoordinateSystem>
	{
		return _tail;
	}

	auto CompoundCoordinateSystem::GetDimension() const -> int
	{
		return _head->GetDimension() + _tail->GetDimension();
	}

	auto CompoundCoordinateSystem::GetAxis(int dimension) const -> AxisInfo
	{
		if (dimension < 2)
			return _head->GetAxis(dimension);

		return _tail->GetAxis(dimension - 2);
	}

	auto CompoundCoordinateSystem::GetUnits(int dimension) const -> AnyUnit
	{
		if (dimension < 2)
			return _head->GetUnits(dimension);

		return _tail->GetUnits(dimension - 2);
	}

	auto CompoundCoordinateSystem::GetName() const -> std::string
	{
		if (GetDefaultContext()->createHorizontalWktIfVerticalLocal && nullptr != dynamic_pointer_cast<LocalCoordinateSystem>(_tail))
			return _head->GetName();

		return _head->GetName() + " + " + _tail->GetName();
	}

	auto CompoundCoordinateSystem::GetWkt(CrsContext const& context) const -> std::string
	{
		auto const version = context.wktVersion;

		if (IsWkt2(version))
			return Wkt::ToWkt2(*this, version);

		std::string text;

		if (context.createHorizontalWktIfVerticalLocal && nullptr != std::dynamic_pointer_cast<LocalCoordinateSystem>(_tail))
		{
			return _head->GetWkt(context);
		}

		if (!_authority.empty() && _authorityCode && !IsEsri(version))
			text = std::format("COMPD_CS[\"{}\",{},{},AUTHORITY[\"{}\",\"{}\"]]",
						_name.c_str(),
						_head->GetWkt(context).c_str(),
						_tail->GetWkt(context).c_str(),
						_authority.c_str(),
						_authorityCode);
		else
			text = std::format("COMPD_CS[\"{}\",{},{}]",
						_name.c_str(),
						_head->GetWkt(context).c_str(),
						_tail->GetWkt(context).c_str());

		if (IsEsri(version) &&
			nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(_head) &&
			nullptr == dynamic_pointer_cast<LocalCoordinateSystem>(_tail) &&
			!_head->GetAuthority().empty() && _head->GetAuthorityCode() != UnknownAuthorityCode &&
			!_tail->GetAuthority().empty() && _tail->GetAuthorityCode() != UnknownAuthorityCode)
			text = EsriProjectionEngine::CompoundWkt(_head->GetAuthorityCode(), _tail->GetAuthorityCode(), text);

		return text;
	}
}
