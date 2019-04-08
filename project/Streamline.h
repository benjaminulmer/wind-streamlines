#pragma once

#include <Eigen/Dense>

#include <vector>

#include "Renderable.h"
class SphericalVectorField;


// Class for storing and representing a single streamline
class Streamline {

public:
	Streamline(double totalTime, size_t numPoints, const SphericalVectorField& field);

	void addPoint(const Eigen::Vector3d& p);
	const std::vector<Eigen::Vector3d>& getPoints() const { return points; }
	size_t size() const { return _size; }

	double getTotalTime() const { return totalTime; }
	double getTotalLength() const { return totalLength; }
	double getTotalAngle() const { return totalAngle; }

	void addToRenderable(Renderable& r, double d) const;

	const Eigen::Vector3d& operator[](size_t i) const { return points[i]; }

private:
	std::vector<Eigen::Vector3d> points;
	size_t _size;

	double totalTime;
	double totalLength;
	double totalAngle;

	const SphericalVectorField& field;
};

