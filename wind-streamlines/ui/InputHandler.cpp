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
	case SDL_MOUSEMOTION:
		// before sending to evc see if subwindow manager wants it
		swm.handleMouseMove(mouseOldX, e.motion.x, mouseOldY, e.motion.y, e.motion.state);
		evc.updateRotation(mouseOldX, e.motion.x, mouseOldY, e.motion.y, e.motion.state);
		mouseOldX = e.motion.x;
		mouseOldY = e.motion.y;
		break;
	case SDL_MOUSEWHEEL:
		evc.updateCameraDist(e.wheel.y, mouseOldX, mouseOldY);
		break;
	case SDL_MOUSEBUTTONDOWN:
		if ((mods & KMOD_LSHIFT) == KMOD_LSHIFT) {
			swm.createSubWindow(e.button.x, e.button.y);
		}
		else if ((mods & KMOD_LCTRL) == KMOD_LCTRL) {
			swm.deleteSubWindow(e.button.x, e.button.y);
		}
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