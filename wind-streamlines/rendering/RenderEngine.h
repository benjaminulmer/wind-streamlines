#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include <vector>

class Renderable;
class Window;


// Class for managing and rendering to an SDL OpenGL window
// TODO better integration/communication with Renderable classes
class RenderEngine {

public:
	RenderEngine(const Window& window, double cameraDist);
	RenderEngine(const Window& window, int x, int y, int width, int height, double cameraDist);
 
	void clearViewport();
	void render(const std::vector<Renderable*>& objects, const glm::dmat4& view, float dTimeS);

	void setViewport(int newX, int newY, int newWidth, int newHeight);
	void updateScaleFactor(int dir);
	void updatePlanes(double cameraDist);

	std::pair<double, double> pixelToNormDevice(int _x, int _y);

	double getFovY() const { return fovYRad; }
	double getAspectRatio() const { return (float)width/height; }
	double getNear() const { return near; }
	double getFar() const { return far; }
	const glm::dmat4& getProjection() const { return projection; }

	void ImGui();
	
private:
	const Window& window;

	int x, y;
	int width, height;

	double fovYRad;
	double near;
	double far;

	float totalTime;
	float timeMultiplier;
	float timeRepeat;
	float alphaPerSecond;
	bool pause;

	bool specular;
	bool diffuse;

	float lineWidth;
	float outlineWidth;
	float scaleFactor;

	GLuint mainProgram;
	GLuint streamlineProgram;
	GLuint streamlineProgram2;

	glm::dmat4 projection;
};

