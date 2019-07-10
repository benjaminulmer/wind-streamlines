#pragma once

#include "Conversions.h"
#include "rendering/Camera.h"
#include "rendering/Renderable.h"
#include "rendering/RenderEngine.h"
#include "rendering/Window.h"
#include "streamlines/SphericalVectorField.h"
#include "ui/EarthViewController.h"
#include "ui/InputHandler.h"
#include "ui/SubWindowManager.h"

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
	static constexpr double initialCameraDist = RADIUS_EARTH_M * 3.0;

	Window window;
	RenderEngine renderEngine;
	Camera camera;
	EarthViewController evc;
	SubWindowManager swm;
	InputHandler input;
	SeedingEngine* seeder;

	ColourRenderable sphereRender;
	ColourRenderable coastRender;

	SphericalVectorField field;

	void mainLoop();
};
