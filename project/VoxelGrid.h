#pragma once

#include <Eigen/Dense>

#include <vector>

class VoxelGrid {

public:
	VoxelGrid(double rad, double sepDist);

	void addPoint(const Eigen::Vector3d& p);
	bool testPoint(const Eigen::Vector3d& p) const;

	size_t indexToOffset(size_t x, size_t y, size_t z) const;

	std::vector<Eigen::Vector3d>& operator()(size_t x, size_t y, size_t z);
	const std::vector<Eigen::Vector3d>& operator()(size_t x, size_t y, size_t z) const;

private:
	double rad;
	double sepDist;
	size_t numCells;

	std::vector<std::vector<Eigen::Vector3d>> grid;
};

