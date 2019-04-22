#define _USE_MATH_DEFINES
#include "VoxelGrid.h"

#include "Conversions.h"


// Create voxel grid for a size and seperation distance
//
// rad - radius (half width) of grid
// sepDist - distance between cell centres i.e. cell width
VoxelGrid::VoxelGrid(double rad, double sepDist) :
	rad(rad),
	sepDist(sepDist),
	numCells((size_t)((rad * 2.0) / sepDist) + 1),
	grid(numCells * numCells) {}


// Adds point to the grid
//
// p - point to add in cartesian
void VoxelGrid::addPoint(const Eigen::Vector3d& p) {

	size_t x = (size_t)((p.x() + rad) / sepDist);
	size_t y = (size_t)((p.y() + rad) / sepDist);
	size_t z = (size_t)((p.z() + rad) / sepDist);

	grid[indexToOffset(x, y, z)].push_back(p);
}


// Test if point is within seperation distance of other points in grid
//
// p - point to test in cartesian
// return - false if point is within sepDist of another point, otherwise true
bool VoxelGrid::testPoint(const Eigen::Vector3d& p) const {

	size_t xM1 = (size_t)((p.x() + rad) / sepDist) - 1;
	size_t yM1 = (size_t)((p.y() + rad) / sepDist) - 1;
	size_t zM1 = (size_t)((p.z() + rad) / sepDist) - 1;

	for (size_t xI = 0; xI < 3; xI++) {
		for (size_t yI = 0; yI < 3; yI++) {
			for (size_t zI = 0; zI < 3; zI++) {

				size_t x = xM1 + xI;
				size_t y = yM1 + yI;
				size_t z = zM1 + zI;

				if (x < 0 || y < 0 || z < 0 ||
					x > numCells - 1 || y > numCells - 1 || z > numCells - 1) {

					continue;
				}

				if (grid.find(indexToOffset(x, y, z)) != grid.end()) {
					const auto cell = grid.at(indexToOffset(x, y, z));
					for (const Eigen::Vector3d t : cell) {

						double pLen = p.norm();
						double tLen = t.norm();

						double height = std::min(pLen, tLen);

						double vert = pLen - tLen;
						double geod = height * acos((p / pLen).dot(t / tLen));

						if (geod * geod + RADIAL_DIST_SCALE * RADIAL_DIST_SCALE * vert * vert < sepDist * sepDist) {
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}


// Convert x, y, z index to absolute index
//
// x - x index
// y - y index
// z - z index
// return - absolute 1D index
size_t VoxelGrid::indexToOffset(size_t x, size_t y, size_t z) const {
	return y + numCells * (x + numCells * z);
}
