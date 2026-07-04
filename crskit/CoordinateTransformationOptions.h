#pragma once

// Cross-cutting delegates and per-call options for building coordinate transformations.
// Factored out of the CrsKit.h umbrella so the headers that reference these aliases
// (IUtilities, the transformation factories) can be self-contained instead of depending on
// the umbrella's include order.

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "CoordinateTransformations/CoordinateOperation.h"
#include "CoordinateTransformations/IMathTransform.h"
#include "CoordinateSystems/CoordinateSystem.h"

namespace CrsKit
{
	using DelegateDetectedMultipleCoordinateOperations = std::function<int(std::string const&, std::string const&, std::vector<CrsKit::CoordinateTransformations::CoordinateOperation> const&)>;
	using DelegateCreateVerticalTransformation = std::function<std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform>(std::string const&, std::string const&)>;

	// Policy for when EXACTLY ONE of the two systems is local/unknown.
	enum class UnknownCrsPolicy
	{
		Reject,    // default: refuse known <-> unknown/local (safe)
		Identity   // place as-is (assumes the same frame and units). For unit scaling or
		           // custom placement, provide resolveTransform instead.
	};

	// Sign convention used to interpret the three rotations of a WKT TOWGS84[...] node when building a
	// datum-shift transform from it. The seven numbers are identical in both conventions except for the
	// sign of the rotations, so a WKT authored under the wrong convention is off by that sign.
	enum class Towgs84RotationConvention
	{
		PositionVector,   // default: EPSG 9606, as GDAL/PROJ interpret WKT 1 TOWGS84
		CoordinateFrame   // EPSG 9607, as ESRI authors TOWGS84 (rotations carry the opposite sign)
	};

	// Delegate to resolve the unknown/local case (e.g. the client shows a dialog).
	// Receives both systems; returns the transform to use, or nullptr to reject.
	using DelegateResolveTransform = std::function<std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform>(
		std::shared_ptr<CrsKit::CoordinateSystems::CoordinateSystem> const& source,
		std::shared_ptr<CrsKit::CoordinateSystems::CoordinateSystem> const& target)>;

	// Transform-creation options (extensible, per-call, no global state).
	struct CoordinateTransformationOptions
	{
		UnknownCrsPolicy unknownCrsPolicy{ UnknownCrsPolicy::Reject };
		// Provided by the caller to resolve a transform the library cannot build itself
		// (unknown/local CRS, or two known verticals with no EPSG operation). nullptr -> give up.
		DelegateResolveTransform resolveTransform{};
		DelegateDetectedMultipleCoordinateOperations selectOperation{};
		// How to read the rotations of a WKT TOWGS84 when no EPSG operation applies (Bursa-Wolf fallback).
		Towgs84RotationConvention towgs84RotationConvention{ Towgs84RotationConvention::PositionVector };
	};
}
