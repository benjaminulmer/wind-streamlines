#pragma once

class Camera;
class EarthViewController;
class RenderEngine;
class Program;

#include <SDL2/SDL.h>


// Class for processing inputs and updating state
class InputHandler {

public:
	InputHandler(Camera& camera, RenderEngine& renderEngine, EarthViewController& evc, Program& program);

	void pollEvent(SDL_Event& e);

	void key(SDL_KeyboardEvent& e);
	void motion(SDL_MouseMotionEvent& e);
	void scroll(SDL_MouseWheelEvent& e);
	void reshape(SDL_WindowEvent& e);

private: 
	Camera& camera;
	RenderEngine& renderEngine;
	EarthViewController& evc;
	Program& program;

	int mouseOldX;
	int mouseOldY;
};
