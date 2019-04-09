#define _USE_MATH_DEFINES
#pragma once

#include <Eigen/Dense>

#include <cmath>

const double RADIUS_EARTH_M = 6371008.0;
const double RADIUS_EARTH_VIEW = 10.0;

inline double mbarsToAlt(double mb) {
	return 0.3048 * 145366.45 * (1.0 - pow(mb / 1013.25, 0.190284));
}

inline double mbarsToAbs(double mb) {
	return RADIUS_EARTH_M + mbarsToAlt(mb);
}

inline double altToMBars(double m) {
	return 1013.25 * pow(1.0 - m / (0.3048 * 145366.45), 1.0 / 0.190284);
}

inline double absToMBars(double m) {
	return altToMBars(m - RADIUS_EARTH_M);
}

inline Eigen::Vector3d sphToCart(const Eigen::Vector3d& v) {
	double rad = mbarsToAbs(v.z());
	return Eigen::Vector3d(sin(v.y()) * cos(v.x()), sin(v.x()), cos(v.y()) * cos(v.x())) * rad;
}

inline Eigen::Vector3d cartToSph(const Eigen::Vector3d& v) {
	double rad = v.norm();
	double lng = atan2(v.x(), v.z());
	if (lng < 0.0) lng += 2.0 * M_PI;
	return Eigen::Vector3d(asin(v.y() / rad), lng, absToMBars(rad));
}


#include <vector>
#include "Streamline.h"

inline bool pointValid(Eigen::Vector3d point, const std::vector<Streamline>& streamlines, double sepDist) {

	for (const Streamline& s : streamlines) {
		for (const Eigen::Vector3d p : s.getPoints()) {

			Eigen::Vector3d pointC = sphToCart(point);
			Eigen::Vector3d pC = sphToCart(p);

			if ((pointC - pC).squaredNorm() < sepDist * sepDist) {
				return false;
			}
		}
	}
	return true;
}