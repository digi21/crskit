#pragma once

#include <string>
#include <utility>

namespace CrsKit::CoordinateSystems
{
	// 7-parameter Bursa-Wolf (Helmert) parameters relating a geodetic datum to WGS 84:
	// three translations (metres), three rotations (arc-seconds) and a scale difference (ppm).
	// This is the content of a WKT 1 TOWGS84[dx,dy,dz,ex,ey,ez,ppm] node.
	class Wgs84ConversionInfo
	{
		double _dx{};
		double _dy{};
		double _dz{};
		double _ex{};
		double _ey{};
		double _ez{};
		double _ppm{};
		std::string _areaOfUse;

	public:
		Wgs84ConversionInfo() = default;
		Wgs84ConversionInfo(double dx, double dy, double dz, double ex, double ey, double ez, double ppm, std::string areaOfUse = {})
			: _dx{dx}, _dy{dy}, _dz{dz}, _ex{ex}, _ey{ey}, _ez{ez}, _ppm{ppm}, _areaOfUse{std::move(areaOfUse)}
		{
		}

		[[nodiscard]] auto GetDx() const -> double { return _dx; }
		[[nodiscard]] auto GetDy() const -> double { return _dy; }
		[[nodiscard]] auto GetDz() const -> double { return _dz; }
		[[nodiscard]] auto GetEx() const -> double { return _ex; }
		[[nodiscard]] auto GetEy() const -> double { return _ey; }
		[[nodiscard]] auto GetEz() const -> double { return _ez; }
		[[nodiscard]] auto GetPpm() const -> double { return _ppm; }
		[[nodiscard]] auto GetAreaOfUse() const -> std::string { return _areaOfUse; }
	};
}
