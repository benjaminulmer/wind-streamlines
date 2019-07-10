#include "SubWindowManager.h"

#include "EarthViewController.h"
#include "SubWindow.h"
#include "rendering/Window.h"

#include <algorithm>


const SDL_SystemCursor SubWindowManager::stateToCurs[] =
	{SDL_SYSTEM_CURSOR_SIZEALL, SDL_SYSTEM_CURSOR_SIZENS, SDL_SYSTEM_CURSOR_SIZEWE, SDL_SYSTEM_CURSOR_SIZENS,
	SDL_SYSTEM_CURSOR_SIZEWE, SDL_SYSTEM_CURSOR_SIZENESW, SDL_SYSTEM_CURSOR_SIZENWSE, SDL_SYSTEM_CURSOR_SIZENESW,
	SDL_SYSTEM_CURSOR_SIZENWSE, SDL_SYSTEM_CURSOR_ARROW};


SubWindowManager::SubWindowManager(const Window& window, const EarthViewController& evc) : 
	window(window),
	evc(evc) {}


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


bool SubWindowManager::deleteSubWindow(int x, int y) {
	
	SubWindow* active = nullptr;
	SubWindowMouseState activeState = SubWindowMouseState::OUTSIDE;

	for (SubWindow* s : windows) {

		SubWindowMouseState state = s->testMousePos(x, y);
		if (state != SubWindowMouseState::OUTSIDE) {
			active = s;
			activeState = state;
		}
	}
	if (activeState == SubWindowMouseState::INSIDE) {

		windows.erase(std::remove(windows.begin(), windows.end(), active), windows.end());
		delete active;

		return true;
	}
	else {
		return false;
	}
}


bool SubWindowManager::handleMouseMove(int oldX, int newX, int oldY, int newY, unsigned int buttonMask) {
	
	SubWindow* active = nullptr;
	SubWindowMouseState activeState = SubWindowMouseState::OUTSIDE;

	for (SubWindow* s : windows) {

		SubWindowMouseState state = s->testMousePos(oldX, oldY);
		if (state != SubWindowMouseState::OUTSIDE) {
			active = s;
			activeState = state;
		}
	}
	SDL_Cursor* curs = SDL_CreateSystemCursor(stateToCurs[(int)activeState]);
	SDL_SetCursor(curs);

	if (active != nullptr) {
		if (buttonMask & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			handleSubWindowState(activeState, active, newX - oldX, newY - oldY);
		}
		return true;
	}
	else {
		return false;
	}
}


void SubWindowManager::handleSubWindowState(SubWindowMouseState state, SubWindow* sw, int dx, int dy) {

	switch (state) {
	case SubWindowMouseState::TOP:
		sw->expandUp(-dy);
		break;
	case SubWindowMouseState::BOTTOM:
		sw->expandDown(dy);
		break;
	case SubWindowMouseState::LEFT:
		sw->expandLeft(-dx);
		break;
	case SubWindowMouseState::RIGHT:
		sw->expandRight(dx);
		break;
	case SubWindowMouseState::TOP_LEFT:
		sw->expandUp(-dy);
		sw->expandLeft(-dx);
		break;
	case SubWindowMouseState::BOTTOM_RIGHT:
		sw->expandDown(dy);
		sw->expandRight(dx);
		break;
	case SubWindowMouseState::TOP_RIGHT:
		sw->expandUp(-dy);
		sw->expandRight(dx);
		break;
	case SubWindowMouseState::BOTTOM_LEFT:
		sw->expandDown(dy);
		sw->expandLeft(-dx);
		break;
	case SubWindowMouseState::INSIDE:
		sw->move(dx, dy);
		break;
	}
}