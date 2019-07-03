#include "Camera.h"

#include "Conversions.h"


// Create camera with intial distance from the surface of the Earth
//
// initialDist - distance camera is from surface
Camera::Camera(double initialDist) {
	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M + initialDist);
	up = glm::dvec3(0.0, 1.0, 0.0);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M);
}


// Sets vectors to the provided ones
//
// eye - new eye position
// up - new up vector
// centre - new look at location
void Camera::setVectors(glm::dvec3& eye, glm::dvec3& centre, glm::dvec3& up) {
	this->eye = eye;
	this->up = up;
	this->centre = centre;
}