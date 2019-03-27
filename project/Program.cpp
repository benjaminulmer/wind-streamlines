#define _USE_MATH_DEFINES
#include "Program.h"

#include <GL/glew.h>
#include <glm/gtx/intersect.hpp>
#include <netcdf>
#include <SDL2/SDL_opengl.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

#include "Constants.h"
#include "ContentReadWrite.h"
#include "InputHandler.h"
#include "Geometry.h"
#include "SphCoord.h"

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

	// Set starting radius
	scale = 1.0;
	radius = RADIUS_EARTH_M * 4.0 / 3.0;

	// Load coastline vector data
	rapidjson::Document cl = ContentReadWrite::readJSON("data/coastlines.json");
	coastLines = Renderable(cl);

	// Load vector field and find critical points
	netCDF::NcFile file("data/2018-05-27T12.nc", netCDF::NcFile::read);
	field = SphericalVectorField(file);

	testLine.drawMode = GL_LINE_STRIP;
	testLine2.drawMode = GL_LINE_STRIP;

	std::cout << "Integrating small h" << std::endl;
	std::vector<Eigen::Vector3d> line = field.streamLine(Eigen::Vector3d(0.0, 3.2, 990), 1000000.0, 1.0);

	std::cout << "Integrating large h" << std::endl;
	std::vector<Eigen::Vector3d> line2 = field.streamLine(Eigen::Vector3d(0.0, 3.2, 990.0), 1000000.0, 10.0);

	for (const Eigen::Vector3d& p : line) {
		SphCoord sph(p(0), p(1));
			
		double alt = altToAbs(SphericalVectorField::mbarsToMeters(p(2)));
		testLine.verts.push_back(sph.toCartesian(alt));

		float norm = (SphericalVectorField::mbarsToMeters(p(2)) / 66000.0) + 0.5f;

		testLine.colours.push_back(glm::vec3(0.f, norm, 0.f));

		//std::cout << p << std::endl;
	}

	for (const Eigen::Vector3d& p : line2) {
		SphCoord sph(p(0), p(1));

		double alt = altToAbs(SphericalVectorField::mbarsToMeters(p(2)));
		testLine2.verts.push_back(sph.toCartesian(alt));

		float norm = (SphericalVectorField::mbarsToMeters(p(2)) / 66000.0) + 0.5f;

		testLine2.colours.push_back(glm::vec3(norm, 0.f, 0.f));

		//std::cout << p << std::endl;
	}

	//std::vector<std::pair<Eigen::Vector3i, int>> criticalIndices = field.findCriticalPoints();

	//for (const std::pair<Eigen::Vector3i, int>& i : criticalIndices) {
	//	Eigen::Vector3d coords = field.sphCoords(i.first);
	//	SphCoord sph(coords(0), coords(1));
	//	
	//	double alt = altToAbs(coords(2));
	//	criticalPoints.verts.push_back(sph.toCartesian(alt));

	//	float norm = (coords(2) / 66000.0) + 0.5f;
	//	if (i.second == 1) {
	//		criticalPoints.colours.push_back(glm::vec3(norm, 0.f, 0.f));
	//	}
	//	else {
	//		criticalPoints.colours.push_back(glm::vec3(0.f, 0.f, norm));
	//	}

	//}
	//std::cout << criticalIndices.size() << std::endl;


	// Objects to draw initially
	objects.push_back(&coastLines);
	objects.push_back(&testLine);
	objects.push_back(&testLine2);
	objects.push_back(&criticalPoints);

	coastLines.doubleToFloats();
	testLine.doubleToFloats();
	testLine2.doubleToFloats();
	criticalPoints.doubleToFloats();

	RenderEngine::assignBuffers(coastLines, false);
	RenderEngine::assignBuffers(testLine, false);
	RenderEngine::assignBuffers(testLine2, false);
	RenderEngine::assignBuffers(criticalPoints, false);

	RenderEngine::setBufferData(coastLines, false);
	RenderEngine::setBufferData(testLine, false);
	RenderEngine::setBufferData(testLine2, false);
	RenderEngine::setBufferData(criticalPoints, false);

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

		// Process all SDL events
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			InputHandler::pollEvent(e);
		}

		// Find min and max distance from camera to cell renderable - used for fading effect
		glm::vec3 cameraPos = camera->getPosition();
		float max = glm::length(cameraPos) + RADIUS_EARTH_VIEW;
		float min = glm::length(cameraPos) - RADIUS_EARTH_VIEW;

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