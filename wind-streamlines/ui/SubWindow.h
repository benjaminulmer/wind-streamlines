#pragma once

#include "Frustum.h"
#include "rendering/Camera.h"
#include "rendering/RenderEngine.h"
#include "ui/EarthViewController.h"

class Window;

#include <vector>


enum class SubWindowMouseState {
	INSIDE,
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


class SubWindow {

public:
	SubWindow(const Window& window, int x, int y, int width, int height);

	virtual void render(const std::vector<Renderable*>& objects , float dTimeS);

	Frustum getFrustum() const { return Frustum(camera, renderEngine); }
	double getCameraDist() const { return camera.getDist(); }

	SubWindowMouseState testMousePos(int _x, int _y);
	void move(int dx, int dy);
	void expandLeft(int amount);
	void expandRight(int amount);
	void expandUp(int amount);
	void expandDown(int amount);

protected:
	static constexpr int selectionBuffer = 5;

	const Window& window;
	Camera camera;
	RenderEngine renderEngine;
	EarthViewController evc;

	int x, y;
	int width, height;

	std::vector<SubWindow*> children;

	void updateViewport();
};


class BaseSubWindow : public SubWindow {

public:
	BaseSubWindow(const Window& window, int x, int y, int width, int height, double middleLat, double middleLong);

private:

};


class ChildSubWindow : public SubWindow {

public:
	ChildSubWindow(const SubWindow& window, int x, int y, int width, int height, double parentXNorm, double parentYNorm);

private:
	const SubWindow& parent;
	
	double parentXNorm, parentYNorm;
};