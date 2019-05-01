#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include <vector>

class Renderable;


// Class for managine rendering to an SDL OpenGL window
// TODO better integration/communication with Renderable classes
class RenderEngine {

public:
	RenderEngine(SDL_Window* window, double cameraDist);

	void render(const std::vector<Renderable*>& objects, const glm::dmat4& view, float dTimeS);

	void setWindowSize(int newWidth, int newHeight);
	void updateScaleFactor(int dir);
	void updatePlanes(double cameraDist);

	double getFovY() const { return fovYRad; }
	double getAspectRatio() const { return (float)width/height; }
	double getNear() const { return near; }
	double getFar() const { return far; }
	const glm::dmat4& getProjection() const { return projection; }

	void ImGui();
	
private:
	SDL_Window* window;
	int width, height;

	double fovYRad;
	double near;
	double far;

	float totalTime;
	float timeMultiplier;
	float timeRepeat;
	float alphaPerSecond;
	bool specular;
	bool diffuse;
	bool colourScale;
	bool pause;

	GLuint mainProgram;
	GLuint streamlineProgram;
	float scaleFactor;

	glm::dmat4 projection;
};

