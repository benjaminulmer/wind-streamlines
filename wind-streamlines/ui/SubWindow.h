#pragma once

#include "Frustum.h"
#include "rendering/Camera.h"
#include "rendering/RenderEngine.h"
#include "ui/EarthViewController.h"

class Window;

#include <vector>


// Enum for state of mouse in relation to subwindow
enum class SubWindowMouseState {
	INSIDE_EARTH,
	INSIDE_NOEARTH,
	TOP,
	RIGHT,
	BOTTOM,
	LEFT,
	TOP_RIGHT,
	BOTTOM_RIGHT,
	BOTTOM_LEFT,
	TOP_LEFT,
	OUTSIDE
};


// Abstract class for managing a subwindow. Has its own rendering engine and camera
class SubWindow {

public:
	SubWindow(const Window& window, int x, int y, int width, int height);

	virtual void render(const std::vector<Renderable*>& objects , float dTimeS);

	Frustum getFrustum() const { return Frustum(camera, renderEngine); }
	double getCameraDist() const { return camera.getDist(); }
	EarthViewController& getEVC() { return evc; }

	SubWindowMouseState testMousePos(int _x, int _y);
	void move(int dx, int dy);
	void expandLeft(int amount);
	void expandRight(int amount);
	void expandUp(int amount);
	void expandDown(int amount);

	static bool stateInside(SubWindowMouseState state) {
		return (state == SubWindowMouseState::INSIDE_EARTH || state == SubWindowMouseState::INSIDE_NOEARTH);
	}
	static bool stateResize(SubWindowMouseState state) {
		return (state != SubWindowMouseState::INSIDE_EARTH && state != SubWindowMouseState::INSIDE_NOEARTH && state != SubWindowMouseState::OUTSIDE);
	}

protected:
	static constexpr int selectionBuffer = 10;

	const Window& window;
	Camera camera;
	RenderEngine renderEngine;
	EarthViewController evc;

	int x, y;
	int width, height;

	std::vector<SubWindow*> children;

	void updateViewport();
};


// Base level subwindow that references the main Earth reference
class BaseSubWindow : public SubWindow {

public:
	BaseSubWindow(const Window& window, int x, int y, int width, int height, double middleLat, double middleLong);

private:

};


// Child subwindow that references another subwindow. Not currently implemented or used
class ChildSubWindow : public SubWindow {

public:
	ChildSubWindow(const SubWindow& window, int x, int y, int width, int height, double parentXNorm, double parentYNorm);

private:
	const SubWindow& parent;
	
	double parentXNorm, parentYNorm;
};