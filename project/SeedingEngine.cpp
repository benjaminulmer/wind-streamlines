#include "SeedingEngine.h"

#include "Conversions.h"
#include "Frustum.h"
#include "SphericalVectorField.h"
#include "VoxelGrid.h"

#include <imgui.h>

#include <algorithm>
#include <queue>
#include <random>


// Dear ImGUI window. Slider for controlling multiscale
void SeedingEngine::ImGui() {
	if (ImGui::CollapsingHeader("Streamlines")) {
		ImGui::SliderInt("Show levels", &showLevels, 1, numLevels);
		updateCols = updateCols || ImGui::Checkbox("Second colour", &bothCols);
		updateCols = updateCols || ImGui::ColorEdit3("Colour 1", &col1.x);
		if (bothCols) {
			updateCols = updateCols || ImGui::ColorEdit3("Colour 2", &col2.x);
		}
	}
}


// Create engine for the provided vector field
//
// field - spherical vector field that will be seeded
SeedingEngine::SeedingEngine(SphericalVectorField & field) :
	field(field),
	numLevels(5),
	showLevels(1),
	updateCols(false),
	bothCols(true),
	col1(0.f, 0.f, 0.545f),
	col2(0.f, 1.f, 1.f) {}


// Seed streamlines
void SeedingEngine::seed() {

	double minLength = 1000000.0 * 1.25;
	double sepDist = 200000.0 * 1.25;

	// Multiresolution streamlines
	for (int i = 0; i < numLevels; i++) {

		streamlines.push_back(std::vector<Streamline>());

		minLength *= 0.8;
		sepDist *= 0.8;

		VoxelGrid vg(mbarsToAbs(1.0) + 100.0, sepDist);
		std::queue<Streamline> seedLines;

		// Need a starting streamline to seed off of
		if (i == 0) {
			Streamline first = field.streamline(Eigen::Vector3d(0.0, 1.0, 999.0), 10000000.0, 1000.0, 10000.0, vg);
			seedLines.push(first);
			first.createRenderable(col1, col2);
			streamlines[0].push_back(first);
		}

		// Put all streamline points in voxel grid
		for (int j = 0; j < i; j++) {
			for (Streamline& s : streamlines[j]) {
				for (const Eigen::Vector3d& p : s.getPoints()) {
					vg.addPoint(p);
				}
				seedLines.push(s);
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

					for (const Eigen::Vector3d& p : newLine.getPoints()) {
						vg.addPoint(p);
					}
					seedLines.push(newLine);
					newLine.createRenderable(col1, col2);
					streamlines[i].push_back(newLine);
				}
			}
		}
		std::cout << i << " done" << std::endl;
	}
}


// Get set of streamlines that should be rendered based on camera distance and view frustum
//
// f - view frustum for culling
// cameraDist - distance to camera for determining the resolution of lines to show (currently not used and this is done manually)
std::vector<Renderable*> SeedingEngine::getLinesToRender(const Frustum& f, double cameraDist) {

	if (updateCols) {

		for (std::vector<Streamline>& v : streamlines) {
			for (Streamline& s : v) {
				s.createRenderable(col1, (bothCols) ? col2 : col1);
			}
		}
		updateCols = false;
	}

	std::vector<Streamline> linesR;

	int i = 1;
	for (const std::vector<Streamline>& v : streamlines) {

		// Determine if this resolution of lines should be shown
		if (i <= showLevels) {
			for (const Streamline& s : v) {
				if (f.overlap(s.getPoints())) {
					linesR.push_back(s);
				}
			}
		}
		else {
			break;
		}
		i++;
	}

	// Sort for transparency. Not perfect but drastically reduces number of errors
	auto compAvgAlt = [](const Streamline& before, const Streamline& after) {
		double bAvgAlt = before.getSumAlt() / before.size();
		double aAvgAlt = after.getSumAlt() / after.size();

		return bAvgAlt > aAvgAlt;
	};
	std::sort(linesR.begin(), linesR.end(), compAvgAlt);

	// Make list of respective renderables
	std::vector<Renderable*> toReturn;
	for (Streamline& s : linesR) {
		toReturn.push_back(s.getRender());
	}

	return toReturn;
}