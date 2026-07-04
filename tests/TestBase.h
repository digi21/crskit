#pragma once

// Common utilities for the GIGS test battery ported from the
// C# project (PruebasUnitariasImplementacionOpenGisCoordinateTransformations) to native
// C++ on GoogleTest.
//
// Equivalent to MapProjectionTestBase.cs from the original project.

#include <cmath>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "../crskit/CrsKit.h"

namespace TestCrsKit
{
	using CrsKit::CoordinateSystems::CoordinateSystem;
	using CrsKit::CoordinateSystems::ICoordinateSystem;
	using CrsKit::CoordinateTransformations::IMathTransform;

	using MathTransform = std::shared_ptr<IMathTransform>;

	enum class CardinalPoint { E, W, N, S };

	// Converts sexagesimal degrees (degrees, minutes, seconds + cardinal point)
	// to signed decimal degrees (N/E positive, S/W negative).
	inline double Sexa2DecimalDegrees(unsigned degrees, unsigned minutes, double seconds, CardinalPoint cardinalPoint)
	{
		if (cardinalPoint == CardinalPoint::E || cardinalPoint == CardinalPoint::N)
			return degrees + minutes / 60.0 + seconds / 3600.0;

		return -static_cast<double>(degrees) - minutes / 60.0 - seconds / 3600.0;
	}

	inline void ExpectFinite(double value)
	{
		EXPECT_FALSE(std::isnan(value));
		EXPECT_FALSE(std::isinf(value));
	}

	// Transforms (latitude, longitude) and checks that the result matches (x, y)
	// within the sigma tolerance.
	inline void TestDirectTransform(MathTransform const& t, double latitude, double longitude, double x, double y, double sigma)
	{
		auto const transformed = t->Transform({ latitude, longitude });

		ExpectFinite(transformed[0]);
		ExpectFinite(transformed[1]);
		EXPECT_NEAR(x, transformed[0], sigma);
		EXPECT_NEAR(y, transformed[1], sigma);
	}

	// Transforms (x, y) and checks that the result matches (latitude, longitude)
	// within the sigma tolerance.
	inline void TestInverseTransform(MathTransform const& t, double latitude, double longitude, double x, double y, double sigma)
	{
		auto const transformed = t->Transform({ x, y });

		ExpectFinite(transformed[0]);
		ExpectFinite(transformed[1]);
		EXPECT_NEAR(latitude, transformed[0], sigma);
		EXPECT_NEAR(longitude, transformed[1], sigma);
	}

	// Transforms a 3D triple (a, b, c) and checks that it matches (x, y, z).
	// sigmaAng applies to the first two components (usually latitude and
	// longitude in degrees) and sigmaLin to the third (height or linear component). For
	// purely linear outputs (e.g. geocentric X,Y,Z) just pass the same
	// tolerance in both arguments.
	inline void TestTransform3D(MathTransform const& t, double a, double b, double c, double x, double y, double z, double sigmaAng, double sigmaLin)
	{
		auto const transformed = t->Transform({ a, b, c });

		ExpectFinite(transformed[0]);
		ExpectFinite(transformed[1]);
		ExpectFinite(transformed[2]);
		EXPECT_NEAR(x, transformed[0], sigmaAng);
		EXPECT_NEAR(y, transformed[1], sigmaAng);
		EXPECT_NEAR(z, transformed[2], sigmaLin);
	}

	// Applies 1000 transformations alternating direct/inverse and checks that it
	// returns to the starting point (numerical stability test).
	inline void ExecuteIterations(MathTransform const& d, MathTransform const& i, double lat, double lon, double sigma = 1E-3)
	{
		std::vector<double> transformed{ lat, lon };
		bool sw = true;
		for (int _i = 0; _i < 1000; ++_i)
		{
			transformed = sw ? d->Transform(transformed) : i->Transform(transformed);

			ExpectFinite(transformed[0]);
			ExpectFinite(transformed[1]);

			sw = !sw;
		}
		EXPECT_NEAR(lat, transformed[0], sigma);
		EXPECT_NEAR(lon, transformed[1], sigma);
	}

	struct DirectInverse
	{
		MathTransform d;
		MathTransform i;
	};

	// Builds the direct transformation between two coordinate systems and its
	// inverse (equivalent to MapProjectionTestBase.ExecuteTests(source, target)).
	inline DirectInverse BuildDirectInverse(std::shared_ptr<ICoordinateSystem> const& source, std::shared_ptr<ICoordinateSystem> const& target)
	{
		auto const ct = CrsKit::GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(std::dynamic_pointer_cast<CoordinateSystem>(source), std::dynamic_pointer_cast<CoordinateSystem>(target));
		auto const d = ct->GetMathTransform();
		auto const i = d->GetInverse();
		return { d, i };
	}

	// Builds the transformation between two coordinate systems selecting a
	// a specific EPSG operation by code (equivalent to the `value => "NNNN"` delegate from C#,
	// which chooses one of the several candidate transformations between source and target).
	inline DirectInverse BuildDirectInverseWithOperation(std::shared_ptr<ICoordinateSystem> const& source, std::shared_ptr<ICoordinateSystem> const& target, int operationCode)
	{
		CrsKit::DelegateDetectedMultipleCoordinateOperations const selectOperation =
			[operationCode](std::string const&, std::string const&, std::vector<CrsKit::CoordinateTransformations::CoordinateOperation> const&) -> int
			{
				return operationCode;
			};

		auto const ct = CrsKit::GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(std::dynamic_pointer_cast<CoordinateSystem>(source), std::dynamic_pointer_cast<CoordinateSystem>(target), CrsKit::CoordinateTransformationOptions{ .selectOperation = selectOperation });
		auto const d = ct->GetMathTransform();
		auto const i = d->GetInverse();
		return { d, i };
	}

	// --- Helpers for vertical tests (1D transformations: height -> height) ---
	// Equivalent to VerticalCoordinateSystemTestBase from the C#.

	inline void TestDirectTransformVertical(MathTransform const& t, double original, double transformed, double sigma)
	{
		auto const r = t->Transform({ original });
		ExpectFinite(r[0]);
		EXPECT_NEAR(transformed, r[0], sigma);
	}

	// Builds the transformation between two vertical coordinate systems.
	inline MathTransform BuildVerticalTransform(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target)
	{
		auto const ct = CrsKit::GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target);
		return ct->GetMathTransform();
	}
}
