#pragma once

#include <glm/glm.hpp>


// Class for camera information in look at format
class Camera {

public:
	Camera(double scale);

	glm::dmat4 getLookAt() const;
	glm::dvec3 getPosition() const;
	glm::dvec3 getUp() const;
	glm::dvec3 getLookDir() const;

	void setScale(double scale);

	void updateFromVertical(double rad);
	void updateNorthRotation(double rad);

	void reset();

private:
	double scale;

	glm::dvec3 eye;
	glm::dvec3 up;
	glm::dvec3 centre;

	glm::dvec3 rotatedEye;
	glm::dvec3 rotatedUp;

	double fromVerticalRad;
	double northRotationRad;

	void updateVectors();
};
