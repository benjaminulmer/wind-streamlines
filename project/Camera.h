#pragma once

#include <glm/glm.hpp>

class Camera {

public:
	Camera();

	glm::dmat4 getLookAt() const;
	glm::dvec3 getPosition() const;
	glm::dvec3 getUp() const;
	glm::dvec3 getLookDir() const;

	void setScale(double scale);

	void updateLongitudeRotation(double rad);
	void updateLatitudeRotation(double rad);
	void updateZoom(int value);
	void translate(const glm::dvec3& planeTranslation);

	void reset();

private:
	const double zoomScale;
	const double rotScale;

	double curScale;

	glm::dvec3 eye;
	glm::dvec3 up;
	glm::dvec3 centre;

	double longitudeRotRad;
	double latitudeRotRad;

	glm::dvec3 translation;
};
