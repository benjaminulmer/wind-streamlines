#pragma once

class Camera;
class EarthViewController;
class RenderEngine;
class SubWindowManager;
class Program;

#include <SDL2/SDL.h>


enum class Clicked {
	NONE,
	MAIN_VIEW_ROTATE,
	MAIN_VIEW_HEADING_TILT,
	SUBWINDOW_ROTATE,
	SUBWINDOW_HEADING_TILT,
	SUBWINDOW_RESIZE,
	SUBWINDOW_MOVE
};

enum class InputEvent {
	MAIN_VIEW_ROTATE,
	MAIN_VIEW_HEADING_TILT,
	MAIN_VIEW_ZOOM,
	SUBWINDOW_ROTATE,
	SUBWINDOW_HEADING_TILT,
	SUBWINDOW_ZOOM,
	SUBWINDOW_RESIZE,
	SUBWINDOW_MOVE
};

// Class for processing inputs and updating state
class InputHandler {

public:
	InputHandler(RenderEngine& renderEngine, EarthViewController& evc, SubWindowManager& swm, Program& program);

	void pollEvent(SDL_Event& e);
	void keyDownSwitch(SDL_KeyboardEvent& e);
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
