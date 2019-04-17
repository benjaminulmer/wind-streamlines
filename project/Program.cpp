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
#include "VoxelGrid.h"

#include <GL/glew.h>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>
#include <netcdf>
#include <SDL2/SDL_opengl.h>

#include <chrono>
#include <iostream>
#include <queue>
#include <random>


Program::Program() :
	window(nullptr),
	width(800),
	height(800),
	io(nullptr),
	renderEngine(nullptr),
	camera(nullptr),
	input(nullptr),
	numNewLines(0),
	scale(1.0),
	latRot(0.0),
	longRot(0.0) {}

// Called to start the program. Conducts set up then enters the main loop
void Program::start() {	

	setupWindow();
	GLenum err = glewInit();
	if (glewInit() != GLEW_OK) {
		std::cerr << glewGetErrorString(err) << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	camera = new Camera();
	renderEngine = new RenderEngine(window);
	input = new InputHandler(camera, renderEngine, this);

	// Load coastline vector data
	rapidjson::Document cl = ContentReadWrite::readJSON("data/coastlines.json");
	coastRender = ColourRenderable(cl);

	objects.push_back(&coastRender);
	coastRender.assignBuffers();
	coastRender.setBufferData();

	// Load vector field
	netCDF::NcFile file("data/2018-05-27T12.nc", netCDF::NcFile::read);
	field = SphericalVectorField(file);

	// Set up renderable for streamlines
	objects.push_back(&streamlineRender);
	streamlineRender.assignBuffers();
	streamlineRender.setDrawMode(GL_LINES);

	numNewLines = 0;
	std::thread t1(&Program::integrateStreamlines, this);
	mainLoop();

	t1.join();
}


// Creates SDL window for the program and sets callbacks for input
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
		//TODO: cleanup methods upon exit
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
	}
	//SDL_GL_SetSwapInterval(1); // Vsync on

	// Set up IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO();
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	const char* glsl_version = "#version 430 core";
	ImGui_ImplOpenGL3_Init(glsl_version);
}


// Main loop
void Program::mainLoop() {

	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

	while (true) {

		if (numNewLines > 0) {
			mtx.lock();
			size_t size = streamlines.size();
			for (size_t i = 0; i < numNewLines; i++) {
				streamlines[size - 1 - i].addToRenderable(streamlineRender);
			}
			numNewLines = 0;
			streamlineRender.setBufferData();
			mtx.unlock();
		}

		// Process all SDL events
		SDL_Event e;
		while (SDL_PollEvent(&e)) {

			ImGui_ImplSDL2_ProcessEvent(&e);
			if (!io->WantCaptureMouse || !io->WantCaptureKeyboard) {
				input->pollEvent(e);
			}
		}

		// Find min and max distance from camera to cell renderable - used for fading effect
		glm::vec3 cameraPos = camera->getPosition();
		float max = glm::length(cameraPos) + (float)RADIUS_EARTH_VIEW;
		float min = glm::length(cameraPos) - (float)RADIUS_EARTH_VIEW;

		glm::dmat4 worldModel(1.f);
		double s = scale * (1.0 / RADIUS_EARTH_M) * RADIUS_EARTH_VIEW;
		worldModel = glm::scale(worldModel, glm::dvec3(s, s, s));
		worldModel = glm::rotate(worldModel, latRot, glm::dvec3(-1.0, 0.0, 0.0));
		worldModel = glm::rotate(worldModel, longRot, glm::dvec3(0.0, 1.0, 0.0));

		// Testing
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		{
			ImGui::Begin("Parameters");
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Checkbox("Specular highlights", &renderEngine->specular);
			ImGui::Checkbox("Stop", &this->stop);
			ImGui::InputFloat("Time scale factor", &renderEngine->timeMultiplier, 100.f, 1000.f);
			ImGui::InputFloat("Time repeat interval", &renderEngine->timeRepeat, 100.f, 1000.f);
			ImGui::SliderFloat("Alpha multiplier/s", &renderEngine->alphaPerSecond, 0.0f, 1.0f);
			ImGui::SliderFloat("Altitude scale factor", &renderEngine->scaleFactor, 0.0f, 100.f);


			ImGui::End();
		}


		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		std::chrono::duration<float> dTimeS = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
		t0 = t1;

		ImGui::Render();
		renderEngine->render(objects, (glm::dmat4)camera->getLookAt() * worldModel, max, min, dTimeS.count());
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}
}


