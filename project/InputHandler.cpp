#include "InputHandler.h"

Camera* InputHandler::camera;
RenderEngine* InputHandler::renderEngine;
Program* InputHandler::program;
int InputHandler::mouseOldX;
int InputHandler::mouseOldY;
bool InputHandler::moved;

// Must be called before processing any SDL2 events
void InputHandler::setUp(Camera* camera, RenderEngine* renderEngine, Program* program) {
	InputHandler::camera = camera;
	InputHandler::renderEngine = renderEngine;
	InputHandler::program = program;
}

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

// Callback for key presses
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

// Callback for mouse button presses
void InputHandler::mouse(SDL_MouseButtonEvent& e) {
	mouseOldX = e.x;
	mouseOldY = e.y;
}

// Callback for mouse motion
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
	//program->setMousePos(iX, iY);

	mouseOldX = e.x;
	mouseOldY = e.y;
}

// Callback for mouse scroll
void InputHandler::scroll(SDL_MouseWheelEvent& e) {
	int dy;
	dy = e.x - e.y;

	const Uint8 *state = SDL_GetKeyboardState(0);
	if (state[SDL_SCANCODE_U]) {
		//program->updateRadialBounds(RadialBound::MAX, -dy);
	}
	else if (state[SDL_SCANCODE_J]) {
		//program->updateRadialBounds(RadialBound::MIN, -dy);
	}
	else if (state[SDL_SCANCODE_M]) {
		//program->updateRadialBounds(RadialBound::BOTH, -dy);
	}
	else {
		program->updateScale(-dy);
		//camera->updateZoom(dy);
	}
}

// Callback for window reshape/resize
void InputHandler::reshape(SDL_WindowEvent& e) {
	if (e.event == SDL_WINDOWEVENT_RESIZED) {
		renderEngine->setWindowSize(e.data1, e.data2);
		program->setWindowSize(e.data1, e.data2);
	}
}
