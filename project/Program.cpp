#define _USE_MATH_DEFINES
#include "Program.h"

#include <GL/glew.h>
#include <glm/gtx/intersect.hpp>
#include <netcdf>
#include <SDL2/SDL_opengl.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>

#include "Conversions.h"
#include "ContentReadWrite.h"
#include "InputHandler.h"


Program::Program() {

	window = nullptr;
	renderEngine = nullptr;
	camera = nullptr;

	longRot = 0;
	latRot = 0;

	width = height = 800;
}


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
	InputHandler::setUp(camera, renderEngine, this);

	// Set starting scale
	scale = 1.0;

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

	reIntegrate = true;
	mainLoop();
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

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
	}
	SDL_GL_SetSwapInterval(1); // Vsync on
}


// Main loop
void Program::mainLoop() {

	while (true) {

		if (reIntegrate) {
			integrateStreamlines();
			reIntegrate = false;
		}

		// Process all SDL events
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			InputHandler::pollEvent(e);
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

		renderEngine->render(objects, (glm::dmat4)camera->getLookAt() * worldModel, max, min);
		SDL_GL_SwapWindow(window);
	}
}


// Seeds and integrates streamlines
void Program::integrateStreamlines() {

	//streamlineRender.clear();
	//streamlines.clear();

	//glm::dmat4 worldModel(1.f);
	//worldModel = glm::rotate(worldModel, latRot, glm::dvec3(-1.0, 0.0, 0.0));
	//worldModel = glm::rotate(worldModel, longRot, glm::dvec3(0.0, 1.0, 0.0));
	//worldModel = glm::inverse(worldModel);

	//glm::dmat4 projView = renderEngine->getProjection() * camera->getLookAt();
	//glm::dmat4 invProjView = glm::inverse(projView);


	//int num = 30;
	//for (int x = 0; x < num; x++) {
	//	for (int y = 0; y < num; y++) {

	//		double step = 2.0 / num;
	//		double xScreen = -1.0 + x * step;
	//		double yScreen = -1.0 + y * step;

	//		glm::dvec4 world(xScreen, yScreen, -1.0, 1.0);

	//		world = invProjView * world;
	//		world /= world.w;

	//		glm::dvec3 rayO = camera->getPosition();
	//		glm::dvec3 rayD = glm::normalize(glm::dvec3(world) - rayO);
	//		double sphereRad = RADIUS_EARTH_VIEW * scale;
	//		glm::dvec3 sphereO = glm::dvec3(0.0);

	//		glm::dvec3 iPos, iNorm;

	//		if (glm::intersectRaySphere(rayO, rayD, sphereO, sphereRad, iPos, iNorm)) {

	//			iPos = worldModel * glm::dvec4(iPos, 1.0);

	//			double lng = atan2(iPos.x, iPos.z);
	//			if (lng < 0.0) lng += 2.0 * M_PI;
	//			double lat = M_PI_2 - acos(iPos.y / sphereRad);

	//			for (int i = 0; i < 37; i++) {

	//				Eigen::Vector3d pos(lat, lng, field.level(i));

	//				Streamline s = field.streamline(pos, 1000000.0, 1000.0, 5000.0);
	//				streamlines.push_back(s);
	//				s.addToRenderable(streamlineRender, 0.0);
	//			}
	//		}
	//	}
	//}

	//size_t count = 0;
	//for (const Streamline& s : streamlines) {
	//	count += s.size();
	//}
	//std::cout << count << std::endl;
	std::random_device dev;
	std::default_random_engine rng(dev());
	std::uniform_real_distribution<double> latDist(-1.0, 1.0);
	std::uniform_real_distribution<double> lngDist(0.0, 2.0 * M_PI);
	std::uniform_real_distribution<double> lvlDist(1.0, 1000.0);

	std::cout << "starting critical points" << std::endl;
	std::vector<std::pair<Eigen::Matrix<size_t, 3, 1>, int>> criticalIndices;// = field.findCriticalPoints();
	std::cout << criticalIndices.size() << std::endl;

	double maxA = -1.0;

	for (int i = 0; i < 3000; i++) {

		Eigen::Vector3d pos(asin(latDist(rng)), lngDist(rng), lvlDist(rng));
		streamlines.push_back(field.streamline(pos, 1000000.0, 1000.0, 5000.0));
		//std::cout << streamlines.back().getTotalLength() << " : " << streamlines.back().getTotalAngle() << std::endl;

		double ratio = streamlines.back().getTotalAngle() / streamlines.back().getTotalLength();
		//std::cout << ratio << std::endl;
		maxA = std::max(ratio, maxA);
	}
	for (const Streamline& s : streamlines) {
		s.addToRenderable(streamlineRender);
	}
	streamlineRender.setBufferData();
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