// Seeds and integrates streamlines
void Program::integrateStreamlines() {

	//std::random_device dev;
	//std::default_random_engine rng(dev());
	//rng.seed(1);
	//std::uniform_real_distribution<double> latDist(-1.0, 1.0);
	//std::uniform_real_distribution<double> lngDist(0.0, 2.0 * M_PI);
	//std::uniform_real_distribution<double> lvlDist(1.0, 1000.0);

	double minLength = 1000000.0;
	double sepDist = 250000.0;

	VoxelGrid vg(mbarsToAbs(1.0) + 100.0, sepDist);


	//while (streamlines.size() < 2000 && !stop) {

	//	Eigen::Vector3d seed(asin(latDist(rng)), lngDist(rng), lvlDist(rng));
	//	if (!vg.testPoint(sphToCart(seed))) {
	//		continue;
	//	}
	//	Streamline newLine = field.streamline(seed, 10000000.0, 1000.0, 5000.0, vg);

	//	if (newLine.getTotalLength() > minLength) {

	//		for (const Eigen::Vector3d& p : newLine.getPoints()) {
	//			vg.addPoint(sphToCart(p));
	//		}
	//		mtx.lock();
	//		numNewLines++;
	//		streamlines.push_back(newLine);
	//		mtx.unlock();
	//		std::cout << newLine.size() << " : " << streamlines.size() << " : " << streamlineRender.tangents.size() << std::endl;
	//	}
	//}
	//std::cout << "end" << std::endl;
	//vg.prints();

	std::queue<Streamline> seedLines;

	//for (const Eigen::Vector3d& p : field.getHighVertPoints()) {
	//	Streamline line = field.streamline(p, 5000000.0, 1000.0, 5000.0, vg);
	//	for (const Eigen::Vector3d& p : line.getPoints()) {
	//		vg.addPoint(sphToCart(p));
	//	}
	//	seedLines.push(line);
	//	mtx.lock();
	//	numNewLines++;
	//	streamlines.push_back(line);
	//	mtx.unlock();
	//}
	//std::cout << "high vert done" << std::endl;
	//std::this_thread::sleep_for(std::chrono::seconds(50));
	//std::cout << "resuming" << std::endl;


	// Need a starting streamline to seed off of
	Streamline first = field.streamline(Eigen::Vector3d(0.0, 0.0, 999.0), 10000000.0, 1000.0, 10000.0, vg);
	for (const Eigen::Vector3d& p : first.getPoints()) {
		vg.addPoint(sphToCart(p));
	}
	seedLines.push(first);
	mtx.lock();
	numNewLines++;
	streamlines.push_back(first);
	mtx.unlock();

	while (!seedLines.empty()) {

		std::cout << seedLines.size() << std::endl;

		Streamline seedLine = seedLines.front();
		seedLines.pop();

		std::vector<Eigen::Vector3d> seeds = seedLine.getSeeds(sepDist);

		for (const Eigen::Vector3d& seed : seeds) {

			// Do not use seed if it is too close to other lines
			if (!vg.testPoint(sphToCart(seed))) {
				continue;
			}

			// Integrate streamline and add it if it was long enough
			Streamline newLine = field.streamline(seed, 10000000.0, 1000.0, 10000.0, vg);
			if (newLine.getTotalLength() > minLength) {

				seedLines.push(newLine);
				for (const Eigen::Vector3d& p : newLine.getPoints()) {
					vg.addPoint(sphToCart(p));
				}
				mtx.lock();
				numNewLines++;
				streamlines.push_back(newLine);
				mtx.unlock();
			}
		}
	}
	std::cout << "end" << std::endl;



	//std::random_device dev;
	//std::default_random_engine rng(dev());
	//std::uniform_real_distribution<double> latDist(-1.0, 1.0);
	//std::uniform_real_distribution<double> lngDist(0.0, 2.0 * M_PI);
	//std::uniform_real_distribution<double> lvlDist(1.0, 1000.0);

	//std::cout << "starting critical points" << std::endl;
	//std::vector<std::pair<Eigen::Matrix<size_t, 3, 1>, int>> criticalIndices;// = field.findCriticalPoints();
	//std::cout << criticalIndices.size() << std::endl;

	//double maxA = -1.0;

	//for (int i = 0; i < 3000; i++) {

	//	Eigen::Vector3d pos(asin(latDist(rng)), lngDist(rng), lvlDist(rng));
	//	streamlines.push_back(field.streamline(pos, 1000000.0, 1000.0, 5000.0));
	//	//std::cout << streamlines.back().getTotalLength() << " : " << streamlines.back().getTotalAngle() << std::endl;

	//	double ratio = streamlines.back().getTotalAngle() / streamlines.back().getTotalLength();
	//	//std::cout << ratio << std::endl;
	//	maxA = std::max(ratio, maxA);
	//}
	//for (const Streamline& s : streamlines) {
	//	s.addToRenderable(streamlineRender);
	//}
	//streamlineRender.setBufferData();
}


// Updates camera rotation
// Locations are in pixel coordinates
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
	double sphereRad = RADIUS_EARTH_VIEW * scale;
	glm::dvec3 sphereO = glm::dvec3(0.0);

	glm::dvec3 iPosOld, iPosNew, iNorm;

	if (glm::intersectRaySphere(rayO, rayDOld, sphereO, sphereRad, iPosOld, iNorm) && 
			glm::intersectRaySphere(rayO, rayDNew, sphereO, sphereRad, iPosNew, iNorm)) {

		double longOld = atan2(iPosOld.x, iPosOld.z);
		double latOld = M_PI_2 - acos(iPosOld.y / sphereRad);

		double longNew = atan2(iPosNew.x, iPosNew.z);
		double latNew = M_PI_2 - acos(iPosNew.y / sphereRad);
		
		if (skew) {
			camera->updateLatitudeRotation(latNew - latOld);
		}
		else {
			latRot += latNew - latOld;
			longRot += longNew - longOld;
		}
		//reIntegrate = true;
	}
}


// Changes scale of model
void Program::updateScale(int inc) {

	if (inc < 0) {
		scale /= 1.4f;
	}
	else {
		scale *= 1.4f;
	}
	camera->setScale(scale);
}


void Program::cleanup() {

	coastRender.deleteBufferData();
	streamlineRender.deleteBufferData();

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