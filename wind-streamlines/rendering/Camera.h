#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>


// Class for camera in look at format
class Camera {

public:
	Camera(double initialDist);

	void setVectors(glm::dvec3& eye, glm::dvec3& centre, glm::dvec3& up);

	glm::dmat4 getLookAt() const { return glm::lookAt(eye, centre, up); }
	glm::dvec3 getPosition() const { return eye; }
	glm::dvec3 getCentre() const { return centre; }
	glm::dvec3 getUp() const { return up; }
	glm::dvec3 getLookDir() const { return glm::normalize(centre - eye); }

private:
	glm::dvec3 eye;
	glm::dvec3 centre;
	glm::dvec3 up;
};
