#pragma once

// Transforms a point with a math transform and the result with the inverse of that math transform, and compares the
// returned point with the starting point.

#include <cmath>
#include <format>
#include <memory>
#include <numbers>
#include <string>
#include <typeinfo>
#include <vector>

#include <gtest/gtest.h>

#include "../crskit/CrsKit.h"

namespace TestCrsKit
{
	struct RoundTrip
	{
		std::vector<double> transformedPoint;
		std::vector<double> returnedPoint;
		// Type name and message of the std::exception that Transform or GetInverse threw; empty when no call threw.
		std::string exceptionDescription;
	};

	// Applies mathTransform to point and mathTransform->GetInverse() to the result. If a call throws a std::exception, the
	// round trip stops and RoundTrip::exceptionDescription receives the type name and the message of the exception.
	inline auto ApplyTransformAndInverse(std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform> const& mathTransform, std::vector<double> const& point) -> RoundTrip
	{
		RoundTrip roundTrip;
		try
		{
			roundTrip.transformedPoint = mathTransform->Transform(point);
			roundTrip.returnedPoint = mathTransform->GetInverse()->Transform(roundTrip.transformedPoint);
		}
		catch (std::exception const& exception)
		{
			roundTrip.exceptionDescription = std::format("{}: {}", typeid(exception).name(), exception.what());
		}
		return roundTrip;
	}

	// Tolerance of a round trip for one axis of a coordinate reference system (CRS), in the unit of that axis.
	struct AxisTolerance
	{
		double tolerance{};
		// Full turn in the unit of the axis for a longitude axis, so that a longitude and the same longitude plus 360 degrees
		// compare equal; 0 for any other axis.
		double fullTurnOfLongitude{};
	};

	// 1 mm for projected, geocentric and vertical coordinates and for ellipsoidal heights, and angleToleranceInDegrees for
	// latitudes and longitudes (1e-8 degrees is about 1.1 mm of latitude), each converted to the unit of the axis. The
	// ellipsoidal height of a geographic 3D CRS is taken in metres.
	inline auto CalculateAxisTolerances(std::shared_ptr<CrsKit::CoordinateSystems::ICoordinateSystem> const& coordinateSystem, double angleToleranceInDegrees = 1e-8) -> std::vector<AxisTolerance>
	{
		using namespace CrsKit::CoordinateSystems;
		constexpr auto lengthTolerance = 1e-3;

		if (auto const compound = std::dynamic_pointer_cast<CompoundCoordinateSystem>(coordinateSystem))
		{
			auto tolerances = CalculateAxisTolerances(compound->GetHeadCS(), angleToleranceInDegrees);
			for (auto const& tolerance : CalculateAxisTolerances(compound->GetTailCS(), angleToleranceInDegrees))
				tolerances.push_back(tolerance);
			return tolerances;
		}
		if (auto const geographic = std::dynamic_pointer_cast<GeographicCoordinateSystem>(coordinateSystem))
		{
			auto const degreesPerUnit = geographic->GetAngularUnit().GetRadiansPerUnit() * 180.0 / std::numbers::pi;
			std::vector<AxisTolerance> tolerances;
			for (auto axisIndex = 0; axisIndex < 2; ++axisIndex)
			{
				auto const orientation = geographic->GetAxis(axisIndex).GetOrientation();
				auto const isLongitude = AxisOrientationEnum::East == orientation || AxisOrientationEnum::West == orientation;
				tolerances.push_back({ angleToleranceInDegrees / degreesPerUnit, isLongitude ? 360.0 / degreesPerUnit : 0.0 });
			}
			if (3 == geographic->GetDimension())
				tolerances.push_back({ lengthTolerance });
			return tolerances;
		}

		auto metresPerUnit = 1.0;
		if (auto const projected = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(coordinateSystem))
			metresPerUnit = projected->GetLinearUnit().GetMetersPerUnit();
		else if (auto const geocentric = std::dynamic_pointer_cast<GeocentricCoordinateSystem>(coordinateSystem))
			metresPerUnit = geocentric->GetLinearUnit().GetMetersPerUnit();
		else if (auto const vertical = std::dynamic_pointer_cast<VerticalCoordinateSystem>(coordinateSystem))
			metresPerUnit = vertical->GetVerticalUnit().GetMetersPerUnit();
		return std::vector<AxisTolerance>(coordinateSystem->GetDimension(), AxisTolerance{ lengthTolerance / metresPerUnit });
	}

