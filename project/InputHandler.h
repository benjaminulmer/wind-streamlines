#pragma once

class Camera;
class RenderEngine;
class Program;

#include <SDL2/SDL.h>


class InputHandler {

public:
	InputHandler(Camera* camera, RenderEngine* renderEngine, Program* program);

	void pollEvent(SDL_Event& e);

	void key(SDL_KeyboardEvent& e);
	void mouse(SDL_MouseButtonEvent& e);
	void motion(SDL_MouseMotionEvent& e);
	void scroll(SDL_MouseWheelEvent& e);
	void reshape(SDL_WindowEvent& e);

private: 
	Camera* camera;
	RenderEngine* renderEngine;
	Program* program;

	int mouseOldX;
	int mouseOldY;

	bool moved;
};
