#include "pch.h"
using namespace CrsKit::Math;
#include "../../CoordinateSystems/Projection.h"
#include "Afine.h"
#include "MolodenskyBadekas.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	namespace
	{
		// Affine 4x4 matrix of Molodensky-Badekas (Coordinate Frame convention).
		// M = (1 + ds*1e-6) * R_CF, with R_CF the frame rotation matrix (the
		// same signs as CreateCoordinateFrameRotationParameters). The column
		// independent term is c = t + (I - M)*P, with P the evaluation point.
		// Result: Xt = M*Xs + c. Rotations are received in degrees (just like
		// the rest of the library's transformations, via DEGREES_TO_RADIANS).
		auto BuildMatrix(double dx, double dy, double dz,
			double rx, double ry, double rz, double ds,
			double px, double py, double pz) -> Math::Matrix
		{
			rx = DEGREES_TO_RADIANS(rx);
			ry = DEGREES_TO_RADIANS(ry);
			rz = DEGREES_TO_RADIANS(rz);
			auto const m = 1.0 + ds * 1E-6;

			double const M[3][3] = {
				{       m,  m * rz, -m * ry },
				{ -m * rz,       m,  m * rx },
				{  m * ry, -m * rx,       m },
			};
			double const P[3] = { px, py, pz };
			double const t[3] = { dx, dy, dz };

			double c[3];
			for (int r = 0; r < 3; ++r)
			{
				c[r] = t[r];
				for (int k = 0; k < 3; ++k)
					c[r] += ((r == k ? 1.0 : 0.0) - M[r][k]) * P[k];
			}

			Math::Matrix mat{ 4, 4 };
			for (int r = 0; r < 3; ++r)
			{
				for (int k = 0; k < 3; ++k)
					mat[r][k] = M[r][k];
				mat[r][3] = c[r];
			}
			mat[3][0] = 0.0;
			mat[3][1] = 0.0;
			mat[3][2] = 0.0;
			mat[3][3] = 1.0;
			return mat;
		}

		auto BuildFromParameters(std::shared_ptr<IProjection> const& p, bool inverse) -> Math::Matrix
		{
			auto const mat = BuildMatrix(
				get<double>(p->GetParameter("x_axis_translation").GetValue()),
				get<double>(p->GetParameter("y_axis_translation").GetValue()),
				get<double>(p->GetParameter("z_axis_translation").GetValue()),
				get<double>(p->GetParameter("x_axis_rotation").GetValue()),
				get<double>(p->GetParameter("y_axis_rotation").GetValue()),
				get<double>(p->GetParameter("z_axis_rotation").GetValue()),
				get<double>(p->GetParameter("scale_difference").GetValue()),
				get<double>(p->GetParameter("ordinate_1_of_evaluation_point").GetValue()),
				get<double>(p->GetParameter("ordinate_2_of_evaluation_point").GetValue()),
				get<double>(p->GetParameter("ordinate_3_of_evaluation_point").GetValue()));
			return inverse ? mat.ComputeInverse() : mat;
		}
	}

	MolodenskyBadekas::MolodenskyBadekas(double dx, double dy, double dz, double rx, double ry, double rz, double ds, double px, double py, double pz)
		: Affine{4, 4, BuildMatrix(dx, dy, dz, rx, ry, rz, ds, px, py, pz), false}
	{
	}

	MolodenskyBadekas::MolodenskyBadekas(std::shared_ptr<IProjection> const& parameters)
		: Affine{4, 4, BuildFromParameters(parameters, false), false}
	{
	}

	MolodenskyBadekas::MolodenskyBadekas(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: Affine{4, 4, BuildFromParameters(parameters, inverse), false}
	{
	}
}
