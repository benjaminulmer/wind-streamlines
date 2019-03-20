#pragma once

#include <glm/glm.hpp>

#include "Renderable.h"

class Geometry {

public:
	static glm::dvec3 geomSlerp(const glm::dvec3& v1, const glm::dvec3& v2, double t);
	
	static void createArcR(const glm::dvec3& p1, const glm::dvec3& p2, const glm::dvec3& centre, Renderable& r);
	static void createLineR(const glm::dvec3& p1, const glm::dvec3& p2, Renderable& r);
};

