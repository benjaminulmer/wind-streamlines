#define _USE_MATH_DEFINES
#include "SphCoord.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/intersect.hpp>

#include <algorithm>
#include <cmath>

// Zero all fields
SphCoord::SphCoord() : latitude(0.0), longitude(0.0) {}


// Construct using provided lat long and radius of 1. If radians is false values are treated as degrees and converted to radians
SphCoord::SphCoord(double latitude, double longitude, bool radians) {

	if (radians) {
		this->longitude = longitude;
		this->latitude = latitude;
	}
	else {
		this->longitude = longitude * M_PI / 180.0;
		this->latitude = latitude * M_PI / 180.0;
	}
}


// Constructs from a cartesian point. Assumes sphere is at origin
SphCoord::SphCoord(const glm::dvec3& point) {
	double radius = glm::length(point);
	latitude = asin(point.y / radius);
	longitude = atan2(point.x, point.z);
}


// Returns the arc length between this and another spherical point
//
// other - other spherical point to calculate arc length between
double SphCoord::arcLength(const SphCoord& other) const {

	glm::dvec3 cart1 = this->toCartesian(1.0);
	glm::dvec3 cart2 = other.toCartesian(1.0);

	return asin(glm::length(glm::cross(cart1, cart2)));
}


// Returns the arc length between this and a vector representing a spherical point
//
// other - other spherical point to calculate arc length between
double SphCoord::arcLength(const glm::dvec3& other) const {

	glm::dvec3 cart1 = this->toCartesian(1.0);
	glm::dvec3 cart2 = glm::normalize(other);

	return asin(glm::length(glm::cross(cart1, cart2)));
}


// Converts spherical coordinates to cartesian with a given spherical radius. Assumes sphere is at origin
//
// radius - radius of sphere
glm::dvec3 SphCoord::toCartesian(double radius) const {
	return glm::dvec3( sin(longitude) * cos(latitude), sin(latitude), cos(longitude) * cos(latitude) ) * radius;
}


// Returns latitude in degrees
double SphCoord::latitudeDeg() const {
	return latitude * 180.0 / M_PI;
}


// Returns longitude in degrees
double SphCoord::longitudeDeg() const {
	return longitude * 180.0 / M_PI;
}


// Returns if two provided great circle arcs intersect. If they do, the result is stored in intersection
//
// a0 - start of first arc
// a1 - end of first arc
// b0 - start of second arc
// b1 - end of second arc
// intersection - output for intersection point
bool SphCoord::greatCircleArc2Intersect(const SphCoord& a0, const SphCoord& a1, const SphCoord& b0, const SphCoord& b1, SphCoord& intersection) {

	// Calculate planes for the two arcs
	glm::dvec3 planeA = glm::normalize(glm::cross(a0.toCartesian(1.0), a1.toCartesian(1.0)));
	glm::dvec3 planeB = glm::normalize(glm::cross(b0.toCartesian(1.0), b1.toCartesian(1.0)));

	// If planes are parallel, treat as no intersection
	glm::bvec3 equal = glm::epsilonEqual(planeA, planeB, 0.000001);
	if (equal.x && equal.y && equal.z) {
		return false;
	}

	// Planes are not the same, get the line of intersection between them
	glm::dvec3 lineDir = glm::cross(planeA, planeB);

	// Find the candidate intersection points by intersecting the line with the sphere
	glm::dvec3 inter1, inter2, norm1, norm2;
	if (glm::intersectLineSphere(glm::dvec3(), lineDir, glm::dvec3(), 1.0, inter1, norm1, inter2, norm2)) {

		double arcA = a0.arcLength(a1);
		double arcB = b0.arcLength(b1);
		double eps = std::min(arcA, arcB) * 0.01;

		// Test if point 1 is on both arcs
		double distA0 = a0.arcLength(inter1);
		double distA1 = a1.arcLength(inter1);
		double distB0 = b0.arcLength(inter1);
		double distB1 = b1.arcLength(inter1);

		if (abs(arcA - distA0 - distA1) < eps && abs(arcB - distB0 - distB1) < eps) {
			intersection = SphCoord(inter1);
			return true;
		}

		// Test if point 2 is on both arcs
		distA0 = a0.arcLength(inter2);
		distA1 = a1.arcLength(inter2);
		distB0 = b0.arcLength(inter2);
		distB1 = b1.arcLength(inter2);

		if (abs(arcA - distA0 - distA1) < eps && abs(arcB - distB0 - distB1) < eps) {
			intersection = SphCoord(inter2);
			return true;
		}
		return false;
	}
	else {
		// Should never happen
		return false;
	}
}


// Returns if a great circle arc and line of latitude intersect. If they do, the result is stored in intersection
// 
// a0 - start of arc
// a1 - end of arc
// latRad - line of latitude in radians
// minLongRad - minimum longitude for latitude line in radians
// maxLongRad - maximum longitude for latitude line in radians
// intersection - output for intersection point
bool SphCoord::greatCircleArcLatIntersect(const SphCoord& a0, const SphCoord& a1, double latRad, double minLongRad, double maxLongRad, SphCoord& intersection) {

	glm::dvec3 planeA = glm::normalize(glm::cross(a0.toCartesian(1.0), a1.toCartesian(1.0)));
	glm::dvec3 planeLat = glm::dvec3(0.0, 1.0, 0.0);

	// If planes are parallel, treat as no intersection
	glm::bvec3 equal = glm::epsilonEqual(planeA, planeLat, 0.000001);
	if (equal.x && equal.y && equal.z) {
		return false;
	}

	// Planes are not the same, get the line of intersection between them
	glm::dvec3 lineDir = glm::cross(planeA, planeLat);
	glm::dvec3 linePoint;
	double y = sin(latRad);

	if (abs(planeA.x) > 0.0001) {
		double x = -(planeA.y / planeA.x) * y;
		linePoint = glm::dvec3(x, y, 0.0);
	}
	else {
		double z = -(planeA.y / planeA.z) * y;
		linePoint = glm::dvec3(0.0, y, z);
	}

	// Find the candidate intersection points by intersecting the line with the sphere
	glm::dvec3 inter1, inter2, norm1, norm2;
	if (glm::intersectLineSphere(linePoint, linePoint + lineDir, glm::dvec3(), 1.0, inter1, norm1, inter2, norm2)) {

		double arcA = a0.arcLength(a1);
		double eps = std::min(arcA, abs(maxLongRad - minLongRad)) * 0.01;

		// Test if point 1 is on both arcs
		double distA0 = a0.arcLength(inter1);
		double distA1 = a1.arcLength(inter1);
		SphCoord sph1(inter1);

		if (abs(arcA - distA0 - distA1) < eps &&
				((minLongRad < (sph1.longitude + eps) && sph1.longitude < (maxLongRad + eps)) ||
				(maxLongRad < (sph1.longitude + eps) && sph1.longitude < (minLongRad + eps)))) {

			intersection = sph1;
			return true;
		}

		// Test if point 2 is on both arcs
		distA0 = a0.arcLength(inter2);
		distA1 = a1.arcLength(inter2);
		SphCoord sph2(inter2);

		if (abs(arcA - distA0 - distA1) < eps &&
				((minLongRad < (sph2.longitude + eps) && sph2.longitude < (maxLongRad + eps)) ||
				(maxLongRad < (sph2.longitude + eps) && sph2.longitude < (minLongRad + eps)))) {

			intersection = sph2;
			return true;
		}
		return false;
	}
	else {
		return false;
	}
}