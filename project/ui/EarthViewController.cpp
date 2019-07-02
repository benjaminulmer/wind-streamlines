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


// Updates rotation/orientation of Earth model
//
// oldX - old x pixel location of mouse
// oldY - old y pixel location of mouse
// newX - new x pixel location of mouse
// newY - new y pixel location of mouse
// skew - true tilts the camera, otherwise rotates the Earth
void EarthViewController::updateRotation(int oldX, int newX, int oldY, int newY, bool skew) {

	glm::dmat4 projView = renderEngine.getProjection() * camera.getLookAt();
	glm::dmat4 invProjView = glm::inverse(projView);

	double oldXN = (2.0 * oldX) / (renderEngine.getWidth()) - 1.0;
	double oldYN = (2.0 * oldY) / (renderEngine.getHeight()) - 1.0;
	oldYN *= -1.0;

	double newXN = (2.0 * newX) / (renderEngine.getWidth()) - 1.0;
	double newYN = (2.0 * newY) / (renderEngine.getHeight()) - 1.0;
	newYN *= -1.0;

	glm::dvec4 worldOld(oldXN, oldYN, -1.0, 1.0);
	glm::dvec4 worldNew(newXN, newYN, -1.0, 1.0);

	worldOld = invProjView * worldOld;
	worldOld /= worldOld.w;

	worldNew = invProjView * worldNew;
	worldNew /= worldNew.w;

	glm::dvec3 rayO = camera.getPosition();
	glm::dvec3 rayDOld = glm::normalize(glm::dvec3(worldOld) - rayO);
	glm::dvec3 rayDNew = glm::normalize(glm::dvec3(worldNew) - rayO);
	double sphereRad = RADIUS_EARTH_M;
	glm::dvec3 sphereO = glm::dvec3(0.0);

	glm::dvec3 iPosOld, iPosNew, iNorm;

	if (glm::intersectRaySphere(rayO, rayDOld, sphereO, sphereRad, iPosOld, iNorm) &&
		glm::intersectRaySphere(rayO, rayDNew, sphereO, sphereRad, iPosNew, iNorm)) {

		double longOld = atan2(iPosOld.x, iPosOld.z);
		double latOld = M_PI_2 - acos(iPosOld.y / sphereRad);

		double longNew = atan2(iPosNew.x, iPosNew.z);
		double latNew = M_PI_2 - acos(iPosNew.y / sphereRad);

		if (skew) {
			updateFromVertRot(newYN - oldYN);
			updateNorthRot(oldXN - newXN);
		}
		else {
			updateLatRot(latNew - latOld);
			updateLngRot(longNew - longOld);
		}
	}
}


// Moves camera towards or away from Earth
//
// dir - direction of change, possitive for closer and negative for farther
void EarthViewController::updateCameraDist(int dir) {

	if (dir > 0) {
		cameraDist /= 1.2f;
	}
	else if (dir < 0) {
		cameraDist *= 1.2f;
	}
	eye = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M + cameraDist);
	centre = glm::dvec3(0.0, 0.0, RADIUS_EARTH_M);
	newCameraVectors();
	renderEngine.updatePlanes(cameraDist);
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

