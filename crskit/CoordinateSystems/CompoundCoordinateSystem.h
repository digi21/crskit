#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../TokenWkt.h"
#include "CoordinateSystem.h"
#include "ICompoundCoordinateSystem.h"

namespace CrsKit::CoordinateSystems
{
	class _modeCrsKit CompoundCoordinateSystem final : public CoordinateSystem, virtual public ICompoundCoordinateSystem
	{
	private:
		std::shared_ptr<CoordinateSystem> _head;
		std::shared_ptr<CoordinateSystem> _tail;

	public:
		CompoundCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::shared_ptr<CoordinateSystem> const& head, std::shared_ptr<CoordinateSystem> const& tail);
		CompoundCoordinateSystem(std::string const& name, std::shared_ptr<CoordinateSystem> const& head, std::shared_ptr<CoordinateSystem> const& tail);
		static auto FromWkt(Wkt::TokenWkt const& token) -> std::shared_ptr<CompoundCoordinateSystem>;

#pragma region CompoundCoordinateSystem members

	public:
		auto GetHeadCS() const -> std::shared_ptr<ICoordinateSystem> override;
		auto GetTailCS() const -> std::shared_ptr<ICoordinateSystem> override;
#pragma endregion
#pragma region CoordinateSystem members
		auto GetName() const -> std::string override;

		auto GetDimension() const -> int override;

		auto GetAxis(int dimension) const -> AxisInfo override;
		auto GetUnits(int dimension) const -> AnyUnit override;
#pragma region IBaseInfo members
		using CoordinateSystem::GetWkt;
		[[nodiscard]] auto GetWkt(CrsContext const& context) const -> std::string override;
#pragma endregion
	};
}
