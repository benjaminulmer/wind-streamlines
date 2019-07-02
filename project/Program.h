#pragma once

#include "rendering/Renderable.h"
#include "streamlines/SphericalVectorField.h"
#include "streamlines/Streamline.h"

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
	void cleanup();

	void ImGui();

private:
	RenderEngine* renderEngine;
	Camera* camera;
	InputHandler* input;
	SeedingEngine* seeder;

	ColourRenderable sphereRender;
	ColourRenderable coastRender;

	SphericalVectorField field;

	double cameraDist;

	void mainLoop();
};
