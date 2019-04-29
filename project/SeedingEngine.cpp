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
	ImGui::Text("Level %i", maxI);
	//ImGui::Text("%i streamline vertices", streamlineRender.size());
}


SeedingEngine::SeedingEngine(SphericalVectorField & field) :
	field(field),
	globalDone(false) {}


void SeedingEngine::seedGlobal() {

	double minLength = 1000000.0 * 1.25;
	double sepDist = 200000.0 * 1.25;

	for (int i = 0; i < 5; i++) {

		streamlines.push_back(std::vector<Streamline>());

		minLength *= 0.8;
		sepDist *= 0.8;

		VoxelGrid vg(mbarsToAbs(1.0) + 100.0, sepDist);
		std::queue<Streamline> seedLines;

		// Need a starting streamline to seed off of

		if (i == 0) {
			Streamline first = field.streamline(Eigen::Vector3d(0.0, 0.0, 999.0), 10000000.0, 1000.0, 10000.0, vg);
			for (const Eigen::Vector3d& p : first.getPoints()) {
				vg.addPoint(p);
			}
			seedLines.push(first);
			StreamlineRenderable* r = new StreamlineRenderable();
			r->setDrawMode(GL_LINES);
			first.addToRenderable(*r);
			first.r = r;
			streamlines[0].push_back(first);
		}
		else {
			for (int j = 0; j < i; j++) {
				for (Streamline& s : streamlines[j]) {
					for (const Eigen::Vector3d& p : s.getPoints()) {
						vg.addPoint(p);
					}
					seedLines.push(s);
				}
			}
		}


		// Seed until you can't seed no more
		while (!seedLines.empty()) {

			Streamline seedLine = seedLines.front();
			seedLines.pop();

			std::vector<Eigen::Vector3d> seeds = seedLine.getSeeds(sepDist);

			for (const Eigen::Vector3d& seed : seeds) {

				// Do not use seed if it is too close to other lines
				if (!vg.testPoint(seed)) {
					continue;
				}

				// Integrate streamline and add it if it was long enough
				Streamline newLine = field.streamline(cartToSph(seed), 10000000.0, 1000.0, 10000.0, vg);
				if (newLine.getTotalLength() > minLength) {

					seedLines.push(newLine);
					for (const Eigen::Vector3d& p : newLine.getPoints()) {
						vg.addPoint(p);
					}

					StreamlineRenderable* r = new StreamlineRenderable();
					r->setDrawMode(GL_LINES);
					newLine.addToRenderable(*r);
					newLine.r = r;
					streamlines[i].push_back(newLine);
				}
			}
		}
		std::cout << i << " done" << std::endl;
	}
	globalDone = true;
}


std::vector<Renderable*> SeedingEngine::getLinesToRender(const Frustum& f, double cameraDist) {

	std::vector<Streamline> linesR;

	double out = 7681094.0;
	double close = 498545.0;

	double step = (out - close) / 4.0;

	int i = 0;
	for (const std::vector<Streamline>& v : streamlines) {

		if (i == 0 || cameraDist < out - step * i) {
			for (const Streamline& s : v) {
				if (f.overlap(s.getPoints())) {
					linesR.push_back(s);
				}
			}
			prevNum = linesR.size();
			maxI = i;
		}
		else {
			break;
		}
		i++;
	}

	

	std::vector<Renderable*> toReturn;
	for (const Streamline& s : linesR) {
		toReturn.push_back(s.r);
	}

	return toReturn;
}