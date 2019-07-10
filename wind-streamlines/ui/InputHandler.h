#pragma once

class Camera;
class EarthViewController;
class RenderEngine;
class SubWindowManager;
class Program;

#include <SDL2/SDL.h>


// Class for processing inputs and updating state
class InputHandler {

public:
	InputHandler(RenderEngine& renderEngine, EarthViewController& evc, SubWindowManager& swm, Program& program);

	void pollEvent(SDL_Event& e);
	void keyDownSwitch(SDL_KeyboardEvent& e);

private: 
	RenderEngine& renderEngine;
	EarthViewController& evc;
	SubWindowManager& swm;
	Program& program;

	int mouseOldX;
	int mouseOldY;
};
