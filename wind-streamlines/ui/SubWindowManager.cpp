#include "SubWindowManager.h"

#include "EarthViewController.h"
#include "SubWindow.h"
#include "rendering/Window.h"


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


bool SubWindowManager::handleMouseMove(int oldX, int newX, int oldY, int newY, unsigned int buttonMask) {
	
}