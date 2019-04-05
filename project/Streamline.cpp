#define _USE_MATH_DEFINES
#include "Streamline.h"

#include "SphericalVectorField.h"
#include "Conversions.h"

Streamline::Streamline(double totalTime, size_t numPoints) :
	points(numPoints),
	_size(0),
	totalTime(totalTime),
	totalLength(-1.0),
	totalAngle(-1.0) {}


void Streamline::addPoint(const Eigen::Vector3d& p) {
	points[_size] = p;
	_size++;
}


double Streamline::getTotalLength() const {
	if (totalLength < 0.0) {
		calculateLength();
	}
	return totalLength;
}


void Streamline::calculateLength() const {
	
	totalLength = 0.0;

	if (_size < 2) {
		return;
	}

	for (size_t i = 0; i < _size - 1; i++) {

		totalLength += (sphToCart(points[i + 1]) - sphToCart(points[i])).norm();
	}
}


double Streamline::getTotalAngle() const {
	if (totalAngle < 0.0) {
		calculateAngle();
	}
	return totalAngle;
}


void Streamline::calculateAngle() const {

	totalAngle = 0.0;

	if (_size < 3) {
		return;
	}

	for (size_t i = 0; i < _size - 2; i++) {

		Eigen::Vector3d p0 = sphToCart(points[i]);
		Eigen::Vector3d p1 = sphToCart(points[i + 1]);
		Eigen::Vector3d p2 = sphToCart(points[i + 2]);

		Eigen::Vector3d v0 = (p1 - p0).normalized();
		Eigen::Vector3d v1 = (p2 - p1).normalized();

		totalAngle += acos(v0.transpose() * v1);
	}
}


void Streamline::addToRenderable(Renderable& r, const SphericalVectorField& field) const {

	double maxMagSq = field.getMaxMagSq();

	Eigen::Vector3d cart0 = sphToCart(points[0]);
	double mag0Sq = field.velocityAtM(points[0]).squaredNorm();

	r.verts.push_back(glm::dvec3(cart0.x(), cart0.y(), cart0.z()));
	r.colours.push_back(glm::vec3(0.f, 0.f, mag0Sq / maxMagSq));

	for (size_t i = 1; i < points.size() - 2; i++) {

		Eigen::Vector3d cart = sphToCart(points[i]);
		double magSq = field.velocityAtM(points[i]).squaredNorm();

		r.verts.push_back(glm::dvec3(cart.x(), cart.y(), cart.z()));
		r.colours.push_back(glm::vec3(0.f, 0.f, magSq / maxMagSq));
		r.verts.push_back(glm::dvec3(cart.x(), cart.y(), cart.z()));
		r.colours.push_back(glm::vec3(0.f, 0.f, magSq / maxMagSq));
	}

	Eigen::Vector3d cartE = sphToCart(points[_size - 1]);
	double magESq = field.velocityAtM(points[_size - 1]).squaredNorm();

	r.verts.push_back(glm::dvec3(cartE.x(), cartE.y(), cartE.z()));
	r.colours.push_back(glm::vec3(0.f, 0.f, magESq / maxMagSq));
}