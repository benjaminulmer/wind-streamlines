#pragma once
constexpr double RADIUS_EARTH_M = 6371008.0;
constexpr double RADIUS_EARTH_VIEW = 10.0;
constexpr double RADIAL_SCALE_FACTOR = 100.0;

inline double altToAbs(double alt) {
	return RADIUS_EARTH_M + alt * RADIAL_SCALE_FACTOR;
}