	// Returns returnedCoordinate - startingCoordinate; for a longitude axis, subtracts whole turns so that the absolute
	// difference is at most half a turn.
	inline auto CalculateDifference(double startingCoordinate, double returnedCoordinate, AxisTolerance const& axisTolerance) -> double
	{
		auto const difference = returnedCoordinate - startingCoordinate;
		if (0.0 == axisTolerance.fullTurnOfLongitude)
			return difference;
		return difference - axisTolerance.fullTurnOfLongitude * std::round(difference / axisTolerance.fullTurnOfLongitude);
	}

	// True when returnedPoint has as many coordinates as startingPoint and each difference is within the tolerance of its axis.
	inline auto IsWithinTolerances(std::vector<double> const& startingPoint, std::vector<double> const& returnedPoint, std::vector<AxisTolerance> const& axisTolerances) -> bool
	{
		if (returnedPoint.size() != startingPoint.size() || axisTolerances.size() < startingPoint.size())
			return false;
		for (std::size_t coordinateIndex = 0; coordinateIndex < startingPoint.size(); ++coordinateIndex)
			if (!(std::fabs(CalculateDifference(startingPoint[coordinateIndex], returnedPoint[coordinateIndex], axisTolerances[coordinateIndex])) <= axisTolerances[coordinateIndex].tolerance))
				return false;
		return true;
	}

	inline auto FormatPoint(std::vector<double> const& point) -> std::string
	{
		std::string text{ "(" };
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			text += std::format("{}{:.10g}", 0 == coordinateIndex ? "" : ", ", point[coordinateIndex]);
		return text + ")";
	}

	// Starting point, transformed point, returned point, difference of each coordinate and exception of a round trip.
	inline auto DescribeRoundTrip(std::vector<double> const& startingPoint, RoundTrip const& roundTrip, std::vector<AxisTolerance> const& axisTolerances) -> std::string
	{
		auto text = std::format("start {}", FormatPoint(startingPoint));
		if (!roundTrip.transformedPoint.empty())
			text += std::format(", transformed {}", FormatPoint(roundTrip.transformedPoint));
		if (!roundTrip.returnedPoint.empty())
		{
			text += std::format(", returned {}", FormatPoint(roundTrip.returnedPoint));
			if (roundTrip.returnedPoint.size() == startingPoint.size() && axisTolerances.size() >= startingPoint.size())
			{
				std::vector<double> differences;
				for (std::size_t coordinateIndex = 0; coordinateIndex < startingPoint.size(); ++coordinateIndex)
					differences.push_back(CalculateDifference(startingPoint[coordinateIndex], roundTrip.returnedPoint[coordinateIndex], axisTolerances[coordinateIndex]));
				text += std::format(", difference {}", FormatPoint(differences));
			}
		}
		if (!roundTrip.exceptionDescription.empty())
			text += std::format(", exception {}", roundTrip.exceptionDescription);
		return text;
	}

	// Expects that the round trip of point through mathTransform throws no exception and returns point within axisTolerances.
	inline auto ExpectRoundTrip(std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform> const& mathTransform, std::vector<double> const& point, std::vector<AxisTolerance> const& axisTolerances) -> void
	{
		auto const roundTrip = ApplyTransformAndInverse(mathTransform, point);
		EXPECT_TRUE(roundTrip.exceptionDescription.empty() && IsWithinTolerances(point, roundTrip.returnedPoint, axisTolerances)) << DescribeRoundTrip(point, roundTrip, axisTolerances);
	}
}
