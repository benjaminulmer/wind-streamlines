#define _USE_MATH_DEFINES
#include "Camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <cmath>

#include "Conversions.h"

Camera::Camera() : zoomScale(1.3), rotScale(0.008) {
	reset();
}

// Returns view matrix for the camera
glm::dmat4 Camera::getLookAt() const {

	// Rotate eye along longitude
	glm::dvec3 eyeTemp = glm::rotateY(eye, -longitudeRotRad);

	// Find axis then rotate eye and up along latitude
	glm::dvec3 axis = glm::cross(eyeTemp, glm::dvec3(0.0, 1.0, 0.0));

	eyeTemp = glm::rotate(eyeTemp, latitudeRotRad, axis);
	glm::dvec3 upTemp = glm::rotate(up, latitudeRotRad, axis);

	return glm::lookAt(eyeTemp + translation, centre + translation, upTemp);
}

// Returns position of the camera
glm::dvec3 Camera::getPosition() const {

	glm::dvec3 eyeTemp = glm::rotateY(eye, -longitudeRotRad);
	eyeTemp = glm::rotate(eyeTemp, latitudeRotRad, glm::cross(eyeTemp, glm::dvec3(0.0, 1.0, 0.0)));

	return eyeTemp + translation;
}

// Returns up of the camera
glm::dvec3 Camera::getUp() const {

	// Rotate eye along longitude
	glm::dvec3 eyeTemp = glm::rotateY(eye, -longitudeRotRad);

	// Find axis then rotate eye and up along latitude
	glm::dvec3 axis = glm::cross(eyeTemp, glm::dvec3(0.0, 1.0, 0.0));

	eyeTemp = glm::rotate(eyeTemp, latitudeRotRad, axis);
	return glm::rotate(up, latitudeRotRad, axis);
}

// Returns looking direction of camera
glm::dvec3 Camera::getLookDir() const {
	return glm::normalize(centre - getPosition());
}

// Sets current model scale
void Camera::setScale(double scale) {
	curScale = scale;

	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_VIEW * scale + 30.0);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_VIEW * scale);
	latitudeRotRad = 0.0;
}

// Rotates camera along longitudinal axis (spherical coords)
void Camera::updateLongitudeRotation(double rad) {
	longitudeRotRad += rad;
}

// Rotates camera along latitudinal axis (spherical coords)
void Camera::updateLatitudeRotation(double rad) {
	latitudeRotRad -= rad;
}

// Zooms camera in or out (+1 or -1)
void Camera::updateZoom(int sign) {

	if (sign < 0) {
		eye.z = (eye.z - RADIUS_EARTH_VIEW) / zoomScale + RADIUS_EARTH_VIEW;
	}
	else {
		eye.z = (eye.z - RADIUS_EARTH_VIEW) * zoomScale + RADIUS_EARTH_VIEW;;
	}
	if (eye.z > 4.0 * RADIUS_EARTH_VIEW) eye.z = 4.0 * RADIUS_EARTH_VIEW;
}

// Translates camera along x and y of view plane
void Camera::translate(const glm::dvec3& planeTranslation) {

	glm::dvec3 pTrans(planeTranslation);

	// Scale translation based on zoom level
	double scale = (eye.z / 500.f);
	pTrans *= scale;

	// Get rotation axis
	glm::dvec3 eyeTemp = glm::rotateY(eye, -longitudeRotRad);
	glm::dvec3 axis = glm::cross(eyeTemp, glm::dvec3(0.0, 1.0, 0.0));

	// Convert screen space translation into world space translation
	glm::dvec3 rotatedTranslation = glm::rotateY(planeTranslation, -longitudeRotRad);
	rotatedTranslation = glm::rotate(rotatedTranslation, latitudeRotRad, axis);

	translation += rotatedTranslation;
}

// Reset camera to starting position
void Camera::reset() {
	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_VIEW + 30.0);
	up = glm::dvec3(0.0, 1.0, 0.0);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_VIEW);

	longitudeRotRad = 0;
	latitudeRotRad = 0;

	translation = glm::dvec3(0.0);
}