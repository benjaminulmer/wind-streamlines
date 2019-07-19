#pragma once

class Camera;
class EarthViewController;
class RenderEngine;
class SubWindowManager;
class Program;

#include <SDL2/SDL.h>


// Enum for what should happen based on latest click event
enum class Clicked {
	NONE,
	MAIN_VIEW_ROTATE,
	MAIN_VIEW_HEADING_TILT,
	SUBWINDOW_ROTATE,
	SUBWINDOW_HEADING_TILT,
	SUBWINDOW_RESIZE,
	SUBWINDOW_MOVE
};


// Class for processing inputs and updating state
class InputHandler {

public:
	InputHandler(RenderEngine& renderEngine, EarthViewController& evc, SubWindowManager& swm, Program& program);

	void pollEvent(SDL_Event& e);
	void keyDown(SDL_KeyboardEvent& e);
	void mouseMotion(SDL_MouseMotionEvent& e);
	void mouseWheel(SDL_MouseWheelEvent& e);
	void mouseDown(SDL_MouseButtonEvent& e);

	void updateCursor();

private: 
	RenderEngine& renderEngine;
	EarthViewController& evc;
	SubWindowManager& swm;
	Program& program;

	Clicked clicked;

	int mouseOldX;
	int mouseOldY;
};
