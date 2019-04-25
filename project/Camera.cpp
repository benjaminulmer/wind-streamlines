#define _USE_MATH_DEFINES
#define GLM_ENABLE_EXPERIMENTAL

#include "Camera.h"
#include "Conversions.h"

#include <glm/gtx/rotate_vector.hpp>

#include <algorithm>
#include <cmath>


// Create camera with intial distance from the surface of the Earth
//
// initialDist - distance camera is from surface
Camera::Camera(double initialDist) :
	latRot(0.0),
	lngRot(0.0),
	fromVertRot(0.0),
	northRot(0.0) {

	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M + initialDist);
	up = glm::dvec3(0.0, 1.0, 0.0);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M);
	reset();
}


// Rotates vectors to create final eye, up, and centre vector
void Camera::rotateVectors() {

	glm::dvec3 rightW(1.0, 0.0, 0.0);
	glm::dvec3 forwW(0.0, 0.0, 1.0);
	glm::dvec3 downW(0.0, -1.0, 0.0);

	// Tile and north rotation
	rotatedEye = glm::rotate((eye - centre), fromVertRot, rightW);
	rotatedEye = glm::rotate(rotatedEye, northRot, forwW);
	rotatedUp = glm::rotate(up, fromVertRot, rightW);
	rotatedUp = glm::rotate(rotatedUp, northRot, forwW);

	// Latitude and longitude rotation, i.e. panning
	rotatedEye = glm::rotate(rotatedEye, latRot, rightW);
	rotatedEye = glm::rotate(rotatedEye, lngRot, downW);
	rotatedUp = glm::rotate(rotatedUp, latRot, rightW);
	rotatedUp = glm::rotate(rotatedUp, lngRot, downW);
	rotatedCentre = glm::rotate(centre, latRot, rightW);
	rotatedCentre = glm::rotate(rotatedCentre, lngRot, downW);
}


// Returns the view matrix for the camera
//
// return - view matrix
glm::dmat4 Camera::getLookAt() const {
	return glm::lookAt(rotatedCentre + rotatedEye, rotatedCentre, rotatedUp);
}


// Returns position of the camera
// 
// return - position of camera
glm::dvec3 Camera::getPosition() const {
	return rotatedCentre + rotatedEye;
}


// Returns up vector of the camera
//
// return - up vector
glm::dvec3 Camera::getUp() const {
	return rotatedUp;
}


// Returns looking direction of camera
//
// return - looking direction vector
glm::dvec3 Camera::getLookDir() const {
	return glm::normalize(rotatedCentre - getPosition());
}


// Sets the distance camera is from the surface of the Earth
//
// newDist - new distance
void Camera::setDist(double newDist) {

	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M + newDist);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M);
	rotateVectors();
}


// Updates amount of rotation about the vector tangent to the Earth and going right in screen space
//
// rad - number of radians to add to current rotation
void Camera::updateFromVertRot(double rad) {
	fromVertRot += rad;
	fromVertRot = std::clamp(fromVertRot, 0.0, M_PI_2);
	rotateVectors();
}


// Updates amount of rotation about the normal at the look at position
//
// rad - number of radians to add to current rotation
void Camera::updateNorthRot(double rad) {
	northRot += rad;
	rotateVectors();
}


// Updates amount of latitude rotation
//
// rad - number of radians to add to current rotation
void Camera::updateLatRot(double rad) {
	latRot += rad;
	rotateVectors();
}


// Updates amount of longitude rotation
//
// rad - number of radians to add to current rotation
void Camera::updateLngRot(double rad) {
	lngRot += rad;
	rotateVectors();
}


// Reset tilt and north rotation of camera
void Camera::reset() {
	fromVertRot = 0.0;
	northRot = 0.0;
	rotateVectors();
}