#include "SubWindow.h"


SubWindow::SubWindow(const Window& window, int x, int y, int width, int height) :
	window(window),
	camera(2371000.0),
	renderEngine(window, x, y, width, height, 2371000.0),
	evc(camera, renderEngine, 2371000.0),
	x(x),
	y(y),
	width(width),
	height(height) {}


void SubWindow::render(const std::vector<Renderable*>& objects, float dTimeS) {
	renderEngine.clearViewport();
	renderEngine.render(objects, camera.getLookAt(), dTimeS);
}


BaseSubWindow::BaseSubWindow(const Window& window, int x, int y, int width, int height, double middleLat, double middleLong) :
	SubWindow(window, x, y, width, height) {

	evc.updateLatRot(-middleLat);
	evc.updateLngRot(-middleLong);
}