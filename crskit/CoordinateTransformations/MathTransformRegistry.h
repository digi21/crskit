#pragma once

namespace CrsKit
{
	// Registers all built-in math-transform algorithms (and their metadata: WKT classification
	// name(s), EPSG authority+operation code, parameters) into MathTransformGenerics.
	//
	// Called once from Initialize(). This replaces the previous macro-based self-registration
	// (BEGIN_REGISTER_TYPE + static Registrator), which depended on static-initialization order and
	// could be silently dropped by the linker if an algorithm object had no referenced symbols.
	void RegisterBuiltInMathTransforms();
}
