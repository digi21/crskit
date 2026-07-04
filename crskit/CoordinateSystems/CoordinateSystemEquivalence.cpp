#include "pch.h"

#include "VerticalDatum.h"
#include "LocalDatum.h"

// Structural equivalence of coordinate systems: two systems are equivalent when their mathematical
// definition matches (datum, prime meridian, projection + parameters, units), ignoring name,
// authority and axis order/orientation. This answers "are these the same CRS for transformation
// purposes?"; the factory then decides the axis handling (identity, swap, or the normal path).

namespace CrsKit::CoordinateSystems
{
	namespace
	{
		constexpr double kRelTol = 1e-9;

		auto closeEnough(double a, double b) -> bool
		{
			return std::fabs(a - b) <= kRelTol * std::max({ 1.0, std::fabs(a), std::fabs(b) });
		}

		auto sameLinearUnit(LinearUnit const& a, LinearUnit const& b) -> bool
		{
			return closeEnough(a.GetMetersPerUnit(), b.GetMetersPerUnit());
		}

		auto sameAngularUnit(AngularUnit const& a, AngularUnit const& b) -> bool
		{
			return closeEnough(a.GetRadiansPerUnit(), b.GetRadiansPerUnit());
		}

		auto samePrimeMeridian(PrimeMeridian const& a, PrimeMeridian const& b) -> bool
		{
			// Compare the longitude from Greenwich in radians, so different units do not matter.
			return closeEnough(a.GetLongitude() * a.GetAngularUnit().GetRadiansPerUnit(),
			                   b.GetLongitude() * b.GetAngularUnit().GetRadiansPerUnit());
		}

		auto sameParameter(Parameter const& a, Parameter const& b) -> bool
		{
			if (0 != compareNoCase(a.GetName().c_str(), b.GetName().c_str()))
				return false;

			auto const va = a.GetValue();
			auto const vb = b.GetValue();
			if (va.index() != vb.index())
				return false;
			if (1 == va.index())
				return closeEnough(std::get<double>(va), std::get<double>(vb));
			return std::get<std::string>(va) == std::get<std::string>(vb);
		}

		auto sameProjection(std::shared_ptr<IProjection> const& a, std::shared_ptr<IProjection> const& b) -> bool
		{
			if (!a || !b)
				return a == b;
			if (0 != compareNoCase(a->GetClassName().c_str(), b->GetClassName().c_str()))
				return false;

			auto const& pa = a->GetParameters();
			auto const& pb = b->GetParameters();
			if (pa.size() != pb.size())
				return false;
			for (std::size_t i = 0; i < pa.size(); ++i)
				if (!sameParameter(pa[i], pb[i]))
					return false;
			return true;
		}
	}

	namespace
	{
		// Awards `weight` iff the component matches. The per-type weights below sum to 100, so a full
		// match scores exactly 100 (making AreEquivalent := CompareCrs == 100) and a partial match scores
		// proportionally. `graded` folds in a recursive CompareCrs (0..100) scaled by its weight.
		constexpr auto award(bool matches, int weight) -> int { return matches ? weight : 0; }
		constexpr auto graded(int subScore, int weight) -> int { return subScore * weight / 100; }
	}

	auto CompareCrs(std::shared_ptr<ICoordinateSystem> const& a, std::shared_ptr<ICoordinateSystem> const& b) -> int
	{
		if (a == b)
			return 100;
		if (!a || !b)
			return 0;
		if (a->GetDimension() != b->GetDimension())
			return 0;

		if (auto const ga = std::dynamic_pointer_cast<GeographicCoordinateSystem>(a), gb = std::dynamic_pointer_cast<GeographicCoordinateSystem>(b); ga || gb)
		{
			if (!ga || !gb)
				return 0;
			return award(0 == ga->GetHorizontalDatum()->CompareTo(gb->GetHorizontalDatum()), 60)
			     + award(samePrimeMeridian(ga->GetPrimeMeridian(), gb->GetPrimeMeridian()), 20)
			     + award(sameAngularUnit(ga->GetAngularUnit(), gb->GetAngularUnit()), 20);
		}

		if (auto const pa = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(a), pb = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(b); pa || pb)
		{
			if (!pa || !pb)
				return 0;
			return graded(CompareCrs(pa->GetGeographicCoordinateSystem(), pb->GetGeographicCoordinateSystem()), 40)
			     + award(sameProjection(pa->GetProjection(), pb->GetProjection()), 40)
			     + award(sameLinearUnit(pa->GetLinearUnit(), pb->GetLinearUnit()), 20);
		}

		if (auto const ca = std::dynamic_pointer_cast<GeocentricCoordinateSystem>(a), cb = std::dynamic_pointer_cast<GeocentricCoordinateSystem>(b); ca || cb)
		{
			if (!ca || !cb)
				return 0;
			return award(0 == ca->GetHorizontalDatum()->CompareTo(cb->GetHorizontalDatum()), 60)
			     + award(samePrimeMeridian(ca->GetPrimeMeridian(), cb->GetPrimeMeridian()), 20)
			     + award(sameLinearUnit(ca->GetLinearUnit(), cb->GetLinearUnit()), 20);
		}

		if (auto const va = std::dynamic_pointer_cast<VerticalCoordinateSystem>(a), vb = std::dynamic_pointer_cast<VerticalCoordinateSystem>(b); va || vb)
		{
			if (!va || !vb)
				return 0;
			// CompareTo lives on the concrete VerticalDatum, not on the IVerticalDatum interface.
			auto const datum = std::dynamic_pointer_cast<VerticalDatum>(va->GetVerticalDatum());
			if (!datum)
				return 0;
			return award(0 == datum->CompareTo(vb->GetVerticalDatum()), 70)
			     + award(sameLinearUnit(va->GetVerticalUnit(), vb->GetVerticalUnit()), 30);
		}

		if (auto const la = std::dynamic_pointer_cast<LocalCoordinateSystem>(a), lb = std::dynamic_pointer_cast<LocalCoordinateSystem>(b); la || lb)
		{
			if (!la || !lb)
				return 0;
			auto const datum = std::dynamic_pointer_cast<LocalDatum>(la->GetLocalDatum());
			return datum && 0 == datum->CompareTo(lb->GetLocalDatum()) ? 100 : 0;
		}

		if (auto const oa = std::dynamic_pointer_cast<CompoundCoordinateSystem>(a), ob = std::dynamic_pointer_cast<CompoundCoordinateSystem>(b); oa || ob)
		{
			if (!oa || !ob)
				return 0;
			return graded(CompareCrs(oa->GetHeadCS(), ob->GetHeadCS()), 50)
			     + graded(CompareCrs(oa->GetTailCS(), ob->GetTailCS()), 50);
		}

		return 0;
	}

	auto AreEquivalent(std::shared_ptr<ICoordinateSystem> const& a, std::shared_ptr<ICoordinateSystem> const& b) -> bool
	{
		return CompareCrs(a, b) == 100;
	}
}
