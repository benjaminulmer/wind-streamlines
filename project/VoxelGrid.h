#pragma once

#include <Eigen/Dense>

#include <unordered_map>


class VoxelGrid {

public:
	VoxelGrid(double rad, double sepDist);

	void addPoint(const Eigen::Vector3d& p);
	bool testPoint(const Eigen::Vector3d& p) const;

	size_t indexToOffset(size_t x, size_t y, size_t z) const;

	//void prints() const {
	//	int count = 0;
	//	int total = 0;
	//	for (const auto& c : grid) {
	//		if (c.size() > 0) {
	//			count++;
	//			total += c.size();
	//		}
	//	}
	//	std::cout << total / count << std::endl;
	//}

private:
	double rad;
	double sepDist;
	size_t numCells;

	std::unordered_map<size_t, std::vector<Eigen::Vector3d>> grid;
};

