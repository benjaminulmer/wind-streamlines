#define _USE_MATH_DEFINES
#define GLM_ENABLE_EXPERIMENTAL
#include "Program.h"

#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Camera.h"
#include "ContentReadWrite.h"
#include "Conversions.h"
#include "InputHandler.h"
#include "RenderEngine.h"
#include "SeedingEngine.h"
#include "VoxelGrid.h"

#include <GL/glew.h>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>
#include <netcdf>
#include <SDL2/SDL_opengl.h>

#include <chrono>
#include <iostream>


// Dear ImGUI window. Show misc statuses
void Program::ImGui() {
	ImGui::Begin("Status");
	ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	seeder->ImGui();
	ImGui::End();
}


Program::Program() :
	window(nullptr),
	width(800),
	height(800),
	io(nullptr),
	renderEngine(nullptr),
	camera(nullptr),
	input(nullptr),
	seeder(nullptr),
	//numNewLines(0),
	scale(10.0),
	latRot(0.0),
	lngRot(0.0) {}


// Called to start the program. Conducts set up then enters the main loop
void Program::start() {	

	setupWindow();
	GLenum err = glewInit();
	if (glewInit() != GLEW_OK) {
		std::cerr << "glewInit error: " << glewGetErrorString(err) << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	camera = new Camera(scale);
	renderEngine = new RenderEngine(window);
	input = new InputHandler(camera, renderEngine, this);

	// Load coastline vector data and set up renderable for it
	rapidjson::Document cl = ContentReadWrite::readJSON("data/coastlines.json");
	coastRender = ColourRenderable(cl);

	//objects.push_back(&coastRender);
	coastRender.assignBuffers();
	coastRender.setBufferData();

	// Load vector field
	netCDF::NcFile file("data/2018-05-27T12.nc", netCDF::NcFile::read);
	field = SphericalVectorField(file);
	seeder = new SeedingEngine(field);

	// Set up renderable for streamlines
	//objects.push_back(&streamlineRender);
	//streamlineRender.assignBuffers();
	//streamlineRender.setDrawMode(GL_LINES);

	// Start integration and rendering in separate threads
	//numNewLines = 0;
	std::thread t1(&SeedingEngine::seedGlobal, seeder);
	mainLoop();
}


// Creates SDL OpenGL window for the program and connects it with Dear ImGUI 
void Program::setupWindow() {

	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	window = SDL_CreateWindow("615 Project", 10, 30, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		system("pause");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
		system("pause");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	SDL_GL_SetSwapInterval(0); // Vsync on

	// Set up IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO();
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	const char* glsl_version = "#version 430 core";
	ImGui_ImplOpenGL3_Init(glsl_version);
}


// Main loop of the program
void Program::mainLoop() {

	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

	while (true) {

		// Update streamline renderable if new ones have been integrated
		//if (numNewLines > 0) {

		//	mtx.lock();
		//	size_t size = streamlines.size();
		//	for (size_t i = 0; i < numNewLines; i++) {

		//		StreamlineRenderable* r = new StreamlineRenderable();
		//		r->setDrawMode(GL_LINES);
		//		r->assignBuffers();
		//		streamlines[size - 1 - i].addToRenderable(*r);
		//		r->setBufferData();
		//		objects.push_back(r);
		//	}
		//	numNewLines = 0;

		//	mtx.unlock();
		//}
		
		// Process SDL events
		SDL_Event e;
		while (SDL_PollEvent(&e)) {

			ImGui_ImplSDL2_ProcessEvent(&e);
			if (io->WantCaptureMouse && (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || 
				                         e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEWHEEL)) {
				continue;
			}
			if (io->WantCaptureKeyboard && (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)) {
				continue;
			}
			input->pollEvent(e);
		}

		// TODO replace Earth reference model and remove all code related to this effect
		// Find min and max distance from camera to cell renderable - used for fading effect
		glm::vec3 cameraPos = camera->getPosition();
		float max = glm::length(cameraPos) + (float)scale;
		float min = glm::length(cameraPos) - (float)scale;

		glm::dmat4 worldModel(1.f);
		double s = scale * (1.0 / RADIUS_EARTH_M);
		worldModel = glm::scale(worldModel, glm::dvec3(s, s, s));
		worldModel = glm::rotate(worldModel, latRot, glm::dvec3(-1.0, 0.0, 0.0));
		worldModel = glm::rotate(worldModel, lngRot, glm::dvec3(0.0, 1.0, 0.0));

		// Dear ImGUI setup
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		renderEngine->ImGui();
		ImGui();

		// Update time since last frame
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		std::chrono::duration<float> dTimeS = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
		t0 = t1;

		// Render everything
		ImGui::Render();
		std::vector<Renderable*> objects = seeder->getLinesToRender(5);
		objects.push_back(&coastRender);
		renderEngine->render(objects, (glm::dmat4)camera->getLookAt() * worldModel, max, min, dTimeS.count());
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
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

	double oldXN = (2.0 * oldX) / (width) - 1.0; 
	double oldYN = (2.0 * oldY) / (height) - 1.0;
	oldYN *= -1.0;

	double newXN = (2.0 * newX) / (width) - 1.0;
	double newYN = (2.0 * newY) / (height) - 1.0;
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
	double sphereRad = scale;
	glm::dvec3 sphereO = glm::dvec3(0.0);

	glm::dvec3 iPosOld, iPosNew, iNorm;

	if (glm::intersectRaySphere(rayO, rayDOld, sphereO, sphereRad, iPosOld, iNorm) && 
			glm::intersectRaySphere(rayO, rayDNew, sphereO, sphereRad, iPosNew, iNorm)) {

		double longOld = atan2(iPosOld.x, iPosOld.z);
		double latOld = M_PI_2 - acos(iPosOld.y / sphereRad);

		double longNew = atan2(iPosNew.x, iPosNew.z);
		double latNew = M_PI_2 - acos(iPosNew.y / sphereRad);
		
		if (skew) {
			camera->updateFromVertical(newYN - oldYN);
			camera->updateNorthRotation(newXN - oldXN);
		}
		else {
			latRot += latNew - latOld;
			lngRot += longNew - longOld;
		}
	}
}


// Changes scale of Earth model (zooms camera)
//
// dir - direction of change, possitive or negative
void Program::updateScale(int dir) {

	if (dir < 0) {
		scale /= 1.2f;
	}
	else if (dir > 0) {
		scale *= 1.2f;
	}
	camera->setScale(scale);
}


// Perform cleanup and exit
void Program::cleanup() {

	coastRender.deleteBufferData();
	//streamlineRender.deleteBufferData();

	SDL_DestroyWindow(window);
	delete renderEngine;
	delete camera;
	delete input;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	exit(0);
}