#pragma once

#include "Streamline.h"

class Frustum;
class SphericalVectorField;

#include <mutex>
#include <vector>


// Class for seeding streamlines in a vector field
class SeedingEngine {

public:
	SeedingEngine(SphericalVectorField& field);

	void seed();
	std::vector<Renderable*> getLinesToRender(const Frustum& f, double cameraDist);

	void ImGui();

private:
	SphericalVectorField& field;
	std::vector<std::vector<Streamline>> streamlines;

	int numLevels;
	int showLevels;

	bool updateCols;
	bool bothCols;
	glm::vec3 col1;
	glm::vec3 col2;
};

