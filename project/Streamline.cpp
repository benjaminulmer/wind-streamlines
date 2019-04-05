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
	return (totalLength >= 0.0) ? totalLength : calculateLength();
}


double Streamline::calculateLength() const {

}


double Streamline::getTotalAngle() const {
	return (totalAngle >= 0.0) ? totalAngle : calculateAngle();
}


double Streamline::calculateAngle() const {

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