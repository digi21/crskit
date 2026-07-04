#pragma once

#pragma warning(disable : 4471)

#include <cstring>
#include <cstdlib>
#include <cwchar>
#include <cmath>
#include <cassert>
#include <vector>
#include <span>
#include <format>
#include <memory>
#include <map>
#include <variant>
#include <optional>
#include <functional>
#include <sstream>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <iterator>
#include <fstream>
#include <mutex>
#include <filesystem>

#include "Factory.h"
#include "Parameter.h"
#include "OpenGisException.h"
#include "GridFileNotFoundException.h"
#include "OpenGisAttributes.h"

#include "Mathematics/Angles.h"
#include "Mathematics/Matrix.h"
#include "Mathematics/Point.h"
#include "Mathematics/Window2D.h"
#include "Mathematics/Interpolation.h"
#include "Mathematics/GridCell.h"
#include "Mathematics/GridInterpolation.h"

#include "CrsKit.h"

using MathTransformGenerics = CrsKit::Factory<CrsKit::CoordinateTransformations::IMathTransform, std::shared_ptr<CrsKit::CoordinateSystems::IProjection> const&, bool&>;
