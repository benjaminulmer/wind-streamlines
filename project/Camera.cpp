#define _USE_MATH_DEFINES
#define GLM_ENABLE_EXPERIMENTAL

#include "Camera.h"
#include "Conversions.h"

#include <glm/gtx/rotate_vector.hpp>

#include <algorithm>
#include <cmath>


Camera::Camera() :
	zoomScale(1.3),
	rotScale(0.008),
	scale(1.0) {

	reset();
}


void Camera::updateVectors() {

	glm::dvec3 axis1 = glm::normalize(glm::cross(eye, glm::dvec3(0.0, 1.0, 0.0)));
	glm::dvec3 axis2 = glm::normalize(eye);

	rotatedEye = glm::rotate((eye - centre), -fromVerticalRad, axis1);
	rotatedEye = glm::rotate(rotatedEye, northRotationRad, axis2);
	rotatedUp = glm::rotate(up, -fromVerticalRad, axis1);
	rotatedUp = glm::rotate(rotatedUp, northRotationRad, axis2);
}


// Returns the view matrix for the camera
//
// return - view matrix
glm::dmat4 Camera::getLookAt() const {
	return glm::lookAt(centre + rotatedEye, centre, rotatedUp);
}


// Returns position of the camera
// 
// return - position of camera
glm::dvec3 Camera::getPosition() const {
	return centre + rotatedEye;
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
	return glm::normalize(centre - getPosition());
}


// Sets current Earth model scale and updates accordingly
//
// newScale - scale to set to
void Camera::setScale(double newScale) {
	scale = newScale;

	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_VIEW * scale + 30.0);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_VIEW * scale);
	updateVectors();
}


// Updates amount of rotation about the vector tangent to the Earth and going right in screen space
//
// rad - number of radians to add to current rotation
void Camera::updateFromVertical(double rad) {
	fromVerticalRad += rad;
	fromVerticalRad = std::clamp(fromVerticalRad, 0.0, M_PI_2);
	updateVectors();
}


// Updates amount of rotation about the normal at the look at position
//
// rad - number of radians to add to current rotation
void Camera::updateNorthRotation(double rad) {
	northRotationRad -= rad;
	updateVectors();
}


// Zooms camera in or out
//
// sign - in or out, possitive or negative
void Camera::updateZoom(int sign) {

	if (sign < 0) {
		eye.z = (eye.z - RADIUS_EARTH_VIEW) / zoomScale + RADIUS_EARTH_VIEW;
	}
	else {
		eye.z = (eye.z - RADIUS_EARTH_VIEW) * zoomScale + RADIUS_EARTH_VIEW;;
	}
	eye.z = std::max(eye.z, 4.0 * RADIUS_EARTH_VIEW);
}


// Reset camera to starting position
void Camera::reset() {
	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_VIEW * scale + 30.0);
	up = glm::dvec3(0.0, 1.0, 0.0);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_VIEW * scale);

	fromVerticalRad = 0;
	northRotationRad = 0;
	updateVectors();
}