#include "InputHandler.h"

#include "Camera.h"
#include "RenderEngine.h"
#include "Program.h"


// Construct with reference to a camera, render engine, and main program
//
// camera - pointer to camera object that should be updated
// renderEngine - pointer to render engine object that should be updated
// program - pointer to main program object that should be updated
InputHandler::InputHandler(Camera* camera, RenderEngine* renderEngine, Program* program) : 
	camera(camera),
	renderEngine(renderEngine),
	program(program) {}


// Process a single event
//
// e - event to process
void InputHandler::pollEvent(SDL_Event& e) {
	if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
		InputHandler::key(e.key);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		moved = false;
	}
	else if (e.type == SDL_MOUSEBUTTONUP) {
		InputHandler::mouse(e.button);
	}
	else if (e.type == SDL_MOUSEMOTION) {
		InputHandler::motion(e.motion);
	}
	else if (e.type == SDL_MOUSEWHEEL) {
		InputHandler::scroll(e.wheel);
	}
	else if (e.type == SDL_WINDOWEVENT) {
		InputHandler::reshape(e.window);
	}
	else if (e.type == SDL_QUIT) {
		program->cleanup();
	}
}


// Handle key press
//
// e - keyboard event
void InputHandler::key(SDL_KeyboardEvent& e) {
	
	auto key = e.keysym.sym;

	if (e.state == SDL_PRESSED) {
		if (key == SDLK_f) {
			renderEngine->toggleFade();
		}
		else if (key == SDLK_i) {
			renderEngine->updateScaleFactor(1);
		}
		else if (key == SDLK_k) {
			renderEngine->updateScaleFactor(-1);
		}
		else if (key == SDLK_ESCAPE) {
			SDL_Quit();
			exit(0);
		}
	}
}


// Handle mouse button press
//
// e - mouse button event
void InputHandler::mouse(SDL_MouseButtonEvent& e) {
	mouseOldX = e.x;
	mouseOldY = e.y;
}


// Handle mouse motion event
//
// e - mouse motion event
void InputHandler::motion(SDL_MouseMotionEvent& e) {
	int dx, dy;
	dx = e.x - mouseOldX;
	dy = e.y - mouseOldY;

	// left mouse button moves camera
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		program->updateRotation(mouseOldX, e.x, mouseOldY, e.y, false);
	}
	else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
		program->updateRotation(mouseOldX, e.x, mouseOldY, e.y, true);
	}

	// Update current position of the mouse
	int width, height;
	SDL_Window* window = SDL_GetWindowFromID(e.windowID);
	SDL_GetWindowSize(window, &width, &height);

	int iX = e.x;
	int iY = height - e.y;

	mouseOldX = e.x;
	mouseOldY = e.y;
}


// Handle mouse scroll event
//
// e - mouse scroll event
void InputHandler::scroll(SDL_MouseWheelEvent& e) {
	program->updateScale(e.y);
}


// Handle resizing of window
//
// e - window event
void InputHandler::reshape(SDL_WindowEvent& e) {
	if (e.event == SDL_WINDOWEVENT_RESIZED) {
		renderEngine->setWindowSize(e.data1, e.data2);
		program->setWindowSize(e.data1, e.data2);
	}
}