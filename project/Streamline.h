#pragma once

#include <Eigen/Dense>

#include <vector>

#include "Renderable.h"
class SphericalVectorField;

class Streamline {

public:
	Streamline(double totalTime, size_t numPoints);

	void addPoint(const Eigen::Vector3d& p);
	const std::vector<Eigen::Vector3d>& getPoints() const { return points; }
	size_t size() { return _size; }

	double getTotalTime() const { return totalTime; }
	double getTotalLength() const;
	double getTotalAngle() const;

	void addToRenderable(Renderable& r, const SphericalVectorField& field) const;

	const Eigen::Vector3d& operator[](size_t i) const { return points[i]; }

private:
	std::vector<Eigen::Vector3d> points;
	size_t _size;

	double totalTime;
	mutable double totalLength;
	mutable double totalAngle;

	void calculateLength() const;
	void calculateAngle() const;
};

