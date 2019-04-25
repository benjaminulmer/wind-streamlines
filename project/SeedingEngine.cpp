#define _USE_MATH_DEFINES
#include "SeedingEngine.h"

#include "Conversions.h"
#include "Frustum.h"
#include "SphericalVectorField.h"
#include "VoxelGrid.h"

#include <imgui.h>

#include <queue>
#include <random>

void SeedingEngine::ImGui() {
	ImGui::Text("%i streamlines", streamlines[0].size());
	ImGui::Text("%i streamlines in view", prevNum);
	//ImGui::Text("%i streamline vertices", streamlineRender.size());
}


SeedingEngine::SeedingEngine(SphericalVectorField & field) :
	field(field) {}


void SeedingEngine::seedGlobal() {

	streamlines.push_back(std::vector<Streamline>());

	double minLength = 1000000.0;
	double sepDist = 200000.0;

	VoxelGrid vg(mbarsToAbs(1.0) + 100.0, sepDist);
	std::queue<Streamline> seedLines;

	// Need a starting streamline to seed off of
	Streamline first = field.streamline(Eigen::Vector3d(0.0, 0.0, 999.0), 10000000.0, 1000.0, 10000.0, vg);
	for (const Eigen::Vector3d& p : first.getPoints()) {
		vg.addPoint(sphToCart(p));
	}
	seedLines.push(first);
	StreamlineRenderable* r = new StreamlineRenderable();
	r->setDrawMode(GL_LINES);
	first.addToRenderable(*r);
	first.r = r;
	streamlines[0].push_back(first);
	//test.push_back(r);


	// Seed until you can't seed no more
	while (!seedLines.empty()) {

		Streamline seedLine = seedLines.front();
		seedLines.pop();

		std::vector<Eigen::Vector3d> seeds = seedLine.getSeeds(sepDist);

		for (const Eigen::Vector3d& seed : seeds) {

			// Do not use seed if it is too close to other lines
			if (!vg.testPoint(sphToCart(seed))) {
				continue;
			}

			// Integrate streamline and add it if it was long enough
			Streamline newLine = field.streamline(seed, 10000000.0, 1000.0, 10000.0, vg);
			if (newLine.getTotalLength() > minLength) {

				seedLines.push(newLine);
				for (const Eigen::Vector3d& p : newLine.getPoints()) {
					vg.addPoint(sphToCart(p));
				}

				StreamlineRenderable* r = new StreamlineRenderable();
				r->setDrawMode(GL_LINES);
				newLine.addToRenderable(*r);
				newLine.r = r;
				streamlines[0].push_back(newLine);
				//test.push_back(r);
			}
		}
	}
}


std::vector<Renderable*> SeedingEngine::getLinesToRender(const Frustum& f) const {
	

	std::vector<Renderable*> toReturn;

	for (const Streamline& s : streamlines[0]) {
		if (f.overlap(s.getPoints())) {
			toReturn.push_back(s.r);
		}
	}
	prevNum = toReturn.size();
	return toReturn;
}