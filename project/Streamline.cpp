#define _USE_MATH_DEFINES
#include "Streamline.h"

#include "SphericalVectorField.h"
#include "Conversions.h"


// Initialize empty steamline with a set number of points and total time
//
// totalTime - total integration time of streamline (forward + backward)
// numPoints - number of points in the steamline
Streamline::Streamline(double totalTime, size_t numPoints, const SphericalVectorField& field) :
	points(numPoints),
	_size(0),
	totalTime(totalTime),
	totalLength(0.0),
	totalAngle(0.0),
	field(field) {}


// Adds a point to the steamline
//
// p - point to add (lat, long, rad) in rads and mbars
void Streamline::addPoint(const Eigen::Vector3d& p) {

	if (_size > 0) {

		Eigen::Vector3d curr = points[_size - 1];
		totalLength += (sphToCart(curr) - sphToCart(p)).norm();

		if (_size > 1) {

			Eigen::Vector3d prev = points[_size - 2];
			Eigen::Vector3d v0 = (curr - prev).normalized();
			Eigen::Vector3d v1 = (p - curr).normalized();

			totalAngle += acos(v0.transpose() * v1);
		}
	}
	
	points[_size] = p;
	_size++;
}


float col(double a, double b) {
	return 0.5 * (a / b) + 0.5;
}


// Adds the steamline to a renderable object
//
// r - renderable to add steamline geometry to
void Streamline::addToRenderable(StreamlineRenderable& r, double d) const {

	double maxMagSq = field.getMaxMagSq();
	double ratio = totalAngle / totalLength;

	Eigen::Vector3d cart0 = sphToCart(points[0]);
	double mag0Sq = field.velocityAtM(points[0]).squaredNorm();
	Eigen::Vector3d tangent0 = (sphToCart(points[1]) - cart0).normalized();

	r.addVert(glm::dvec3(cart0.x(), cart0.y(), cart0.z()));
	r.addColour(glm::vec3(0.f, 0.f, 1.f));
	r.addTangent(glm::vec3(tangent0.x(), tangent0.y(), tangent0.z()));

	for (size_t i = 1; i < points.size() - 2; i++) {

		Eigen::Vector3d cart = sphToCart(points[i]);
		double magSq = field.velocityAtM(points[i]).squaredNorm();
		Eigen::Vector3d tangent = (sphToCart(points[i + 1]) - sphToCart(points[i - 1])).normalized();

		r.addVert(glm::dvec3(cart.x(), cart.y(), cart.z()));
		r.addColour(glm::vec3(0.f, 0.f, 1.f));
		r.addTangent(glm::vec3(tangent.x(), tangent.y(), tangent.z()));
		r.addVert(glm::dvec3(cart.x(), cart.y(), cart.z()));
		r.addColour(glm::vec3(0.f, 0.f, 1.f));
		r.addTangent(glm::vec3(tangent.x(), tangent.y(), tangent.z()));

		if (tangent.squaredNorm() < 0.5) {
			std::cout << tangent << std::endl;
		}
	}

	Eigen::Vector3d cartE = sphToCart(points[_size - 1]);
	double magESq = field.velocityAtM(points[_size - 1]).squaredNorm();
	Eigen::Vector3d tangentE = (sphToCart(points[_size - 1]) - sphToCart(points[_size - 2])).normalized();

	r.addVert(glm::dvec3(cartE.x(), cartE.y(), cartE.z()));
	r.addColour(glm::vec3(0.f, 0.f, 1.f));
	r.addTangent(glm::vec3(tangentE.x(), tangentE.y(), tangentE.z()));
}