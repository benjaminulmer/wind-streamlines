#pragma once

#include "rendering/Renderable.h"
#include "rendering/Window.h"
#include "streamlines/SphericalVectorField.h"
#include "streamlines/Streamline.h"

class Camera;
class EarthViewController;
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
	void cleanup();

	void ImGui();

private:
	Window window;
	RenderEngine* renderEngine;
	Camera* camera;
	EarthViewController* evc;
	InputHandler* input;
	SeedingEngine* seeder;

	ColourRenderable sphereRender;
	ColourRenderable coastRender;

	SphericalVectorField field;

	void mainLoop();
};
