#pragma once

#include <iostream>

#include <SDL2/SDL.h>
#undef main

#include "Camera.h"
#include "RenderEngine.h"
#include "SphericalVectorField.h"


enum class RadialBound {
	MAX,
	MIN,
	BOTH
};

class Program {

public:
	Program();

	void start();

	void updateRotation(int oldX, int newX, int oldY, int newY, bool skew);
	void updateScale(int inc);

private:
	SDL_Window* window;
	int width, height;

	RenderEngine* renderEngine;
	Camera* camera;

	Renderable coastLines;
	std::vector<Renderable*> streamlines;
	std::vector<const Renderable*> objects;

	SphericalVectorField field;

	double scale;
	double latRot;
	double longRot;

	void setupWindow();
	void integrateStreamlines();
	void mainLoop();
};
