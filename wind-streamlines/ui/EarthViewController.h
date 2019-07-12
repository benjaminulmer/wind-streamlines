#pragma once

class Camera;
class RenderEngine;

#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include <optional>


// Class for managing a Camera and RenderEngine for viewing a virtual Earth
class EarthViewController {

public:
	EarthViewController(Camera& camera, RenderEngine& renderEngine, double cameraDist);

	std::optional<glm::dvec2> raySphereIntersectFromPixel(int x, int y) const;

	bool mouseDown(SDL_MouseButtonEvent e);
	void updateRotation(SDL_MouseMotionEvent e);
	void updateHeadingAndTilt(SDL_MouseMotionEvent e);

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
