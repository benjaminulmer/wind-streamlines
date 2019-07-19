#pragma once

#include "SubWindow.h"

class EarthViewController;
class Renderable;
class SeedingEngine;
class Window;

#include <vector>


// Class for managing all subwindows that are active in a window, including creating a destroying them
class SubWindowManager {

public:
	SubWindowManager(const Window& window, const EarthViewController& evc);

	void renderAll(SeedingEngine& seeder, const std::vector<Renderable*>& objects, float dTimeS);

	bool createSubWindow(int x, int y);
	bool deleteSubWindow();

	std::pair<SubWindow*, SubWindowMouseState> getHover(int x, int y, bool active);

	void move(int oldX, int oldY, int newX, int newY);
	void resize(int oldX, int oldY, int newX, int newY);
	std::pair<SubWindow*, SubWindowMouseState> getActive();

	void resetActive();

	void updateCursor(int x, int y, bool move);

private:
	static const SDL_SystemCursor stateToCurs[];

	const Window& window;
	const EarthViewController& evc;

	std::vector<SubWindow*> windows;

	SubWindow* activeWindow;
	SubWindowMouseState activeState;
};