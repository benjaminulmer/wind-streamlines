#define _USE_MATH_DEFINES
#include "Geometry.h"

#include <cmath>

// Geometric slerp between two vectors
glm::dvec3 Geometry::geomSlerp(const glm::dvec3& v1, const glm::dvec3& v2, double t) {

	double theta = acos(glm::dot(glm::normalize(v1), glm::normalize(v2)));

	glm::dvec3 term1 = (sin((1.f - t) * theta) / sin(theta)) * v1;
	glm::dvec3 term2 = (sin(t * theta) / sin(theta)) * v2;
	return term1 + term2;
}

// Creates a renderable for an arc defined by p1, p2, and centre points
void Geometry::createArcR(const glm::dvec3& p1, const glm::dvec3& p2, const glm::dvec3& centre, Renderable& r) {
	glm::dvec3 v1 = p1 - centre;
	glm::dvec3 v2 = p2 - centre;

	// Points on centre, cannot create arc (and will crash)
	if (glm::length(v1) < 0.0001 || glm::length(v2) < 0.0001) {
		return;
	}

	// Compute angle between vectors
	double dot = glm::dot(glm::normalize(v1), glm::normalize(v2));
	if (dot > 1.0) dot = 1.0;
	if (dot < -1.0) dot = -1.0;

	double theta = acos(dot);
	int angleDeg = (int) (theta * 180.0 / M_PI);

	// Points are very close on arc, just draw line between them
	if (angleDeg == 0) {
		r.verts.push_back(p1);
		r.verts.push_back(p2);
		r.colours.push_back(r.lineColour);
		r.colours.push_back(r.lineColour);
		return;
	}

	// #num line segments ~= angle of arc in degrees / 6
	angleDeg /= 6;
	angleDeg++;
	for (int i = 0; i <= angleDeg; i++) {
		double t = (double)i / angleDeg;
		
		glm::dvec3 result = geomSlerp(v1, v2, t);

		r.verts.push_back(centre + result);
		r.colours.push_back(r.lineColour);
		if (i != 0 && i != angleDeg) {
			r.verts.push_back(centre + result);
			r.colours.push_back(r.lineColour);
		}
	}
}

// Creates a renderable for a line defined by p1 and p2
void Geometry::createLineR(const glm::dvec3& p1, const glm::dvec3& p2, Renderable & r) {
	r.verts.push_back(p1);
	r.verts.push_back(p2);
	r.colours.push_back(r.lineColour);
	r.colours.push_back(r.lineColour);
}