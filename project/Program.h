#pragma once

#include <imgui.h>
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
	void setWindowSize(int newWidth, int newHeight) {
		width = newWidth;
		height = newHeight;
	}
	void cleanup();

private:
	SDL_Window* window;
	SDL_GLContext context;
	int width, height;

	ImGuiIO* io;

	RenderEngine* renderEngine;
	Camera* camera;

	ColourRenderable coastRender;
	StreamlineRenderable streamlineRender;
	std::vector<const Renderable*> objects;

	SphericalVectorField field;
	std::vector<Streamline> streamlines;

	std::mutex mtx;
	volatile int numNewLines;

	bool stop = false;
	double scale;
	double latRot;
	double longRot;

	void setupWindow();
	void mainLoop();

	void integrateStreamlines();
};
