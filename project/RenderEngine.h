#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include <vector>

#include "Renderable.h"
#include "Camera.h"

class RenderEngine {

public:
	RenderEngine(SDL_Window* window);

	void render(const std::vector<const Renderable*>& objects, const glm::dmat4& view, float max, float min);

	static void assignBuffers(Renderable& renderable, bool texture);
	static void setBufferData(Renderable& renderable, bool texture);
	static void deleteBufferData(Renderable& renderable, bool texture);

	void setWindowSize(int newWidth, int newHeight);
	void toggleFade() { fade = !fade; }

	double getFovY() { return fovYRad; }
	double getAspectRatio() { return (float)width/height; }
	double getNear() { return near; }
	double getFar() { return far; }
	glm::dmat4 getProjection() { return projection; }

private:
	SDL_Window* window;
	int width, height;
	const double fovYRad = 60.f * ((float)3.14159265358979323846 / 180.f);
	const double near = 1.f;
	const double far = 1000.f;

	GLuint mainProgram;
	bool fade;

	glm::dmat4 projection;
};

