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

#include "Constants.h"
#include "ContentReadWrite.h"
#include "InputHandler.h"
#include "SphCoord.h"

Program::Program() {

	window = nullptr;
	renderEngine = nullptr;
	camera = nullptr;

	longRot = 0;
	latRot = 0;

	width = height = 800;
}

#include <random>
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
		SphCoord sph(p.x(), p.y());
		double rad = RADIUS_EARTH_M + SphericalVectorField::mbarsToMeters(p.z());
		glm::dvec3 cart = sph.toCartesian(rad);

		p = Eigen::Vector3d(cart.x, cart.y, cart.z);
	}
}



// Seeds and integrates streamlines
void Program::integrateStreamlines() {

	std::random_device dev;
	std::default_random_engine rng(dev());
	rng.seed(1);
	std::uniform_real_distribution<double> latDist(-1.0, 1.0);
	std::uniform_real_distribution<double> lngDist(0.0, 2.0 * M_PI);
	std::uniform_real_distribution<double> lvlDist(0.0, 1000.0);


	for (int i = 0; i < 3000; i++) {

		Renderable* r = new Renderable();
		Eigen::Vector3d pos(asin(latDist(rng)), lngDist(rng), lvlDist(rng));
		std::vector<Eigen::Vector3d> line = field.streamline(pos, 100000.0, 25.0);
		toCartesian(line);

		for (int i = 0; i < 5; i++) {
			line = reverseChaikin(line);
		}
		if (line.size() < 50) std::cout << line.size() << std::endl;

		for (const Eigen::Vector3d& p : line) {

			//SphCoord sph(p.x(), p.y());
			//double rad = altToAbs(SphericalVectorField::mbarsToMeters(p.z()));

			//double speed = field.velocityAt(p).squaredNorm();
			//double norm = speed / (60.0 * 60.0);
			//if (norm > 1.0) norm = 1.0;
			//norm /= 2.0;
			//norm += 0.5;

			r->verts.push_back(glm::dvec3(p.x(), p.y(), p.z()));
			r->colours.push_back(glm::vec3(0.f, 0.f, 0.8f));
		}
		r->drawMode = GL_LINE_STRIP;
		streamlines.push_back(r);
		objects.push_back(r);
		r->doubleToFloats();
		RenderEngine::assignBuffers(*r, false);
		RenderEngine::setBufferData(*r, false);

		//std::cout << lat << ", " << lng << ", " << lvl << std::endl;
	}

	//std::vector<std::pair<Eigen::Matrix<size_t, 3, 1>, int>> criticalIndices = field.findCriticalPoints();
	//for (const auto& p : criticalIndices) {

	//	Renderable* r = new Renderable();

	//	Eigen::Vector3d coords = field.sphCoords(p.first);
	//	SphCoord sph(coords.x(), coords.y());

	//	double rad = altToAbs(SphericalVectorField::mbarsToMeters(coords.z()));
	//	r->verts.push_back(sph.toCartesian(rad));

	//	if (p.second == 1) {
	//		r->colours.push_back(glm::vec3(0.f, 1.f, 0.f));
	//	}
	//	else {
	//		r->colours.push_back(glm::vec3(1.f, 0.f, 0.f));
	//	}

	//	streamlines.push_back(r);
	//	objects.push_back(r);
	//	r->doubleToFloats();
	//	RenderEngine::assignBuffers(*r, false);
	//	RenderEngine::setBufferData(*r, false);
	//}

	//	Renderable* r = new Renderable();
	//	std::vector<Eigen::Vector3d> line = field.streamline(field.sphCoords(p.first), 100000.0, 25.0);

	//	for (const Eigen::Vector3d& p : line) {

	//		SphCoord sph(p.x(), p.y());
	//		double rad = altToAbs(SphericalVectorField::mbarsToMeters(p.z()));

	//		double speed = field.velocityAt(p).squaredNorm();
	//		double norm = speed / (70.0 * 70.0);
	//		if (norm > 1.0) norm = 1.0;

	//		r->verts.push_back(sph.toCartesian(rad));
	//		r->colours.push_back(glm::vec3(norm, 0.f, 0.f));
	//	}
	//	r->drawMode = GL_LINE_STRIP;
	//	streamlines.push_back(r);
	//	objects.push_back(r);
	//	r->doubleToFloats();
	//	RenderEngine::assignBuffers(*r, false);
	//	RenderEngine::setBufferData(*r, false);
	//}

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