#include "EarthViewController.h"

#include "Conversions.h"
#include "rendering/Camera.h"
#include "rendering/RenderEngine.h"

#include <glm/gtx/intersect.hpp>
#include <glm/gtx/rotate_vector.hpp>


EarthViewController::EarthViewController(Camera& camera, RenderEngine& renderEngine, double cameraDist) :
	camera(camera),
	renderEngine(renderEngine),
	cameraDist(cameraDist),
	eye(0.0, 0.0, RADIUS_EARTH_M + cameraDist),
	up(0.0, 1.0, 0.0),
	centre(0.0, 0.0, RADIUS_EARTH_M),
	latRot(0.0),
	lngRot(0.0),
	fromVertRot(0.0),
	northRot(0.0) {}


// Intersect ray shot at given pixel with the Earth and return intersection as lat long
//
// x - x pixel location
// y - y pixel location
// return - optional of (lat long) in rad
std::optional<glm::dvec2> EarthViewController::raySphereIntersectFromPixel(int x, int y) const {

	glm::dmat4 projView = renderEngine.getProjection() * camera.getLookAt();
	glm::dmat4 invProjView = glm::inverse(projView);

	std::pair<double, double> norm = renderEngine.pixelToNormDevice(x, y);

	glm::dvec4 world(norm.first, norm.second, -1.0, 1.0);

	world = invProjView * world;
	world /= world.w;

	glm::dvec3 rayO = camera.getEye();
	glm::dvec3 rayD = glm::normalize(glm::dvec3(world) - rayO);
	double sphereRad = RADIUS_EARTH_M;
	glm::dvec3 sphereO = glm::dvec3(0.0);

	glm::dvec3 iPos, iNorm;

	if (glm::intersectRaySphere(rayO, rayD, sphereO, sphereRad, iPos, iNorm)) {

		double lng = atan2(iPos.x, iPos.z);
		double lat = M_PI_2 - acos(iPos.y / sphereRad);

		return glm::dvec2(lat, lng);
	}
	else {
		return {};
	}
}


bool EarthViewController::mouseDown(SDL_MouseButtonEvent e) {
	return (raySphereIntersectFromPixel(e.x, e.y).has_value());
}


// Updates rotation/orientation of Earth model
//
// oldX - old x pixel location of mouse
// oldY - old y pixel location of mouse
// newX - new x pixel location of mouse
// newY - new y pixel location of mouse
// buttonMask - SDL button mask for which buttons are pressed
void EarthViewController::updateRotation(SDL_MouseMotionEvent e) {

	auto oldInt = raySphereIntersectFromPixel(e.x - e.xrel, e.y - e.yrel);
	auto newInt = raySphereIntersectFromPixel(e.x, e.y);

	if (oldInt && newInt) {
		updateLatRot(newInt->x - oldInt->x);
		updateLngRot(newInt->y - oldInt->y);
	}

}


void EarthViewController::updateHeadingAndTilt(SDL_MouseMotionEvent e) {

	std::pair<double, double> oldNorm = renderEngine.pixelToNormDevice(e.x - e.xrel, e.y - e.yrel);
	std::pair<double, double> newNorm = renderEngine.pixelToNormDevice(e.x, e.y);

	updateFromVertRot(newNorm.second - oldNorm.second);
	updateNorthRot(oldNorm.first - newNorm.first);
}


// Moves camera towards or away from Earth 
//
// dir - direction of change, possitive for closer and negative for farther
// x - x pixel location of mouse
// y - y pixel location of mouse
void EarthViewController::updateCameraDist(int dir, int x, int y) {

	auto oldInt = raySphereIntersectFromPixel(x, y);

	if (dir > 0) {
		cameraDist /= 1.2;
	}
	else if (dir < 0) {
		cameraDist *= 1.2;
	}
	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M + cameraDist);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M);
	newCameraVectors();
	renderEngine.updatePlanes(cameraDist);

	auto newInt = raySphereIntersectFromPixel(x, y);

	if (oldInt && newInt) {
		updateLatRot(newInt->x - oldInt->x);
		updateLngRot(newInt->y - oldInt->y);
	}
}


// Reset tilt and north rotation of camera
void EarthViewController::resetCameraTilt() {
	fromVertRot = 0.0;
	northRot = 0.0;
	newCameraVectors();
}


// Rotates vectors to create final eye, up, and centre vector
void EarthViewController::newCameraVectors() {

	glm::dvec3 rightW(1.0, 0.0, 0.0);
	glm::dvec3 forwW(0.0, 0.0, 1.0);
	glm::dvec3 downW(0.0, -1.0, 0.0);

	glm::dvec3 rotatedEye, rotatedUp, rotatedCentre;

	// Tilt and north rotation
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

	glm::dvec3 realEye = rotatedCentre + rotatedEye;
	camera.setVectors(realEye, rotatedCentre, rotatedUp);
}


// Updates amount of rotation about the vector tangent to the Earth and going right in screen space
//
// rad - number of radians to add to current rotation
void EarthViewController::updateFromVertRot(double rad) {
	fromVertRot += rad;
	fromVertRot = std::clamp(fromVertRot, 0.0, M_PI_2);
	newCameraVectors();
}


// Updates amount of rotation about the normal at the look at position
//
// rad - number of radians to add to current rotation
void EarthViewController::updateNorthRot(double rad) {
	northRot += rad;
	newCameraVectors();
}


// Updates amount of latitude rotation
//
// rad - number of radians to add to current rotation
void EarthViewController::updateLatRot(double rad) {
	latRot += rad;
	newCameraVectors();
}


// Updates amount of longitude rotation
//
// rad - number of radians to add to current rotation
void EarthViewController::updateLngRot(double rad) {
	lngRot += rad;
	newCameraVectors();
}

