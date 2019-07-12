#include "SubWindowManager.h"

#include "EarthViewController.h"
#include "SubWindow.h"
#include "rendering/Window.h"

#include <algorithm>


const SDL_SystemCursor SubWindowManager::stateToCurs[] = {
	SDL_SYSTEM_CURSOR_ARROW, SDL_SYSTEM_CURSOR_ARROW, SDL_SYSTEM_CURSOR_SIZENS, SDL_SYSTEM_CURSOR_SIZEWE,
	SDL_SYSTEM_CURSOR_SIZENS, SDL_SYSTEM_CURSOR_SIZEWE, SDL_SYSTEM_CURSOR_SIZENESW, SDL_SYSTEM_CURSOR_SIZENWSE,
	SDL_SYSTEM_CURSOR_SIZENESW, SDL_SYSTEM_CURSOR_SIZENWSE, SDL_SYSTEM_CURSOR_ARROW
};


SubWindowManager::SubWindowManager(const Window& window, const EarthViewController& evc) : 
	window(window),
	evc(evc),
	activeState(SubWindowMouseState::OUTSIDE),
	activeWindow(nullptr) {}


void SubWindowManager::renderAll(const std::vector<Renderable*>& objects, float dTimeS) {

	for (SubWindow* s : windows) {
		s->render(objects, dTimeS);
	}
}


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


bool SubWindowManager::deleteSubWindow() {
	

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

void SubWindowManager::move(int oldX, int oldY, int newX, int newY) {
	activeWindow->move(newX - oldX, newY - oldY);
}


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


std::pair<SubWindow*, SubWindowMouseState> SubWindowManager::getActive() {
	return std::pair<SubWindow*, SubWindowMouseState>(activeWindow, activeState);
}


void SubWindowManager::resetActive() {
	activeWindow = nullptr;
	activeState = SubWindowMouseState::OUTSIDE;
}



#include <iostream>
void SubWindowManager::updateCursor(int x, int y, bool move) {

	SubWindowMouseState state = (activeWindow != nullptr) ? activeState : getHover(x, y, false).second;
	SDL_Cursor* curs = SDL_CreateSystemCursor(stateToCurs[(int)state]);

	if (SubWindow::stateInside(state) && move) {
		curs = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	}

	SDL_SetCursor(curs);
}


void SubWindowManager::handleSubWindowState(SubWindowMouseState state, SubWindow* sw, int dx, int dy) {


}