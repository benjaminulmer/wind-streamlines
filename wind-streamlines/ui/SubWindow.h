#pragma once

#include "Frustum.h"
#include "rendering/Camera.h"
#include "rendering/RenderEngine.h"
#include "ui/EarthViewController.h"

class Window;

#include <vector>


class SubWindow {

public:
	SubWindow(const Window& window, int x, int y, int width, int height);

	//virtual void setSize() = 0;
	//virtual void setPos() = 0;

	virtual void render(const std::vector<Renderable*>& objects, float dTimeS);

	Frustum getFrustum() { return Frustum(camera, renderEngine); }
	double getCameraDist() { return camera.getDist(); }

protected:
	const Window& window;
	Camera camera;
	RenderEngine renderEngine;
	EarthViewController evc;

	int x, y;
	int width, height;

	std::vector<SubWindow*> children;
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