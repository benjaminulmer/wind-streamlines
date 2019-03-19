#pragma once

#include <Eigen/Dense>
#include <netcdf>

class SphericalVectorField {

public:
	SphericalVectorField(const netCDF::NcFile& file);

	Eigen::Vector3d& operator()(size_t level, size_t lat, size_t lng);
	const Eigen::Vector3d& operator()(size_t level, size_t lat, size_t lng) const;

	size_t numLevels() { return levels.size(); }
	size_t numLats() { return lats.size	(); }
	size_t numLongs() { return longs.size(); }

private:
	std::vector<Eigen::Vector3d> data;

	std::vector<double> levels;
	std::vector<double> lats;
	std::vector<double> longs;
};

