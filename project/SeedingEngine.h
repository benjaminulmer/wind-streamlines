#pragma once

#include "Streamline.h"

class SphericalVectorField;

#include <mutex>
#include <vector>


class SeedingEngine {

public:
	SeedingEngine(SphericalVectorField& field);

	void seedGlobal();
	std::vector<Renderable*> getLinesToRender(int frustum) const;

	void ImGui();

private:
	SphericalVectorField& field;
	std::vector<std::vector<Streamline>> streamlines;

	std::vector<Renderable*> test;

	void mainLoop();
};

