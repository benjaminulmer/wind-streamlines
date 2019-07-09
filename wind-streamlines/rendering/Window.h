#pragma once

#include <SDL2/SDL.h>


// Wrapper class for an SDL_Window and Dear ImGUI context for said window (OpenGL)
class Window {

public:
	Window(const char* title, int x, int y, int width, int height);
	~Window();

	void renderSetup();
	void finalizeRender();

	int getWidth() const;
	int getHeight() const;
	int getX() const;
	int getY() const;

private:
	SDL_Window* window;
	SDL_GLContext context;
};

