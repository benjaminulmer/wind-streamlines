#include "InputHandler.h"

#include "imgui/imgui_impl_sdl.h"

#include "EarthViewController.h"
#include "Program.h"
#include "rendering/Camera.h"
#include "rendering/RenderEngine.h"


// Construct with reference to a camera, render engine, and main program
//
// camera - pointer to camera object that should be updated
// renderEngine - pointer to render engine object that should be updated
// program - pointer to main program object that should be updated
InputHandler::InputHandler(RenderEngine& renderEngine, EarthViewController& evc, SubWindowManager& swm, Program& program) :
	renderEngine(renderEngine),
	evc(evc),
	swm(swm),
	program(program),
	mouseOldX(0), 
	mouseOldY(0) {}


// Process a single event
//
// e - event to process
void InputHandler::pollEvent(SDL_Event& e) {

	// If Dear ImGUI wants the event don't handle it
	ImGui_ImplSDL2_ProcessEvent(&e);
	if (ImGui::GetIO().WantCaptureMouse && (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP ||
		                                    e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEWHEEL)) {
		return;
	}
	if (ImGui::GetIO().WantCaptureKeyboard && (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)) {
		return;
	}

	SDL_Keymod mods = SDL_GetModState();

	switch (e.type) {
	case SDL_KEYDOWN:
		keyDownSwitch(e.key);
		break;

	case SDL_MOUSEMOTION: {

		switch (clicked) {
		case Clicked::MAIN_VIEW_ROTATE:
			evc.updateRotation(e.motion);
			break;

		case Clicked::MAIN_VIEW_HEADING_TILT:
			evc.updateHeadingAndTilt(e.motion);
			break;

		case Clicked::SUBWINDOW_ROTATE:
			swm.getActive().first->getEVC().updateRotation(e.motion);
			break;

		case Clicked::SUBWINDOW_HEADING_TILT:
			swm.getActive().first->getEVC().updateHeadingAndTilt(e.motion);
			break;

		case Clicked::SUBWINDOW_MOVE:
			swm.move(e.motion.x - e.motion.xrel, e.motion.y - e.motion.yrel, e.motion.x, e.motion.y);
			break;

		case Clicked::SUBWINDOW_RESIZE:
			swm.resize(e.motion.x - e.motion.xrel, e.motion.y - e.motion.yrel, e.motion.x, e.motion.y);
			break;

		}

		mouseOldX = e.motion.x;
		mouseOldY = e.motion.y;
		break;
	}
	case SDL_MOUSEWHEEL: {

		auto result = swm.getHover(mouseOldX, mouseOldY, false);
		SubWindowMouseState state = result.second;

		if (SubWindow::stateInside(state)) {
			result.first->getEVC().updateCameraDist(e.wheel.y, mouseOldX, mouseOldY);
		}
		else {
			evc.updateCameraDist(e.wheel.y, mouseOldX, mouseOldY);
		}
		break;
	}
	case SDL_MOUSEBUTTONDOWN: {

		SDL_Keymod mods = SDL_GetModState();
		SubWindowMouseState state = swm.getHover(e.button.x, e.button.y, true).second;

		if (e.button.button == SDL_BUTTON_LEFT) {

			if (mods & KMOD_LCTRL) {
				if (!swm.deleteSubWindow()) {
					swm.createSubWindow(e.button.x, e.button.y);
				}
			}
			else if (mods & KMOD_LSHIFT) {
				if (SubWindow::stateInside(state)) {
					clicked = Clicked::SUBWINDOW_MOVE;
				}
			}
			else {
				if (state == SubWindowMouseState::INSIDE_EARTH) {
					clicked = Clicked::SUBWINDOW_ROTATE;
				}
				else if (SubWindow::stateResize(state)) {
					clicked = Clicked::SUBWINDOW_RESIZE;
				}
				else if (state == SubWindowMouseState::OUTSIDE && evc.raySphereIntersectFromPixel(e.button.x, e.button.y)) {
					clicked = Clicked::MAIN_VIEW_ROTATE;
				}
			}
		}
		else if (e.button.button == SDL_BUTTON_RIGHT) {
			if (SubWindow::stateInside(state)) {
				clicked = Clicked::SUBWINDOW_HEADING_TILT;
			}
			else {
				clicked = Clicked::MAIN_VIEW_HEADING_TILT;
			}
		}
		break;
	}
	case SDL_MOUSEBUTTONUP:
		clicked = Clicked::NONE;
		swm.resetActive();
		break;

	case SDL_WINDOWEVENT:
		if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
			renderEngine.setViewport(0, 0, e.window.data1, e.window.data2);
		}
		break;

	case SDL_QUIT:
		program.cleanup();
		break;
	}
}


// Handle key press
//
// e - keyboard event
void InputHandler::keyDownSwitch(SDL_KeyboardEvent& e) {
	
	auto key = e.keysym.sym;

	if (key == SDLK_i) {
		renderEngine.updateScaleFactor(1);
	}
	else if (key == SDLK_k) {
		renderEngine.updateScaleFactor(-1);
	}
	else if (key == SDLK_c) {
		evc.resetCameraTilt();
	}
}

void InputHandler::updateCursor() {

	int x, y;

	SDL_Keymod mods = SDL_GetModState();
	SDL_GetMouseState(&x, &y);
	swm.updateCursor(x, y, mods & KMOD_LSHIFT);
}
