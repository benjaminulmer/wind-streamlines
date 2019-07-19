#include "SubWindowManager.h"

#include "EarthViewController.h"
#include "SubWindow.h"
#include "rendering/Window.h"
#include "streamlines/SeedingEngine.h"

#include <algorithm>


// Lookup for converting SubWindowMouseState to the appropriate system cursor
const SDL_SystemCursor SubWindowManager::stateToCurs[] = {
	SDL_SYSTEM_CURSOR_ARROW, SDL_SYSTEM_CURSOR_ARROW, SDL_SYSTEM_CURSOR_SIZENS, SDL_SYSTEM_CURSOR_SIZEWE,
	SDL_SYSTEM_CURSOR_SIZENS, SDL_SYSTEM_CURSOR_SIZEWE, SDL_SYSTEM_CURSOR_SIZENESW, SDL_SYSTEM_CURSOR_SIZENWSE,
	SDL_SYSTEM_CURSOR_SIZENESW, SDL_SYSTEM_CURSOR_SIZENWSE, SDL_SYSTEM_CURSOR_ARROW
};


// Create a manager for the provided window
//
// window - window to manage
// evc - earth view controller of main window. Used for ray intersection when creating subwindows
SubWindowManager::SubWindowManager(const Window& window, const EarthViewController& evc) : 
	window(window),
	evc(evc),
	activeState(SubWindowMouseState::OUTSIDE),
	activeWindow(nullptr) {}


// Render all active subwindows
//
// seeder - streamline seeding engine
// objects - other renderables to render that are not streamlines
// dTimeS - time since last render in seconds
void SubWindowManager::renderAll(SeedingEngine& seeder, const std::vector<Renderable*>& objects, float dTimeS) {

	std::vector<Renderable*> lines;

	for (SubWindow* s : windows) {

		lines = seeder.getLinesToRender(s->getFrustum(), s->getCameraDist());
		for (Renderable* r : objects) {
			lines.push_back(r);
		}

		s->render(lines , dTimeS);
	}
}


// Try to create a subwindow at the provided click location
//
// x - x pixel location
// y - y pixel location
// return - if subwindow was created or not
bool SubWindowManager::createSubWindow(int x, int y) {

	auto intersect = evc.raySphereIntersectFromPixel(x, y);
	if (intersect) {

		SubWindow* s = new BaseSubWindow(window, x, y, 200, 200, intersect->x, intersect->y);
		windows.push_back(s);

		return true;
	}
	else {
		return false;
	}
}


// Try to delete the active subwindow
//
// return - if a subwindow was deleted or not
bool SubWindowManager::deleteSubWindow() {
	
	// Delete active subwindow if there is one
	if (activeWindow != nullptr) {

		windows.erase(std::remove(windows.begin(), windows.end(), activeWindow), windows.end());
		delete activeWindow;

		resetActive();
		return true;
	}
	else {
		return false;
	}
}


// Get the subwindow and mouse state at the provided mouse location
//
// x - x pixel location
// y - y pixel location
// active - flag if the returned window should be made active or not
// return - pair of window and state at location. nullptr and OUTSIDE if there is none
std::pair<SubWindow*, SubWindowMouseState> SubWindowManager::getHover(int x, int y, bool active) {

	SubWindow* window = nullptr;
	SubWindowMouseState state = SubWindowMouseState::OUTSIDE;

	for (SubWindow* w : windows) {

		SubWindowMouseState s = w->testMousePos(x, y);
		if (s != SubWindowMouseState::OUTSIDE) {
			window = w;
			state = s;
		}
	}
	if (active) {
		activeWindow = window;
		activeState = state;
	}

	return std::pair<SubWindow*, SubWindowMouseState>(window, state);
}


// Move the active subwindow
//
// oldX - old pixel location x
// oldY - old pixel location y
// newX - new pixel location x
// newY - new pixel location y
void SubWindowManager::move(int oldX, int oldY, int newX, int newY) {
	activeWindow->move(newX - oldX, newY - oldY);
}


// Resize the active subwindow
//
// oldX - old pixel location x
// oldY - old pixel location y
// newX - new pixel location x
// newY - new pixel location y
void SubWindowManager::resize(int oldX, int oldY, int newX, int newY) {

	int dx = newX - oldX;
	int dy = newY - oldY;

	switch (activeState) {
	case SubWindowMouseState::TOP:
		activeWindow->expandUp(-dy);
		break;

	case SubWindowMouseState::BOTTOM:
		activeWindow->expandDown(dy);
		break;

	case SubWindowMouseState::LEFT:
		activeWindow->expandLeft(-dx);
		break;

	case SubWindowMouseState::RIGHT:
		activeWindow->expandRight(dx);
		break;

	case SubWindowMouseState::TOP_LEFT:
		activeWindow->expandUp(-dy);
		activeWindow->expandLeft(-dx);
		break;

	case SubWindowMouseState::BOTTOM_RIGHT:
		activeWindow->expandDown(dy);
		activeWindow->expandRight(dx);
		break;

	case SubWindowMouseState::TOP_RIGHT:
		activeWindow->expandUp(-dy);
		activeWindow->expandRight(dx);
		break;

	case SubWindowMouseState::BOTTOM_LEFT:
		activeWindow->expandDown(dy);
		activeWindow->expandLeft(-dx);
		break;
	}
}


// Return the active subwindow and its current mouse state
std::pair<SubWindow*, SubWindowMouseState> SubWindowManager::getActive() {
	return std::pair<SubWindow*, SubWindowMouseState>(activeWindow, activeState);
}


// Reset the active subwindow to be null
void SubWindowManager::resetActive() {
	activeWindow = nullptr;
	activeState = SubWindowMouseState::OUTSIDE;
}


// Update the cursor being displayed based on the current mouse position
//
// x - x pixel location
// y - y pixel location
// move - if the user is trying to move a subwindow
void SubWindowManager::updateCursor(int x, int y, bool move) {

	SubWindowMouseState state = (activeWindow != nullptr) ? activeState : getHover(x, y, false).second;
	SDL_Cursor* curs = SDL_CreateSystemCursor(stateToCurs[(int)state]);

	if (SubWindow::stateInside(state) && move) {
		curs = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	}

	SDL_SetCursor(curs);
}