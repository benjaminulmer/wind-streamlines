#pragma once

class EarthViewController;
class Renderable;
class SubWindow;
class Window;

#include <vector>


class SubWindowManager {

public:
	SubWindowManager(const Window& window, const EarthViewController& evc);

	void renderAll(const std::vector<Renderable*>& objects, float dTimeS);

	bool createSubWindow(int x, int y);
	bool handleMouseMove(int oldX, int newX, int oldY, int newY, unsigned int buttonMask);

private:
	const Window& window;
	const EarthViewController& evc;

	std::vector<SubWindow*> windows;

};

