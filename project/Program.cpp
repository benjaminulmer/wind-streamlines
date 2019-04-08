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
#include "Streamline.h"

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
	coastLines = Renderable(cl);

	objects.push_back(&coastLines);
	coastLines.doubleToFloats();
	RenderEngine::assignBuffers(coastLines, false);
	RenderEngine::setBufferData(coastLines, false);

	// Load vector field and integrate streamlines
	netCDF::NcFile file("data/2018-05-27T12.nc", netCDF::NcFile::read);
	field = SphericalVectorField(file);
	integrateStreamlines();

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



std::vector<Eigen::Vector3d> reverseChaikin(const std::vector<Eigen::Vector3d>& fine) {

	std::vector<Eigen::Vector3d> coarse;

	coarse.push_back(fine[0]);
	coarse.push_back(-0.5 * fine[0] + fine[1] + 0.75 * fine[2] - 0.25 * fine[3]);
	
	for (size_t i = 2; i < fine.size() - 5; i += 2) {
		coarse.push_back(-0.25 * fine[i] + 0.75 * fine[i + 1] + 0.75 * fine[i + 2] - 0.25 * fine[i + 3]);
	}
	size_t m = fine.size() - 1;

	coarse.push_back(-0.25 * fine[m - 3] + 0.75 * fine[m - 2] + fine[m - 1] - 0.5 * fine[m]);
	coarse.push_back(fine[m]);

	return coarse;
}

void toCartesian(std::vector<Eigen::Vector3d>& points) {

	for (Eigen::Vector3d& p : points) {
		p = sphToCart(p); 
	}
}


// Seeds and integrates streamlines
void Program::integrateStreamlines2() {

	std::random_device dev;
	std::default_random_engine rng(dev());
	std::uniform_real_distribution<double> latDist(-1.0, 1.0);
	std::uniform_real_distribution<double> lngDist(0.0, 2.0 * M_PI);
	std::uniform_real_distribution<double> lvlDist(1.0, 1000.0);

	Renderable* points = new Renderable();
	Renderable* lines = new Renderable();

	std::cout << "starting critical points" << std::endl;
	std::vector<std::pair<Eigen::Matrix<size_t, 3, 1>, int>> criticalIndices;// = field.findCriticalPoints();
	std::cout << criticalIndices.size() << std::endl;

	for (int i = 0; i < 3000; i++) {

		Eigen::Vector3d pos(asin(latDist(rng)), lngDist(rng), lvlDist(rng));
		Streamline line = field.streamline(pos, 100000.0, 1000.0, 5000.0);


		line.addToRenderable(*lines, field);
	}

	for (const auto& p : criticalIndices) {

		Renderable* r = new Renderable();

		Eigen::Vector3d coords = field.sphCoords(p.first);
		coords = sphToCart(coords);

		r->verts.push_back(glm::dvec3(coords.x(), coords.y(), coords.z()));

		if (p.second == 1) {
			r->colours.push_back(glm::vec3(0.f, 1.f, 0.f));
		}
		else {
			r->colours.push_back(glm::vec3(1.f, 0.f, 0.f));
		}
	}
	lines->drawMode = GL_LINES;
	objects.push_back(lines);
	lines->doubleToFloats();
	RenderEngine::assignBuffers(*lines, false);
	RenderEngine::setBufferData(*lines, false);

	objects.push_back(points);
	points->doubleToFloats();
	RenderEngine::assignBuffers(*points, false);
	RenderEngine::setBufferData(*points, false);
}


// Seeds and integrates streamlines
void Program::integrateStreamlines() {

	std::random_device dev;
	std::default_random_engine rng(dev());
	std::uniform_real_distribution<double> unif(-2.0, 2.0);

	std::cout << "starting critical points" << std::endl;
	std::vector<std::pair<Eigen::Matrix<size_t, 3, 1>, int>> criticalIndices = field.findCriticalPoints();
	std::cout << criticalIndices.size() << std::endl;

	Renderable* points = new Renderable();
	Renderable* lines = new Renderable();

	for (const auto& p : criticalIndices) {

		// Convert index to sph to cartesian
		Eigen::Vector3d sph = field.sphCoords(p.first);
		Eigen::Vector3d cart = sphToCart(sph);

		// Add to renderable
		points->verts.push_back(glm::dvec3(cart.x(), cart.y(), cart.z()));
		if (p.second == 1) {
			points->colours.push_back(glm::vec3(0.f, 1.f, 0.f));
		}
		else {
			points->colours.push_back(glm::vec3(1.f, 0.f, 0.f));
		}

		// Seed streamlines around point
		for (int i = 0; i < 5; i++) {

			Eigen::Vector3d pos(sph.x() + (M_PI / 180.0) * unif(rng), sph.y() + (M_PI / 180.0) * unif(rng), sph.z() + unif(rng) * 50);

			if (pos.x() > M_PI_2) pos.x() = M_PI_2;
			if (pos.x() < -M_PI_2) pos.x() = -M_PI_2;
			if (pos.y() < 0.0) pos.y() += 2.0 * M_PI;
			if (pos.y() > 2.0 * M_PI) pos.y() -= 2.0 * M_PI;
			if (pos.z() > field.levels[37 - 1]) pos.z() = field.levels[37 - 1];
			if (pos.z() < field.levels[0]) pos.z() = field.levels[0];

			Streamline line = field.streamline(pos, 10000.0, 1000.0, 5000.0);
			//std::cout << line.getTotalLength() << " : " << line.getTotalAngle() << std::endl;
			line.addToRenderable(*lines, field);
		}
	}
	lines->drawMode = GL_LINES;
	objects.push_back(lines);
	lines->doubleToFloats();
	RenderEngine::assignBuffers(*lines, false);
	RenderEngine::setBufferData(*lines, false);

	objects.push_back(points);
	points->doubleToFloats();
	RenderEngine::assignBuffers(*points, false);
	RenderEngine::setBufferData(*points, false);
}

// Main loop
void Program::mainLoop() {

	while (true) {

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