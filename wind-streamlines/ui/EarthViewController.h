#pragma once

class Camera;
class RenderEngine;

#include <glm/glm.hpp>


// Class for managing a Camera and RenderEngine for viewing a virtual Earth
class EarthViewController {

public:
	EarthViewController(Camera& camera, RenderEngine& renderEngine, double cameraDist);

	void updateRotation(int oldX, int newX, int oldY, int newY, unsigned int buttonMask);
	void updateCameraDist(int dir, int x, int y);

	void resetCameraTilt();

	void updateFromVertRot(double rad);
	void updateNorthRot(double rad);
	void updateLatRot(double rad);
	void updateLngRot(double rad);

private:
	Camera& camera;
	RenderEngine& renderEngine;

	double cameraDist;

	glm::dvec3 eye;
	glm::dvec3 centre;
	glm::dvec3 up;

	double latRot;
	double lngRot;
	double fromVertRot;
	double northRot;

	void newCameraVectors();
};
