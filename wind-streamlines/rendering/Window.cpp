#include "Window.h"

#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>

#include <iostream>


// Creates and SDL_Window with provided parameters and sets up OpenGL, GLEW, and Dear ImGUI for said window
//
// title - title of the window
// x - x position
// y - y position
// width - width of window
// height - height of window
Window::Window(const char* title, int x, int y, int width, int height) :
	window(nullptr),
	context(nullptr) {

	// Set SDL GL attributes
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create window
	window = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		system("pause");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	// Create context
	context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
		system("pause");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	SDL_GL_SetSwapInterval(0); // Vsync on

	// Initialize OpenGL extensions 
	GLenum err = glewInit();
	if (glewInit() != GLEW_OK) {
		std::cerr << "glewInit error: " << glewGetErrorString(err) << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	// Setup Dear ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	const char* glsl_version = "#version 430 core";
	ImGui_ImplOpenGL3_Init(glsl_version);
}


// Sets up for rendering. Call before any rendering or Dear ImGUI calls
void Window::renderSetup() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
}


// Finalizes rendering. Draws Dear ImGUI frame and swaps buffers
void Window::finalizeRender() {
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}


// Returns current width of the window
int Window::getWidth() const {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	return w;
}


// Returns current height of the window
int Window::getHeight() const {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	return h;
}


// Returns current x position of window
int Window::getX() const {
	int x, y;
	SDL_GetWindowSize(window, &x, &y);
	return x;
}


// Returns current y position of window
int Window::getY() const {
	int x, y;
	SDL_GetWindowSize(window, &x, &y);
	return y;
}


Window::~Window() {
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
}