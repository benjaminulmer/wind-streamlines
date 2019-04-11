#define _USE_MATH_DEFINES
#include "Streamline.h"

#include "SphericalVectorField.h"
#include "Conversions.h"


// Default constructor
//
// field - Spherical vector field the streamline belongs to
Streamline::Streamline(const SphericalVectorField& field) :
	totalLength(0.0),
	totalAngle(0.0),
	field(field) {}


// Construct streamline from forward and backwards lines integrated at the same seed point
//
// back - backwards integrated streamline
// forw - forward integrated streamline
// field - Spherical vector field the streamline belongs to
Streamline::Streamline(const Streamline& back, const Streamline& forw, const SphericalVectorField& field) :
	points(back.size() + forw.size()),
	totalLength(back.totalLength + forw.totalLength),
	totalAngle(back.totalAngle + forw.totalAngle),
	field(field) {

	for (size_t i = 0; i < back.size(); i++) {
		points[i] = back[back.size() - 1 - i];
	}

	for (size_t i = 0; i < forw.size(); i++) {
		points[back.size() + i] = forw[i];
	}
}


// Adds a point to the steamline and updates total length and angle
//
// p - point to add (lat, long, rad) in rads and mbars
void Streamline::addPoint(const Eigen::Vector3d& p) {

	// Need two points for first distance
	if (size() > 0) {

		Eigen::Vector3d curr = points.back();
		totalLength += (sphToCart(curr) - sphToCart(p)).norm();

		// Need three points for first angle
		if (size() > 1) {

			Eigen::Vector3d prev = points[size() - 2];
			Eigen::Vector3d v0 = (curr - prev).normalized();
			Eigen::Vector3d v1 = (p - curr).normalized();

			totalAngle += acos(v0.transpose() * v1);
		}
	}
	points.push_back(p);
}


std::vector<Eigen::Vector3d> Streamline::getSeeds(double sepDist) {

	sepDist *= 1.001;

	double fac = 50.0;

	std::vector<Eigen::Vector3d> seeds;

	// First point
	Eigen::Vector3d cart0 = sphToCart(points[0]);
	Eigen::Vector3d tangent0 = (sphToCart(points[1]) - cart0).normalized();
	Eigen::Vector3d norm0 = cart0.cross(tangent0).normalized();

	Eigen::Affine3d t10(Eigen::AngleAxis<double>(sepDist / cart0.norm(), tangent0));
	Eigen::Affine3d t20(Eigen::AngleAxis<double>(-sepDist / cart0.norm(), tangent0));

	seeds.push_back(cartToSph(cart0 + cart0.normalized() * (sepDist / fac)));
	seeds.push_back(cartToSph(cart0 - cart0.normalized() * (sepDist / fac)));
	seeds.push_back(cartToSph(t10 * cart0));
	seeds.push_back(cartToSph(t20 * cart0));

	// Non-end point geometry is duplicated for drawing lines
	for (size_t i = 1; i < size() - 2; i++) {

		Eigen::Vector3d cart = sphToCart(points[i]);
		Eigen::Vector3d tangent = (sphToCart(points[i + 1]) - sphToCart(points[i - 1])).normalized();
		Eigen::Vector3d norm = cart.cross(tangent).normalized();

		Eigen::Affine3d t1(Eigen::AngleAxis<double>(sepDist / cart.norm(), tangent));
		Eigen::Affine3d t2(Eigen::AngleAxis<double>(-sepDist / cart.norm(), tangent));

		seeds.push_back(cartToSph(cart + cart.normalized() * (sepDist / fac)));
		seeds.push_back(cartToSph(cart - cart.normalized() * (sepDist / fac)));
		seeds.push_back(cartToSph(t1 * cart));
		seeds.push_back(cartToSph(t2 * cart));
	}

	// Last point
	Eigen::Vector3d cartE = sphToCart(points.back());
	Eigen::Vector3d tangentE = (cartE - sphToCart(points[size() - 2])).normalized();
	Eigen::Vector3d normE = cartE.cross(tangentE).normalized();

	Eigen::Affine3d t1E(Eigen::AngleAxis<double>(sepDist / cartE.norm(), tangentE));
	Eigen::Affine3d t2E(Eigen::AngleAxis<double>(-sepDist / cartE.norm(), tangentE));

	seeds.push_back(cartToSph(cartE + cartE.normalized() * (sepDist / fac)));
	seeds.push_back(cartToSph(cartE - cartE.normalized() * (sepDist / fac)));
	seeds.push_back(cartToSph(t1E * cartE));
	seeds.push_back(cartToSph(t2E * cartE));

	return seeds;
}


// Adds the steamline to a renderable object
//
// r - renderable to add steamline geometry to
void Streamline::addToRenderable(StreamlineRenderable& r) const {

	// First point
	Eigen::Vector3d cart0 = sphToCart(points[0]);
	Eigen::Vector3d tangent0 = (sphToCart(points[1]) - cart0).normalized();

	r.addVert(glm::dvec3(cart0.x(), cart0.y(), cart0.z()));
	r.addColour(glm::vec4(0.f, 0.f, 1.f, 1.f));
	r.addTangent(glm::vec3(tangent0.x(), tangent0.y(), tangent0.z()));

	// Non-end point geometry is duplicated for drawing lines
	for (size_t i = 1; i < size() - 2; i++) {

		Eigen::Vector3d cart = sphToCart(points[i]);
		Eigen::Vector3d tangent = (sphToCart(points[i + 1]) - sphToCart(points[i - 1])).normalized();

		r.addVert(glm::dvec3(cart.x(), cart.y(), cart.z()));
		r.addVert(glm::dvec3(cart.x(), cart.y(), cart.z()));
		r.addColour(glm::vec4(0.f, 0.f, 1.f, 1.f));
		r.addColour(glm::vec4(0.f, 0.f, 1.f, 1.f));
		r.addTangent(glm::vec3(tangent.x(), tangent.y(), tangent.z()));
		r.addTangent(glm::vec3(tangent.x(), tangent.y(), tangent.z()));
	}

	// Last point
	Eigen::Vector3d cartE = sphToCart(points.back());
	Eigen::Vector3d tangentE = (cartE - sphToCart(points[size() - 2])).normalized();

	r.addVert(glm::dvec3(cartE.x(), cartE.y(), cartE.z()));
	r.addColour(glm::vec4(0.f, 0.f, 1.f, 1.f));
	r.addTangent(glm::vec3(tangentE.x(), tangentE.y(), tangentE.z()));
}