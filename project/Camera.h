#pragma once

#include <glm/glm.hpp>


// Class for camera information in look at format
class Camera {

public:
	Camera(double initialDist);

	glm::dmat4 getLookAt() const;
	glm::dvec3 getPosition() const;
	glm::vec3 getPositionNoTilt() const;
	glm::dvec3 getUp() const;
	glm::dvec3 getLookDir() const;

	void setDist(double newDist);

	void updateFromVertRot(double rad);
	void updateNorthRot(double rad);
	void updateLatRot(double rad);
	void updateLngRot(double rad);

	void reset();

private:

	glm::dvec3 eye;
	glm::dvec3 up;
	glm::dvec3 centre;

	glm::dvec3 rotatedEye;
	glm::dvec3 rotatedUp;
	glm::dvec3 rotatedCentre;

	double latRot;
	double lngRot;
	double fromVertRot;
	double northRot;

	void rotateVectors();
};
