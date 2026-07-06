#include "pch.h"
#include "Geographic3DToGravityRelatedHeightEGM2008.h"
#include "GridFiles/GridEGM2008Rednap.h"
#include "GridFiles/GridEGM2008.h"
#include "GridFiles/GridGtx.h"
#include "GridFiles/GridGtg.h"
#include "GridFiles/GridAusGeoid.h"
#include "../../CrsContext.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	namespace
	{
		namespace fs = std::filesystem;
		namespace ge = GravityRelatedHeightEGM2008GridFiles;

		// Case-insensitive "does `s` end with `suffix`?".
		auto endsWithNoCase(std::string const& s, char const* suffix) -> bool
		{
			auto const n = std::string::traits_type::length(suffix);
			return s.size() >= n && 0 == compareNoCase(s.c_str() + s.size() - n, suffix);
		}

		// True if the whole token parses as a number of type T.
		template <typename T>
		auto parses(std::string const& t) -> bool
		{
			T value{};
			auto const [ptr, ec] = std::from_chars(t.data(), t.data() + t.size(), value);
			return ec == std::errc{} && ptr == t.data() + t.size();
		}

		auto splitOn(std::string const& line, std::string_view delims) -> std::vector<std::string>
		{
			std::vector<std::string> tokens;
			for (std::size_t i = 0, start = 0; i <= line.size(); ++i)
				if (i == line.size() || delims.find(line[i]) != std::string_view::npos)
				{
					if (i > start) tokens.emplace_back(line.substr(start, i - start));
					start = i + 1;
				}
			return tokens;
		}

		// ---- EGM2008 global geoid grid ----
		// Named "Und_min<res>x<res>_egm2008_isw=82_WGS84_TideFree", with an optional ".gz" download
		// suffix (the EPSG catalogue may or may not include it). Extract <res> (arc-minutes) from such
		// a name; std::nullopt if it is not one.
		auto egm2008ResolutionFromName(std::string const& fileName) -> std::optional<double>
		{
			constexpr char prefix[]{ "Und_min" };
			constexpr char suffix[]{ "_egm2008_isw=82_WGS84_TideFree" };
			constexpr auto prefixLen = sizeof(prefix) - 1;
			constexpr auto suffixLen = sizeof(suffix) - 1;

			auto core = fileName;
			if (endsWithNoCase(core, ".gz")) core.erase(core.size() - 3);

			if (core.size() < prefixLen + suffixLen) return std::nullopt;
			if (0 != compareNoCase(core.substr(0, prefixLen).c_str(), prefix) || !endsWithNoCase(core, suffix))
				return std::nullopt;

			auto const mid = core.substr(prefixLen, core.size() - prefixLen - suffixLen);  // "<res>x<res>"
			auto const x = mid.find('x');
			if (x == std::string::npos) return std::nullopt;

			double res{};
			auto const head = mid.substr(0, x);
			auto const [ptr, ec] = std::from_chars(head.data(), head.data() + head.size(), res);
			if (ec != std::errc{} || ptr != head.data() + head.size() || !(res > 0.0)) return std::nullopt;
			return res;
		}

		// Grid geometry from the byte size. The file is a FORTRAN unformatted sequential file: each of
		// the `rows` records holds `cols` little-endian floats bracketed by a 4-byte record-length
		// marker at each end, i.e. cols+2 floats per row (this is what the reader skips over). With
		// cols = 2*(rows-1) this gives size == rows*(cols+2)*4 == rows*(2*rows)*4 == 8*rows². Solve for
		// rows; std::nullopt if size is not such a grid.
		auto egm2008GeometryFromSize(std::uintmax_t size) -> std::optional<std::pair<int, int>>
		{
			if (size == 0 || size % 8 != 0) return std::nullopt;
			auto const rowsSquared = size / 8;             // size == 8 * rows²
			auto const rows = static_cast<std::uintmax_t>(std::llround(std::sqrt(static_cast<double>(rowsSquared))));
			if (rows < 2 || rows * rows != rowsSquared) return std::nullopt;
			auto const cols = 2 * (rows - 1);
			return std::pair{ static_cast<int>(rows), static_cast<int>(cols) };
		}

		// Geometry from the resolution in the name (fallback when the file is absent, so the reader can
		// still be built to report it missing): cols = 21600/res, rows = 10800/res + 1.
		auto egm2008GeometryFromResolution(double res) -> std::pair<int, int>
		{
			return { static_cast<int>(std::lround(10800.0 / res)) + 1, static_cast<int>(std::lround(21600.0 / res)) };
		}

		// The canonical uncompressed file the reader opens: the (EPSG) name without its ".gz" download
		// suffix. Matches the name the EPSG catalogue advertises, so a consumer that fetches the grid
		// stores it under exactly this name.
		auto egm2008CanonicalName(std::string const& fileName) -> std::string
		{
			auto core = fileName;
			if (endsWithNoCase(core, ".gz")) core.erase(core.size() - 3);
			return core;
		}

		// Candidate on-disk names to LOCATE the grid from its (EPSG) name: the name as given and, if it
		// carried a ".gz" download suffix, the uncompressed name without it.
		auto egm2008CandidateNames(std::string const& fileName) -> std::vector<std::string>
		{
			std::vector<std::string> names{ fileName };
			auto const canonical = egm2008CanonicalName(fileName);
			if (std::find(names.begin(), names.end(), canonical) == names.end()) names.push_back(canonical);
			return names;
		}

		// A REDNAP ASCII geoid file opens with a header line of 6 numbers: origin lat/lon, cell size
		// lat/lon (arc-minutes) and the vertical/horizontal cell counts.
		auto looksLikeRednap(fs::path const& path) -> bool
		{
			std::ifstream is{ path };
			std::string line;
			if (!std::getline(is, line)) return false;

			auto const tokens = splitOn(line, " \t\r\n,=");
			if (tokens.size() < 6) return false;
			for (int i = 0; i < 4; ++i) if (!parses<double>(tokens[i])) return false;
			return parses<long>(tokens[4]) && parses<long>(tokens[5]);
		}

		// A geoid-grid format. Recognise it by CONTENT (a present file) for the happy path, and by
		// NAME family (an absent file) so its reader can report the file missing precisely. Adding a
		// format means adding a factory to the registry below — CreateLoaderFromFileName never changes.
		struct GeoidGridFactory
		{
			virtual ~GeoidGridFactory() = default;
			virtual auto fromContent(std::string const& dataDir, std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> = 0;
			virtual auto fromName(std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> = 0;
		};

		struct RednapFactory final : GeoidGridFactory
		{
			auto fromContent(std::string const& dataDir, std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				std::error_code ec;
				fs::path const path{ dataDir + fileName };
				if (fs::exists(path, ec) && looksLikeRednap(path))
					return std::make_unique<ge::GridEGM2008Rednap>(fileName.c_str());
				return nullptr;
			}
			auto fromName(std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				if (0 == compareNoCase(fileName.c_str(), "EGM08_REDNAP.txt") ||
					0 == compareNoCase(fileName.c_str(), "EGM08_REDNAP_Canarias.txt"))
					return std::make_unique<ge::GridEGM2008Rednap>(fileName.c_str());  // ctor throws if absent
				return nullptr;
			}
		};

		struct Egm2008Factory final : GeoidGridFactory
		{
			auto fromContent(std::string const& dataDir, std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				for (auto const& name : egm2008CandidateNames(fileName))
				{
					std::error_code ec;
					fs::path const path{ dataDir + name };
					if (!fs::exists(path, ec)) continue;
					auto const size = fs::file_size(path, ec);
					if (ec) continue;
					if (auto const geom = egm2008GeometryFromSize(size))
						return std::make_unique<ge::GridEgm2008>(name, geom->first, geom->second);
				}
				return nullptr;
			}
			auto fromName(std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				auto const res = egm2008ResolutionFromName(fileName);
				if (!res) return nullptr;
				auto const [rows, cols] = egm2008GeometryFromResolution(*res);
				// Build the reader for the canonical file so it reports the precise missing path.
				return std::make_unique<ge::GridEgm2008>(egm2008CanonicalName(fileName), rows, cols);
			}
		};

		struct GtxFactory final : GeoidGridFactory
		{
			auto fromContent(std::string const& dataDir, std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				fs::path const path{ dataDir + fileName };
				if (ge::GridGtx::looksLikeGtx(path))
					return std::make_unique<ge::GridGtx>(fileName.c_str());
				return nullptr;
			}
			auto fromName(std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				if (endsWithNoCase(fileName, ".gtx"))
					return std::make_unique<ge::GridGtx>(fileName.c_str());  // ctor throws if absent
				return nullptr;
			}
		};

		struct GtgFactory final : GeoidGridFactory
		{
			auto fromContent(std::string const& dataDir, std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				fs::path const path{ dataDir + fileName };
				std::error_code ec;
				if (fs::exists(path, ec) && ge::GridGtg::looksLikeGtg(path))
					return std::make_unique<ge::GridGtg>(fileName.c_str());
				return nullptr;
			}
			auto fromName(std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				if (endsWithNoCase(fileName, ".tif") || endsWithNoCase(fileName, ".tiff"))
					return std::make_unique<ge::GridGtg>(fileName.c_str());  // ctor throws if absent
				return nullptr;
			}
		};

		struct AusGeoidFactory final : GeoidGridFactory
		{
			auto fromContent(std::string const& dataDir, std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				fs::path const path{ dataDir + fileName };
				std::error_code ec;
				if (fs::exists(path, ec) && ge::GridAusGeoid::looksLikeNtv2(path))
					return std::make_unique<ge::GridAusGeoid>(fileName.c_str());
				return nullptr;
			}
			auto fromName(std::string const& fileName) const -> std::unique_ptr<IGridInterpolation> override
			{
				if (endsWithNoCase(fileName, ".gsb"))
					return std::make_unique<ge::GridAusGeoid>(fileName.c_str());  // ctor throws if absent
				return nullptr;
			}
		};

		auto registry() -> std::vector<std::unique_ptr<GeoidGridFactory>> const&
		{
			static auto const factories = []
			{
				std::vector<std::unique_ptr<GeoidGridFactory>> v;
				v.push_back(std::make_unique<RednapFactory>());
				v.push_back(std::make_unique<GtxFactory>());
				v.push_back(std::make_unique<GtgFactory>());
				v.push_back(std::make_unique<AusGeoidFactory>());
				v.push_back(std::make_unique<Egm2008Factory>());
				return v;
			}();
			return factories;
		}
	}

	// Locates the geoid grid reader for the given (EPSG) model-file name. Two passes: first recognise
	// the grid by its CONTENT (so a present file is accepted regardless of its exact name), then fall
	// back to recognising the FAMILY by name so an absent file is routed to its reader, which reports
	// it missing (GridFileNotFoundException). Unknown names are rejected as unsupported.
	auto CreateLoaderFromFileName(std::string const& fileName) -> std::unique_ptr<IGridInterpolation>
	{
		auto const dataDir = GetDefaultContext()->dataDirectory;

		for (auto const& factory : registry())
			if (auto reader = factory->fromContent(dataDir, fileName))
				return reader;

		for (auto const& factory : registry())
			if (auto reader = factory->fromName(fileName))
				return reader;

		throw UnsupportedFormatException(std::format("File format is not supported: {}", fileName.c_str()));
	}

	Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008(std::string const& geoidModelFile)
		: _inverse{ false }
		, _methodName{ "Geographic3DToGravityRelatedHeightEGM2008" }
		, _grid{CreateLoaderFromFileName(geoidModelFile)}
	{
	}

	Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008(std::string const& geoidModelFile, bool inverse)
		: _inverse{inverse}
		, _methodName{ "Geographic3DToGravityRelatedHeightEGM2008" }
		, _grid{CreateLoaderFromFileName(geoidModelFile)}
	{
	}

	Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008(std::string const& methodName, std::string const& geoidModelFile, bool inverse)
		: _inverse{ inverse }
		, _methodName{ methodName }
		, _grid{CreateLoaderFromFileName(geoidModelFile)}
	{
	}

	Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: _inverse{inverse}
		, _methodName{ parameters->GetClassName() }
		, _grid{CreateLoaderFromFileName(get<std::string>(parameters->GetParameter("geoid_model_file").GetValue()))}
	{
	}

	Geographic3DToGravityRelatedHeightEGM2008::Geographic3DToGravityRelatedHeightEGM2008(std::shared_ptr<IProjection> const& _parameters)
		: _inverse{ false }
		, _methodName{ _parameters->GetClassName() }
		, _grid{CreateLoaderFromFileName(get<std::string>(_parameters->GetParameter("geoid_model_file").GetValue()))}
	{
	}

#pragma region IMathTransform members
	auto Geographic3DToGravityRelatedHeightEGM2008::GetWkt() const -> std::string
	{
		auto const file = std::filesystem::path(_grid->GetFilePath()).filename().string();
		if (_inverse)
			return std::format("INVERSE_MT[PARAM_MT[\"{}\", PARAMETER[\"geoid_model_file\",{}]]]", _methodName, file);

		return std::format("PARAM_MT[\"{}\", PARAMETER[\"geoid_model_file\",{}]]", _methodName, file);
	}

	auto Geographic3DToGravityRelatedHeightEGM2008::GetSourceDimension() const -> int
	{
		return 3;
	}

	auto Geographic3DToGravityRelatedHeightEGM2008::GetTargetDimension() const -> int
	{
		return 1;
	}

	auto Geographic3DToGravityRelatedHeightEGM2008::GetIsIdentity() const -> bool
	{
		return false;
	}

	auto Geographic3DToGravityRelatedHeightEGM2008::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		std::vector<double> result;
		result.resize(this->GetTargetDimension());

		auto const correction = _grid->ComputeUndulation({ point[0], point[1] });

		if (_inverse)
			result[0] = point[2] + correction;
		else
			result[0] = point[2] - correction;

		return result;
	}

	auto Geographic3DToGravityRelatedHeightEGM2008::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return std::shared_ptr<IMathTransform>(new Geographic3DToGravityRelatedHeightEGM2008(
			_methodName, std::filesystem::path(_grid->GetFilePath()).filename().string(), !_inverse));
	}
#pragma endregion
}
