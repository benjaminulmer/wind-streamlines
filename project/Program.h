#pragma once

#include <SDL2/SDL.h>
#undef main

#include "Camera.h"
#include "RenderEngine.h"
#include "SphericalVectorField.h"
#include "Streamline.h"

#include <mutex>

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

	ColourRenderable coastRender;
	StreamlineRenderable streamlineRender;
	std::vector<const Renderable*> objects;

	SphericalVectorField field;
	std::vector<Streamline> streamlines;

	bool reIntegrate;

	std::mutex mtx;
	int numNewLines;

	double scale;
	double latRot;
	double longRot;

	void setupWindow();
	void mainLoop();

	void integrateStreamlines();
};
