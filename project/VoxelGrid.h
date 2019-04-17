#pragma once

#include <Eigen/Dense>

#include <unordered_map>


// Hashing voxel grid for simple spatial partitioning
// TODO better solution
class VoxelGrid {

public:
	VoxelGrid(double rad, double sepDist);

	void addPoint(const Eigen::Vector3d& p);
	bool testPoint(const Eigen::Vector3d& p) const;

	size_t indexToOffset(size_t x, size_t y, size_t z) const;

private:
	double rad;
	double sepDist;
	size_t numCells;

	std::unordered_map<size_t, std::vector<Eigen::Vector3d>> grid;
};

