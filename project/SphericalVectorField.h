#pragma once

#include "Streamline.h"

#include <Eigen/Dense>
#include <netcdf>


// Class for managing spherical vector field on Earth
// TODO currently hard-coded for specific grid format
class SphericalVectorField {

public:
	static const size_t NUM_LEVELS = 37;
	static const size_t NUM_LATS = 721;
	static const size_t NUM_LONGS = 1440;

	bool param = true;

	SphericalVectorField() = default;
	SphericalVectorField(const netCDF::NcFile& file);

	double getMaxMagSq() const { return maxMagSq; }
	int getMaxLevel() const { return levels[NUM_LEVELS - 1]; }
	int getMinLevel() const { return levels[0]; }

	std::vector<std::pair<Eigen::Matrix<size_t, 3, 1>, int>> findCriticalPoints() const;

	Streamline streamline(const Eigen::Vector3d& seed, double totalTime, double tol, double maxStep) const;
	Eigen::Vector3d velocityAt(const Eigen::Vector3d& pos) const;
	Eigen::Vector3d velocityAtM(const Eigen::Vector3d& pos) const;

	Eigen::Vector3d sphCoords(size_t i) const;
	Eigen::Vector3d sphCoords(size_t lat, size_t lng, size_t lvl) const;
	Eigen::Vector3d sphCoords(const Eigen::Matrix<size_t, 3, 1>& i) const;

	Eigen::Matrix<size_t, 3, 1> offsetToIndex(size_t i) const;
	size_t indexToOffset(size_t lat, size_t lng, size_t lvl) const;
	size_t indexToOffset(const Eigen::Matrix<size_t, 3, 1>& i) const;

	Eigen::Vector3d& operator()(size_t i);
	const Eigen::Vector3d& operator()(size_t i) const;

	Eigen::Vector3d& operator()(size_t lat, size_t lng, size_t lvl);
	const Eigen::Vector3d& operator()(size_t lat, size_t lng, size_t lvl) const;

	Eigen::Vector3d& operator()(const Eigen::Matrix<size_t, 3, 1>& i);
	const Eigen::Vector3d& operator()(const Eigen::Matrix<size_t, 3, 1>& i) const;

private:
	std::vector<Eigen::Vector3d> data;

	double maxMagSq;

	std::vector<int> levels;
	std::vector<double> lats;
	std::vector<double> longs;

	int signTet(const Eigen::Vector4d& v0, const Eigen::Vector4d& v1,
	            const Eigen::Vector4d& v2, const Eigen::Vector4d& v3,
	            size_t i0, size_t i1, size_t i2, size_t i3) const;

	int criticalPointInTet(size_t i0, size_t i1, size_t i2, size_t i3) const;
	Eigen::Vector3d newPos(const Eigen::Vector3d& currPos, const Eigen::Vector3d& velocity) const;
	Eigen::Vector3d RKF45Adaptive(const Eigen::Vector3d& currPos, double& timeStep, double tol, double maxStep) const;
};

