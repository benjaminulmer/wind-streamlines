#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include <vector>

#include "Camera.h"
#include "Renderable.h"

class RenderEngine {

public:
	RenderEngine(SDL_Window* window);

	void render(const std::vector<const Renderable*>& objects, const glm::dmat4& view, float max, float min);

	static void assignBuffers(Renderable& renderable, bool texture);
	static void setBufferData(Renderable& renderable, bool texture);
	static void deleteBufferData(Renderable& renderable, bool texture);

	void setWindowSize(int newWidth, int newHeight);
	void toggleFade() { fade = !fade; }
	void updateScaleFactor(int dir);

	double getFovY() { return fovYRad; }
	double getAspectRatio() { return (float)width/height; }
	double getNear() { return near; }
	double getFar() { return far; }
	glm::dmat4 getProjection() { return projection; }

private:
	SDL_Window* window;
	int width, height;

	double fovYRad;
	double near;
	double far;

	GLuint mainProgram;
	bool fade;
	float scaleFactor;

	glm::dmat4 projection;
};

