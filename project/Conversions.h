#define _USE_MATH_DEFINES
#pragma once

#include <Eigen/Dense>

#include <cmath>

// Collection of constants and formulas for converting between units and coordinate systems
const double RADIUS_EARTH_M = 6371008.0;
const double RADIAL_DIST_SCALE = 34.222;

// mbars to meters above surface of Earth
inline double mbarsToAlt(double mb) {
	return 0.3048 * 145366.45 * (1.0 - pow(mb / 1013.25, 0.190284));
}


// mbars to meters from centre of Earth
inline double mbarsToAbs(double mb) {
	return RADIUS_EARTH_M + mbarsToAlt(mb);
}


// meters above surface of Earth to mbars
inline double altToMBars(double m) {
	return 1013.25 * pow(1.0 - m / (0.3048 * 145366.45), 1.0 / 0.190284);
}


// meters from centre of Earth to mbars
inline double absToMBars(double m) {
	return altToMBars(m - RADIUS_EARTH_M);
}


// Spherical coordinates (lat, long, altitude) in rads and mbars to cartesian coordinates
inline Eigen::Vector3d sphToCart(const Eigen::Vector3d& v) {
	double rad = mbarsToAbs(v.z());
	return Eigen::Vector3d(sin(v.y()) * cos(v.x()), sin(v.x()), cos(v.y()) * cos(v.x())) * rad;
}


// Cartesian coordinates to spherical coordinates (lat, long, altitude) in rads and mbars
inline Eigen::Vector3d cartToSph(const Eigen::Vector3d& v) {
	double rad = v.norm();
	double lng = atan2(v.x(), v.z());
	if (lng < 0.0)
		lng += 2.0 * M_PI;
	return Eigen::Vector3d(asin(v.y() / rad), lng, absToMBars(rad));
}