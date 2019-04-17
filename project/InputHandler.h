#pragma once

class Camera;
class RenderEngine;
class Program;

#include <SDL2/SDL.h>


// Class for processing inputs and updating state
// Does not process GUI inputs, those are done with Dear ImGUI
class InputHandler {

public:
	InputHandler(Camera* camera, RenderEngine* renderEngine, Program* program);

	void pollEvent(SDL_Event& e);

	void key(SDL_KeyboardEvent& e);
	void motion(SDL_MouseMotionEvent& e);
	void scroll(SDL_MouseWheelEvent& e);
	void reshape(SDL_WindowEvent& e);

private: 
	Camera* camera;
	RenderEngine* renderEngine;
	Program* program;

	int mouseOldX;
	int mouseOldY;
};
