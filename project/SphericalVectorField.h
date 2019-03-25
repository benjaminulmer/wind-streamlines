#pragma once

#include <Eigen/Dense>
#include <netcdf>


// Class for managing spherical vector field on Earth
// TODO currently hard-coded for specific grid format
class SphericalVectorField {

public:
	static const int NUM_LEVELS = 37;
	static const int NUM_LATS = 721;
	static const int NUM_LONGS = 1440;

	static double mbToMeters(double mb) { return 0.3048 * 145366.45 * (1.0 - pow(mb / 1013.25, 0.190284)); }

	SphericalVectorField() = default;
	SphericalVectorField(const netCDF::NcFile& file);

	std::vector<std::pair<Eigen::Vector3i, int>> findCriticalPoints();

	std::vector<Eigen::Vector3d> streamLine(const Eigen::Vector3d& seed);

	Eigen::Vector3d sphCoords(size_t i) const;
	Eigen::Vector3d sphCoords(size_t lat, size_t lng, size_t lvl) const;
	Eigen::Vector3d sphCoords(const Eigen::Vector3i& i) const;

	Eigen::Vector3i offsetToIndex(size_t i) const;
	size_t indexToOffset(size_t lat, size_t lng, size_t lvl) const;
	size_t indexToOffset(const Eigen::Vector3i& i) const;

	Eigen::Vector3d& operator()(size_t i);
	const Eigen::Vector3d& operator()(size_t i) const;

	Eigen::Vector3d& operator()(size_t lat, size_t lng, size_t lvl);
	const Eigen::Vector3d& operator()(size_t lat, size_t lng, size_t lvl) const;

	Eigen::Vector3d& operator()(const Eigen::Vector3i& i);
	const Eigen::Vector3d& operator()(const Eigen::Vector3i& i) const;

private:
	std::vector<Eigen::Vector3d> data;

	std::vector<int> levels;
	std::vector<float> lats;
	std::vector<float> longs;

	int sign(const Eigen::Vector4d& v0, const Eigen::Vector4d& v1,
	         const Eigen::Vector4d& v2, const Eigen::Vector4d& v3,
	         size_t i0, size_t i1, size_t i2, size_t i3);

	int criticalPointInTet(size_t i0, size_t i1, size_t i2, size_t i3);

	Eigen::Vector3d velocityAt(const Eigen::Vector3d& pos);
	Eigen::Vector3d newPos(const Eigen::Vector3d& currPos, const Eigen::Vector3d& velocity);
};

