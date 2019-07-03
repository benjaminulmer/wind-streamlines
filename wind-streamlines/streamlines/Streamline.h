#pragma once

#include "rendering/Renderable.h"

class SphericalVectorField;

#include <Eigen/Dense>

#include <vector>


// Class for storing and representing a single streamline. Points are stored in cartesian coordinates
class Streamline {

public:
	Streamline(const SphericalVectorField* field);
	Streamline(const Streamline& back, const Streamline& forw, const SphericalVectorField* field);

	void addPoint(const Eigen::Vector3d& pSph, float time);
	void addPoint(const Eigen::Vector3d& pSph, const Eigen::Vector3d& pCart, float time);
	const std::vector<Eigen::Vector3d>& getPoints() const { return points; }
	size_t size() const { return points.size(); }

	double getSumAlt() const { return sumAlt; }
	double getTotalLength() const { return totalLength; }
	double getTotalAngle() const { return totalAngle; }
	StreamlineRenderable* getRender() { return render; }

	std::vector<Eigen::Vector3d> getSeeds(double sepDist);

	void createRenderable(const glm::vec3& c1, const glm::vec3& c2);

private:
	std::vector<Eigen::Vector3d> points;
	std::vector<float> localTimes;

	float totalTime;
	double sumAlt;
	double totalLength;
	double totalAngle;

	StreamlineRenderable* render;
	const SphericalVectorField* field;
};

