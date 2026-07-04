#include "pch.h"
#include "CoordinateTransformation.h"
#include "../CoordinateSystems/IHorizontalCoordinateSystem.h"

using namespace CrsKit::CoordinateTransformations;
using namespace CrsKit::CoordinateSystems;

namespace CrsKit::CoordinateTransformations
{
	CoordinateTransformation::CoordinateTransformation(std::string const& name, std::string const& comentarios, std::string const& areaOfUse, std::shared_ptr<CoordinateSystem> const& sourceCoordinateSystem, std::shared_ptr<CoordinateSystem> const& targetCoordinateSystem, std::shared_ptr<IMathTransform> const& transformation, int authorityCode)
		: _name{name}
		, _comentarios{comentarios}
		, _areaOfUse{areaOfUse}
		, _authorityCode{authorityCode}
		, _sourceCoordinateSystem{sourceCoordinateSystem}
		, _targetCoordinateSystem{targetCoordinateSystem}
		, _transformation{transformation}
	{
	}

#pragma region ICoordinateTransformation members
	auto CoordinateTransformation::GetName() const -> std::string
	{
		return _name;
	}

	auto CoordinateTransformation::GetAuthority() const -> std::string
	{
		return _authorityCode ? "EPSG" : "";
	}

	auto CoordinateTransformation::GetAuthorityCode() const -> std::string
	{
		return _authorityCode ? std::to_string(_authorityCode) : "";
	}

	auto CoordinateTransformation::GetRemarks() const -> std::string
	{
		return _comentarios;
	}

	auto CoordinateTransformation::GetAreaOfUse() const -> std::string
	{
		return _areaOfUse;
	}

	auto CoordinateTransformation::GetTransformType() const -> TransformType
	{
		// A change of geodetic datum is a Transformation; the same datum (projection, axis or unit
		// change) is a Conversion. When the datums cannot be compared (e.g. vertical or compound
		// systems) we report Other (unclassified).
		auto const source = std::dynamic_pointer_cast<IHorizontalCoordinateSystem>(_sourceCoordinateSystem);
		auto const target = std::dynamic_pointer_cast<IHorizontalCoordinateSystem>(_targetCoordinateSystem);
		if (!source || !target)
			return TransformType::Other;

		auto const sourceDatum = source->GetHorizontalDatum();
		auto const targetDatum = target->GetHorizontalDatum();
		if (!sourceDatum || !targetDatum)
			return TransformType::Other;

		return 0 == sourceDatum->CompareTo(targetDatum) ? TransformType::Conversion : TransformType::Transformation;
	}

	auto CoordinateTransformation::GetSourceCS() const -> std::shared_ptr<ICoordinateSystem>
	{
		return _sourceCoordinateSystem;
	}

	auto CoordinateTransformation::GetTargetCS() const -> std::shared_ptr<ICoordinateSystem>
	{
		return _targetCoordinateSystem;
	}

	auto CoordinateTransformation::GetMathTransform() const -> std::shared_ptr<IMathTransform>
	{
		return _transformation;
	}
#pragma endregion
}
