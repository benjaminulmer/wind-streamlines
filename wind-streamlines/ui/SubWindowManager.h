#pragma once

#include "SubWindow.h"

class EarthViewController;
class Renderable;
class Window;

#include <vector>


class SubWindowManager {

public:
	SubWindowManager(const Window& window, const EarthViewController& evc);

	void renderAll(const std::vector<Renderable*>& objects, float dTimeS);

	bool createSubWindow(int x, int y);
	bool deleteSubWindow(int x, int y);
	bool handleMouseMove(int oldX, int newX, int oldY, int newY, unsigned int buttonMask);

private:
	static const SDL_SystemCursor stateToCurs[];

	const Window& window;
	const EarthViewController& evc;

	std::vector<SubWindow*> windows;

	void handleSubWindowState(SubWindowMouseState state, SubWindow* sw, int dx, int dy);
};