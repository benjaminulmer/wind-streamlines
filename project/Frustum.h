#pragma once

class Camera;
class RenderEngine;

#include <Eigen/Dense>

#include <vector>


// Class for representing and testing against a view frustum
class Frustum {

public:
	Frustum(const Camera& c, const RenderEngine& r);

	bool pointInside(const Eigen::Vector3d& p) const;
	bool overlap(const std::vector<Eigen::Vector3d>& points) const;

private:
	Eigen::Vector3d eye;
	Eigen::Vector3d forw;
	Eigen::Vector3d up;
	Eigen::Vector3d right;

	double near;
	double far;
	double tanAng;
	double aspectRatio;
};

