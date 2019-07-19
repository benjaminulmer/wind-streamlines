#include "InputHandler.h"

#include "imgui/imgui_impl_sdl.h"

#include "EarthViewController.h"
#include "Program.h"
#include "rendering/Camera.h"
#include "rendering/RenderEngine.h"


// Construct with reference to a camera, render engine, and main program
//
// renderEngine - render engine that should be updated
// evc - earth view controller that should be updated
// swm - subwindow manager that should be updated
// program - program that should be updated (only used for exiting application)
InputHandler::InputHandler(RenderEngine& renderEngine, EarthViewController& evc, SubWindowManager& swm, Program& program) :
	renderEngine(renderEngine),
	evc(evc),
	swm(swm),
	program(program),
	clicked(Clicked::NONE),
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

	switch (e.type) {
	case SDL_KEYDOWN:
		keyDown(e.key);
		break;

	case SDL_MOUSEMOTION:
		mouseMotion(e.motion);
		break;

	case SDL_MOUSEWHEEL:
		mouseWheel(e.wheel);
		break;

	case SDL_MOUSEBUTTONDOWN:
		mouseDown(e.button);
		break;

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
void InputHandler::keyDown(SDL_KeyboardEvent& e) {
	
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


// Handle mouse motion 
//
// e - mouse motion event
void InputHandler::mouseMotion(SDL_MouseMotionEvent& e) {

	switch (clicked) {
	case Clicked::MAIN_VIEW_ROTATE:
		evc.updateRotation(e);
		break;

	case Clicked::MAIN_VIEW_HEADING_TILT:
		evc.updateHeadingAndTilt(e);
		break;

	case Clicked::SUBWINDOW_ROTATE:
		swm.getActive().first->getEVC().updateRotation(e);
		break;

	case Clicked::SUBWINDOW_HEADING_TILT:
		swm.getActive().first->getEVC().updateHeadingAndTilt(e);
		break;

	case Clicked::SUBWINDOW_MOVE:
		swm.move(e.x - e.xrel, e.y - e.yrel, e.x, e.y);
		break;

	case Clicked::SUBWINDOW_RESIZE:
		swm.resize(e.x - e.xrel, e.y - e.yrel, e.x, e.y);
		break;

	}

	mouseOldX = e.x;
	mouseOldY = e.y;
}


// Handle mouse wheel event
//
// e - mouse wheel event
void InputHandler::mouseWheel(SDL_MouseWheelEvent& e) {
	
	auto result = swm.getHover(mouseOldX, mouseOldY, false);
	SubWindowMouseState state = result.second;

	if (SubWindow::stateInside(state)) {
		result.first->getEVC().updateCameraDist(e.y, mouseOldX, mouseOldY);
	}
	else {
		evc.updateCameraDist(e.y, mouseOldX, mouseOldY);
	}
}


// Handle mouse down event
//
// e - mouse down event
void InputHandler::mouseDown(SDL_MouseButtonEvent& e) {

	SDL_Keymod mods = SDL_GetModState();
	SubWindowMouseState state = swm.getHover(e.x, e.y, true).second;

	if (e.button == SDL_BUTTON_LEFT) {

		if (mods & KMOD_LCTRL) {
			if (!swm.deleteSubWindow()) {
				swm.createSubWindow(e.x, e.y);
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
			else if (state == SubWindowMouseState::OUTSIDE && evc.raySphereIntersectFromPixel(e.x, e.y)) {
				clicked = Clicked::MAIN_VIEW_ROTATE;
			}
		}
	}
	else if (e.button == SDL_BUTTON_RIGHT) {
		if (SubWindow::stateInside(state)) {
			clicked = Clicked::SUBWINDOW_HEADING_TILT;
		}
		else {
			clicked = Clicked::MAIN_VIEW_HEADING_TILT;
		}
	}
}


// Update which cursor is being displayed
void InputHandler::updateCursor() {

	int x, y;

	SDL_Keymod mods = SDL_GetModState();
	SDL_GetMouseState(&x, &y);
	swm.updateCursor(x, y, mods & KMOD_LSHIFT);
}
