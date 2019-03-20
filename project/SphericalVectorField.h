#pragma once

#include <Eigen/Dense>
#include <netcdf>

class SphericalVectorField {

public:
	const int NUM_LEVELS = 37;
	const int NUM_LATS = 721;
	const int NUM_LONGS = 1440;

	SphericalVectorField(const netCDF::NcFile& file);

	std::vector<Eigen::Vector3i> findCriticalPoints();

	Eigen::Vector3d& operator()(size_t lvl, size_t lat, size_t lng);
	const Eigen::Vector3d& operator()(size_t lvl, size_t lat, size_t lng) const;

	Eigen::Vector3d& operator()(Eigen::Vector3i v) {
		return operator()(v(0), v(1), v(2));
	}
	const Eigen::Vector3d& operator()(Eigen::Vector3i v) const {
		return operator()(v(0), v(1), v(2));
	}

private:
	std::vector<Eigen::Vector3d> data;

	std::vector<int> levels;
	std::vector<float> lats;
	std::vector<float> longs;
};

