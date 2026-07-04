#pragma once

#include <algorithm>
#include <cstddef>
#include <span>
#include <vector>
#include <string>
#include <memory>

namespace CrsKit::CoordinateTransformations
{
	struct IMathTransform
	{
		virtual ~IMathTransform() = default;

		[[nodiscard]] virtual auto GetWkt() const -> std::string = 0;

		[[nodiscard]] virtual auto GetSourceDimension() const -> int = 0;

		[[nodiscard]] virtual auto GetTargetDimension() const -> int = 0;

		[[nodiscard]] virtual auto GetIsIdentity() const -> bool = 0;

		[[nodiscard]] virtual auto Transform(std::vector<double> const& point) const -> std::vector<double> = 0;

		[[nodiscard]] virtual auto GetInverse() -> std::shared_ptr<IMathTransform> = 0;

		///	<summary>
		///		Transforms N points in a single call. Input and output are flat buffers
		///		(row-major): GetSourceDimension() / GetTargetDimension() values per point.
		///		Intended for point clouds and the Python (NumPy) binding without crossing the
		///		boundary per point. The default implementation iterates Transform(); the
		///		implementations may override it to write in-place.
		///	</summary>
		[[nodiscard]] virtual auto TransformPoints(std::span<double const> sourcePoints) const -> std::vector<double>
		{
			auto const sourceDimension = static_cast<size_t>(GetSourceDimension());
			auto const targetDimension = static_cast<size_t>(GetTargetDimension());
			auto const count = sourceDimension ? sourcePoints.size() / sourceDimension : size_t{ 0 };

			std::vector<double> result(count * targetDimension);
			std::vector<double> point(sourceDimension);	// reused on each iteration (a single input allocation)
			for (size_t p = 0; p < count; ++p)
			{
				std::copy_n(sourcePoints.data() + p * sourceDimension, sourceDimension, point.begin());

				auto const transformed = Transform(point);
				std::copy(transformed.begin(), transformed.end(),
					result.begin() + static_cast<std::ptrdiff_t>(p * targetDimension));
			}
			return result;
		}
	};
}
