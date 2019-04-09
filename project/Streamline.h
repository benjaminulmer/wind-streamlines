#pragma once

#include <Eigen/Dense>

#include <vector>

#include "Renderable.h"
class SphericalVectorField;


// Class for storing and representing a single streamline
class Streamline {

public:
	Streamline(const SphericalVectorField& field);
	Streamline(const Streamline& back, const Streamline& forw, const SphericalVectorField& field);

	void addPoint(const Eigen::Vector3d& p);
	const std::vector<Eigen::Vector3d>& getPoints() const { return points; }
	size_t size() const { return points.size(); }

	double getTotalLength() const { return totalLength; }
	double getTotalAngle() const { return totalAngle; }

	void addToRenderable(StreamlineRenderable& r) const;

	const Eigen::Vector3d& operator[](size_t i) const { return points[i]; }

private:
	std::vector<Eigen::Vector3d> points;

	double totalLength;
	double totalAngle;

	const SphericalVectorField& field;
};

