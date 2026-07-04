#pragma once

#include "Concepts.h"

namespace CrsKit::Math
{
	///	<summary>
	///		This class implements interpolations.
	///	</summary>
	template <Arithmetic T>
	class Interpolations
	{
	public:
		///	<summary>
		///		The equation of the line is as follows:
		///
		///		 x -x0   y -y0                     x -x0
		///		 ----- = -----  => y = y0 + (y1-y0)-----
		///		 x1-x0   y1-y0                     x1-x0
		///
		///		From which we can deduce that:
		///						x -x0
		///		y = y0 + (y1-y0)-----
		///                     x1-x0
		///
		///		If we consider the X of this equation as the X coordinate of the line and
		///		the Y coordinates of this equation as the values of the line for those
		///		X values, the computed Y value will be the interpolation of values Y0 and Y1
		///		for the given x value.
		///	</summary>
		static auto InterpolateValue(T pt0x, T pt0y, T pt1x, T pt1y, T x) -> T
		{
			return pt0y + (pt1y - pt0y) * (x - pt0x) / (pt1x - pt0x);
		}

		///	<summary>
		///		A bilinear interpolation is an interpolation between four points with four
		///		values:
		///                 ab
		///		A ----------+----- B
		///					x
		///					|
		///		C ----------+----- D
		///					cd
		///
		///		In this implementation, we assume that the X coordinates of points A and C coincide
		///		(xmin) as well as those of points B and D (xmax). We also assume that the Y coordinate of the
		///		points A and B coincide (ymin) as well as those of points C and D (ymax).
		///
		///		If we have four values, A, B, C and D we can compute the value the point "x" will have
		///		first by interpolating the value of point "x" on line AB, then computing the value of
		///		point "x" on line CD and finally computing the value "x" on line AB-CD.
		/// </summary>
		static auto BilinearInterpolation(T a, T b, T c, T d, T xmin, T xmax, T ymin, T ymax, T x, T y) -> T
		{
			T valueAB = InterpolateValue(xmin, a, xmax, b, x);
			T valueCD = InterpolateValue(xmin, c, xmax, d, x);
			return InterpolateValue(ymin, valueAB, ymax, valueCD, y);
		}
	};
}
