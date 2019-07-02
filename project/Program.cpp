#include "Program.h"

#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include "ContentReadWrite.h"
#include "Conversions.h"
#include "Frustum.h"
#include "InputHandler.h"
#include "VoxelGrid.h"
#include "rendering/Camera.h"
#include "rendering/RenderEngine.h"
#include "streamlines/SeedingEngine.h"

#include <GL/glew.h>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>
#include <netcdf>
#include <SDL2/SDL_opengl.h>

#include <chrono>
#include <iostream>


// Dear ImGUI window. Show misc statuses
void Program::ImGui() {
	if (ImGui::CollapsingHeader("Status")) {
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Camera dist: %.0f", cameraDist);
		ImGui::Text("Near: %.0f", renderEngine->getNear());
		ImGui::Text("Far: %.0f", renderEngine->getFar());
		ImGui::Text("Near / far: %.1f", renderEngine->getFar() / renderEngine->getNear());
		ImGui::Text("FOV X: %.1f", renderEngine->getFovY() * renderEngine->getAspectRatio() * 180.0 / M_PI);
		ImGui::Text("FOV Y: %.1f", renderEngine->getFovY() * 180.0 / M_PI);
		ImGui::Text("X / Y: %.3f", renderEngine->getAspectRatio());
	}
}


Program::Program() :
	renderEngine(nullptr),
	camera(nullptr),
	input(nullptr),
	seeder(nullptr),
	cameraDist(RADIUS_EARTH_M * 3.0) {}


// Called to start the program. Conducts set up then enters the main loop
void Program::start() {	

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	renderEngine = new RenderEngine(cameraDist);
	camera = new Camera(cameraDist);
	input = new InputHandler(camera, renderEngine, this);

	// Load coastline vector data and set up renderable for it
	rapidjson::Document cl = ContentReadWrite::readJSON("./data/coastlines.json");
	coastRender = ColourRenderable(cl);

	ContentReadWrite::loadOBJ("./data/sphere.obj", sphereRender);

	coastRender.assignBuffers();
	coastRender.setBufferData();
	sphereRender.assignBuffers();
	sphereRender.setBufferData();

	// Load vector field
	netCDF::NcFile file("./data/2018-05-27T12.nc", netCDF::NcFile::read);
	field = SphericalVectorField(file);
	seeder = new SeedingEngine(field);

	// Start integration and rendering in separate threads
	//std::thread t1(&SeedingEngine::seedGlobal, seeder);
	seeder->seed();
	mainLoop();
}


// Main loop of the program
void Program::mainLoop() {

	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
	std::vector<Renderable*> objects;

	while (true) {
		
		// Process SDL events
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			input->pollEvent(e);
		}

		renderEngine->preRender();

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
		
		objects = seeder->getLinesToRender(Frustum(*camera, *renderEngine), cameraDist);
		objects.push_back(&coastRender);
		objects.push_back(&sphereRender);
		
		renderEngine->render(objects, (glm::dmat4)camera->getLookAt(), dTimeS.count());
		renderEngine->postRender();
	}
}


// Updates rotation/orientation of Earth model
//
// oldX - old x pixel location of mouse
// oldY - old y pixel location of mouse
// newX - new x pixel location of mouse
// newY - new y pixel location of mouse
// skew - true tilts the camera, otherwise rotates the Earth
void Program::updateRotation(int oldX, int newX, int oldY, int newY, bool skew) {

	glm::dmat4 projView = renderEngine->getProjection() * camera->getLookAt();
	glm::dmat4 invProjView = glm::inverse(projView);

	double oldXN = (2.0 * oldX) / (renderEngine->getWidth()) - 1.0; 
	double oldYN = (2.0 * oldY) / (renderEngine->getHeight()) - 1.0;
	oldYN *= -1.0;

	double newXN = (2.0 * newX) / (renderEngine->getWidth()) - 1.0;
	double newYN = (2.0 * newY) / (renderEngine->getHeight()) - 1.0;
	newYN *= -1.0;

	glm::dvec4 worldOld(oldXN, oldYN, -1.0, 1.0);
	glm::dvec4 worldNew(newXN, newYN, -1.0, 1.0);

	worldOld = invProjView * worldOld; 
	worldOld /= worldOld.w;

	worldNew = invProjView * worldNew;
	worldNew /= worldNew.w;

	glm::dvec3 rayO = camera->getPosition();
	glm::dvec3 rayDOld = glm::normalize(glm::dvec3(worldOld) - rayO);
	glm::dvec3 rayDNew = glm::normalize(glm::dvec3(worldNew) - rayO);
	double sphereRad = RADIUS_EARTH_M;
	glm::dvec3 sphereO = glm::dvec3(0.0);

	glm::dvec3 iPosOld, iPosNew, iNorm;

	if (glm::intersectRaySphere(rayO, rayDOld, sphereO, sphereRad, iPosOld, iNorm) && 
			glm::intersectRaySphere(rayO, rayDNew, sphereO, sphereRad, iPosNew, iNorm)) {

		double longOld = atan2(iPosOld.x, iPosOld.z);
		double latOld = M_PI_2 - acos(iPosOld.y / sphereRad);

		double longNew = atan2(iPosNew.x, iPosNew.z);
		double latNew = M_PI_2 - acos(iPosNew.y / sphereRad);
		
		if (skew) {
			camera->updateFromVertRot(newYN - oldYN);
			camera->updateNorthRot(oldXN - newXN);
		}
		else {
			camera->updateLatRot(latNew - latOld);
			camera->updateLngRot(longNew - longOld);
		}
	}
}


// Moves camera towards or away from Earth
//
// dir - direction of change, possitive for closer and negative for farther
void Program::updateCameraDist(int dir) {

	if (dir > 0) {
		cameraDist /= 1.2f;
	}
	else if (dir < 0) {
		cameraDist *= 1.2f;
	}
	camera->setDist(cameraDist);
	renderEngine->updatePlanes(cameraDist);
}


// Perform cleanup and exit
// TODO cleanup steamline renders
void Program::cleanup() {

	coastRender.deleteBufferData();
	sphereRender.deleteBufferData();
	//streamlineRender.deleteBufferData();

	delete renderEngine;
	delete camera;
	delete input;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_Quit();
	exit(0);
}