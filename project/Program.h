#pragma once

#include "Renderable.h"
#include "SphericalVectorField.h"
#include "Streamline.h"

class Camera;
class InputHandler;
class RenderEngine;
class SeedingEngine;

#include <imgui.h>
#include <SDL2/SDL.h>
#undef main

#include <vector>
#include <mutex>


// Main class for running the program
// TODO god class, split up functionality
class Program {

public:
	Program();

	void start();

	void updateRotation(int oldX, int newX, int oldY, int newY, bool skew);
	void updateCameraDist(int dir);
	void setWindowSize(int newWidth, int newHeight) {
		width = newWidth;
		height = newHeight;
	}
	void cleanup();

	void ImGui();

private:
	SDL_Window* window;
	SDL_GLContext context;
	int width, height;

	ImGuiIO* io;

	RenderEngine* renderEngine;
	Camera* camera;
	InputHandler* input;
	SeedingEngine* seeder;

	ColourRenderable coastRender;
	//std::vector<const Renderable*> objects;

	SphericalVectorField field;
	std::vector<Streamline> streamlines;

	double cameraDist;

	void setupWindow();
	void mainLoop();
};
