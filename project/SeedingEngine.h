#pragma once

#include "Streamline.h"

class Frustum;
class SphericalVectorField;

#include <mutex>
#include <vector>


class SeedingEngine {

public:
	SeedingEngine(SphericalVectorField& field);

	void seedGlobal();
	std::vector<Renderable*> getLinesToRender(const Frustum& f, double cameraDist);

	void ImGui();

private:
	SphericalVectorField& field;
	std::vector<std::vector<Streamline>> streamlines;

	mutable int prevNum;
	bool globalDone;
};

