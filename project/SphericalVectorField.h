#pragma once

#include <Eigen/Dense>
#include <netcdf>

class SphericalVectorField {

public:
	SphericalVectorField(const netCDF::NcFile& file);

	void loopOverCells();

	Eigen::Vector3d& operator()(size_t lvl, size_t lat, size_t lng);
	const Eigen::Vector3d& operator()(size_t lvl, size_t lat, size_t lng) const;

	const int NUM_LEVELS = 37;
	const int NUM_LATS = 721;
	const int NUM_LONGS = 1440;

private:
	std::vector<Eigen::Vector3d> data;

	std::vector<double> levels;
	std::vector<double> lats;
	std::vector<double> longs;
};

