#pragma once

#include <Eigen/Dense>
#include <netcdf>

class SphericalVectorField {

public:
	static const int NUM_LEVELS = 37;
	static const int NUM_LATS = 721;
	static const int NUM_LONGS = 1440;

	SphericalVectorField() = default;
	SphericalVectorField(const netCDF::NcFile& file);

	std::vector<Eigen::Vector3i> findCriticalPoints();

	double mbToMeters(double mb) { return 0.3048 * 145366.45 * (1.0 - pow(mb / 1013.25, 0.190284)); }

	Eigen::Vector3d indexToCoords(size_t lvl, size_t lat, size_t lng) { return Eigen::Vector3d(lats[lat], longs[lng], mbToMeters(levels[lvl])); }
	Eigen::Vector3d indexToCoords(const Eigen::Vector3i& i) { return indexToCoords(i(0), i(1), i(2)); }

	Eigen::Vector3d& operator()(size_t lvl, size_t lat, size_t lng);
	const Eigen::Vector3d& operator()(size_t lvl, size_t lat, size_t lng) const;

	Eigen::Vector3d& operator()(const Eigen::Vector3i& i) { return operator()(i(0), i(1), i(2)); }
	const Eigen::Vector3d& operator()(const Eigen::Vector3i& i) const { return operator()(i(0), i(1), i(2)); }

private:
	std::vector<Eigen::Vector3d> data;

	std::vector<int> levels;
	std::vector<float> lats;
	std::vector<float> longs;
};

