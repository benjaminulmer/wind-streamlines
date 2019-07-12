#include "SubWindow.h"

#include "rendering/Window.h"


SubWindow::SubWindow(const Window& window, int x, int y, int width, int height) :
	window(window),
	camera(2371000.0),
	renderEngine(window, x, window.getHeight() - (y + height), width, height, 2371000.0),
	evc(camera, renderEngine, 2371000.0),
	x(x),
	y(y),
	width(width),
	height(height) {}


void SubWindow::render(const std::vector<Renderable*>& objects, float dTimeS) {
	renderEngine.clearViewport();
	renderEngine.render(objects, camera.getLookAt(), dTimeS);
}


SubWindowMouseState SubWindow::testMousePos(int _x, int _y) {

	int top = y - selectionBuffer;
	int left = x - selectionBuffer;

	if (_x >= left && _x <= left + width + selectionBuffer * 2 &&
		_y >= top && _y <= top + height + selectionBuffer * 2) {

		if (_y < y) {
			if (_x < x + selectionBuffer) {
				return SubWindowMouseState::TOP_LEFT;
			}
			else if (_x > x + width - selectionBuffer) {
				return SubWindowMouseState::TOP_RIGHT;
			}
			else {
				return SubWindowMouseState::TOP;
			}
		}
		else if (_y > y + height) {
			if (_x < x + selectionBuffer) {
				return SubWindowMouseState::BOTTOM_LEFT;
			}
			else if (_x > x + width - selectionBuffer) {
				return SubWindowMouseState::BOTTOM_RIGHT;
			}
			else {
				return SubWindowMouseState::BOTTOM;
			}
		}
		else if (_x < x) {
			if (_y < y + selectionBuffer) {
				return SubWindowMouseState::TOP_LEFT;
			}
			else if (_y > y + height - selectionBuffer) {
				return SubWindowMouseState::BOTTOM_LEFT;
			}
			else {
				return SubWindowMouseState::LEFT;
			}
		}
		else if (_x > x + width) {
			if (_y < y + selectionBuffer) {
				return SubWindowMouseState::TOP_RIGHT;
			}
			else if (_y > y + height - selectionBuffer) {
				return SubWindowMouseState::BOTTOM_RIGHT;
			}
			else {
				return SubWindowMouseState::RIGHT;
			}
		}
		else {
			if (evc.raySphereIntersectFromPixel(_x, _y)) {
				return SubWindowMouseState::INSIDE_EARTH;
			}
			else {
				return SubWindowMouseState::INSIDE_NOEARTH;
			}
		}
	}
	else {
		return SubWindowMouseState::OUTSIDE;
	}

	return SubWindowMouseState();
}


void SubWindow::move(int dx, int dy) {
	x += dx;
	y += dy;
	updateViewport();
}


void SubWindow::expandLeft(int amount) {
	x -= amount;
	width += amount;
	updateViewport();
}


void SubWindow::expandRight(int amount) {
	width += amount;
	updateViewport();
}


void SubWindow::expandUp(int amount) {
	y -= amount;
	height += amount;
	updateViewport();
}


void SubWindow::expandDown(int amount) {
	height += amount;
	updateViewport();
}


void SubWindow::updateViewport() {
	if (width <= 10) width = 10;
	if (height <= 10) height = 10;

	renderEngine.setViewport(x, window.getHeight() - (y + height), width, height);
}


BaseSubWindow::BaseSubWindow(const Window& window, int x, int y, int width, int height, double middleLat, double middleLong) :
	SubWindow(window, x, y, width, height) {

	evc.updateLatRot(-middleLat);
	evc.updateLngRot(-middleLong);
}