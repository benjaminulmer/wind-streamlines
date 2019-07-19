#include "SubWindow.h"

#include "rendering/Window.h"


// Creates subwindow of given dimensions attached to a parent window
//
// window - window the subchild is in
// x - subwindow left location
// y - subwindow upper location
// width - width of subwindow
// height - height of subwindow
SubWindow::SubWindow(const Window& window, int x, int y, int width, int height) :
	window(window),
	camera(2371000.0),
	renderEngine(window, x, window.getHeight() - (y + height), width, height, 2371000.0),
	evc(camera, renderEngine, 2371000.0),
	x(x),
	y(y),
	width(width),
	height(height) {}


// Render objects using own render pipeline
//
// objects - list of renderables to render
// dTimeS - time since last render in seconds
void SubWindow::render(const std::vector<Renderable*>& objects, float dTimeS) {
	renderEngine.clearViewport();
	renderEngine.render(objects, camera.getLookAt(), dTimeS);
}


// Returns the state of the provided mouse location relative to this subwindow
//
// x - x pixel location
// y - y pixel location
// return - appropriate mouse state
SubWindowMouseState SubWindow::testMousePos(int _x, int _y) {

	int top = y - selectionBuffer;
	int left = x - selectionBuffer;

	// Inside focus region of subwindow
	if (_x >= left && _x <= left + width + selectionBuffer * 2 &&
		_y >= top && _y <= top + height + selectionBuffer * 2) {

		// Top border region
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
		// Bottom border region
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
		// Left border region
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
		// Right border region
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
		// Inside window
		else {
			if (evc.raySphereIntersectFromPixel(_x, _y)) {
				return SubWindowMouseState::INSIDE_EARTH;
			}
			else {
				return SubWindowMouseState::INSIDE_NOEARTH;
			}
		}
	}
	// Outside
	else {
		return SubWindowMouseState::OUTSIDE;
	}
}


// Move subwindow provided amount
// 
// dx - pixels to move right
// dy pixels to move up
void SubWindow::move(int dx, int dy) {
	x += dx;
	y += dy;
	updateViewport();
}


// Expands the window to the left
//
// amount - number of pixels to grow
void SubWindow::expandLeft(int amount) {
	x -= amount;
	width += amount;
	updateViewport();
}


// Expands the window to the right
//
// amount - number of pixels to grow
void SubWindow::expandRight(int amount) {
	width += amount;
	updateViewport();
}


// Expands the window up
//
// amount - number of pixels to grow
void SubWindow::expandUp(int amount) {
	y -= amount;
	height += amount;
	updateViewport();
}


// Expands the window down
//
// amount - number of pixels to grow
void SubWindow::expandDown(int amount) {
	height += amount;
	updateViewport();
}


// Updates the viewport of the render engine based on current size and position
void SubWindow::updateViewport() {
	if (width <= 10) width = 10;
	if (height <= 10) height = 10;

	renderEngine.setViewport(x, window.getHeight() - (y + height), width, height);
}


// Create a subwindow centred on the provided lat long position
//
// window - window the subchild is in
// x - subwindow left location
// y - subwindow upper location
// width - width of subwindow
// height - height of subwindow
// middleLat - latitude to centre on in rads
// middleLong - longitude to centre on in rads
BaseSubWindow::BaseSubWindow(const Window& window, int x, int y, int width, int height, double middleLat, double middleLong) :
	SubWindow(window, x, y, width, height) {

	evc.updateLatRot(-middleLat);
	evc.updateLngRot(-middleLong);
}