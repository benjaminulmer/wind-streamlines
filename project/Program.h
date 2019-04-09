#pragma once

#include <SDL2/SDL.h>
#undef main

#include "Camera.h"
#include "RenderEngine.h"
#include "SphericalVectorField.h"
#include "Streamline.h"


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

	ColourRenderable coastLines;
	std::vector<const Renderable*> objects;

	SphericalVectorField field;
	std::vector<Streamline> streamlines;

	double scale;
	double latRot;
	double longRot;

	void setupWindow();
	void integrateStreamlines2();
	void integrateStreamlines();
	void mainLoop();
};
