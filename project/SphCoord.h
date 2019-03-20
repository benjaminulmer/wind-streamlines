#pragma once

#include <glm/glm.hpp>

class SphCoord {

public:
	SphCoord();
	SphCoord(double latitude, double longitude, bool radians = true);
	SphCoord(const glm::dvec3& point);

	double arcLength(const SphCoord& other) const;
	double arcLength(const glm::dvec3& other) const;
	glm::dvec3 toCartesian(double radius) const;
	double latitudeDeg() const;
	double longitudeDeg() const;

	double latitude;  // in radians
	double longitude; // in radians

	// Static members
	static bool greatCircleArc2Intersect(const SphCoord& a0, const SphCoord& a1, const SphCoord& b0, const SphCoord& b1, SphCoord& intersection);
	static bool greatCircleArcLatIntersect(const SphCoord& a0, const SphCoord& a1, double latRad, double minLongRad, double maxLongRad, SphCoord& intersection);
};

