#include "Program.h"

#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include "ContentReadWrite.h"
#include "Conversions.h"
#include "Frustum.h"
#include "VoxelGrid.h"
#include "rendering/Camera.h"
#include "rendering/RenderEngine.h"
#include "streamlines/SeedingEngine.h"
#include "ui/EarthViewController.h"
#include "ui/SubWindow.h"
#include "ui/InputHandler.h"

#include <GL/glew.h>
#include <netcdf>
#include <SDL2/SDL_opengl.h>

#include <chrono>
#include <iostream>


// Dear ImGUI window. Show misc statuses
void Program::ImGui() {
	if (ImGui::CollapsingHeader("Status")) {
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Camera dist: %.0f", camera->getDist());
		ImGui::Text("Near: %.0f", renderEngine->getNear());
		ImGui::Text("Far: %.0f", renderEngine->getFar());
		ImGui::Text("Near / far: %.1f", renderEngine->getFar() / renderEngine->getNear());
		ImGui::Text("FOV X: %.1f", renderEngine->getFovY() * renderEngine->getAspectRatio() * 180.0 / M_PI);
		ImGui::Text("FOV Y: %.1f", renderEngine->getFovY() * 180.0 / M_PI);
		ImGui::Text("X / Y: %.3f", renderEngine->getAspectRatio());
	}
}


Program::Program() :
	window("test", 50, 50, 800, 800),
	renderEngine(nullptr),
	camera(nullptr),
	input(nullptr),
	evc(nullptr),
	seeder(nullptr) {}


// Called to start the program. Conducts set up then enters the main loop
void Program::start() {	

	const double initialCameraDist = RADIUS_EARTH_M * 3.0;

	renderEngine = new RenderEngine(window, initialCameraDist);
	camera = new Camera(initialCameraDist);
	evc = new EarthViewController(*camera, *renderEngine, initialCameraDist);
	input = new InputHandler(*camera, *renderEngine, *evc, *this);

	// Load coastline vector data and set up renderable for it
	rapidjson::Document cl = ContentReadWrite::readJSON("./data/coastlines.json");
	coastRender = ColourRenderable(cl);

	ContentReadWrite::loadOBJ("./data/sphere.obj", sphereRender);

	coastRender.assignBuffers();
	coastRender.setBufferData();
	sphereRender.assignBuffers();
	sphereRender.setBufferData();

	// Load vector field
	netCDF::NcFile file("./data/2017-09-05T12.nc", netCDF::NcFile::read);
	field = SphericalVectorField(file);
	seeder = new SeedingEngine(field);

	// TODO work out multithreading
	//std::thread t1(&SeedingEngine::seedGlobal, seeder);
	seeder->seed();
	mainLoop();
}


// Main loop of the program
void Program::mainLoop() {

	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
	std::vector<Renderable*> objects;


	// Temp test code for multiview focus & context
	BaseSubWindow s1(window, 550, 50, 200, 200, 0.3, -1.2);
	std::vector<Renderable*> objects2;
	// End temp test code


	while (true) {
		
		// Process SDL events
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			input->pollEvent(e);
		}

		window.renderSetup();

		ImGui::Begin("Options");
		ImGui();
		seeder->ImGui();
		renderEngine->ImGui();
		ImGui::End();

		// Update time since last frame
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		std::chrono::duration<float> dTimeS = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
		t0 = t1;

		// Render everything
		ImGui::Render();
		
		objects = seeder->getLinesToRender(Frustum(*camera, *renderEngine), camera->getDist());
		objects.push_back(&coastRender);
		objects.push_back(&sphereRender);
		
		renderEngine->render(objects, camera->getLookAt(), dTimeS.count());


		objects2 = seeder->getLinesToRender(s1.getFrustum(), s1.getCameraDist());
		objects2.push_back(&coastRender);
		objects2.push_back(&sphereRender);

		s1.render(objects2, dTimeS.count());

		window.finalizeRender();
	}
}


// Perform cleanup and exit
// TODO cleanup steamline renders
void Program::cleanup() {

	coastRender.deleteBufferData();
	sphereRender.deleteBufferData();
	//streamlineRender.deleteBufferData();

	delete renderEngine;
	delete camera;
	delete evc;
	delete input;
	delete seeder;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_Quit();
	exit(0);
}