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
	void keyDownSwitch(SDL_KeyboardEvent& e);

private: 
	Camera& camera;
	RenderEngine& renderEngine;
	EarthViewController& evc;
	Program& program;

	int mouseOldX;
	int mouseOldY;
};
