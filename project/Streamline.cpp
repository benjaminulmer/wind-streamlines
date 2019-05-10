#define _USE_MATH_DEFINES
#include "Streamline.h"

#include "color/ColorSpace.h"
#include "Conversions.h"
#include "SphericalVectorField.h"


// Default constructor
//
// field - Spherical vector field the streamline belongs to
Streamline::Streamline(const SphericalVectorField* field) :
	totalTime(0.f),
	sumAlt(0.0),
	totalLength(0.0),
	totalAngle(0.0),
	render(nullptr),
	field(field) {}


// Construct streamline from forward and backwards lines integrated at the same seed point
//
// back - backwards integrated streamline
// forw - forward integrated streamline
// field - Spherical vector field the streamline belongs to
Streamline::Streamline(const Streamline& back, const Streamline& forw, const SphericalVectorField* field) :
	points(back.size() + forw.size()),
	localTimes(back.size() + forw.size()),
	totalTime(back.totalTime + forw.totalTime),
	sumAlt(back.sumAlt + forw.sumAlt),
	totalLength(back.totalLength + forw.totalLength),
	totalAngle(back.totalAngle + forw.totalAngle),
	render(nullptr),
	field(field) {

	for (size_t i = 0; i < back.size(); i++) {
		points[i] = back.points[back.size() - 1 - i];
		localTimes[i] = back.totalTime - back.localTimes[back.size() - 1 - i];
	}

	for (size_t i = 0; i < forw.size(); i++) {
		points[back.size() + i] = forw.points[i];
		localTimes[back.size() + i] = forw.localTimes[i] + back.totalTime;;
	}
}


// Adds a spherical point to the steamline and updates total length and angle
//
// pSph - point to add (lat, long, altitude) in rads and mbars
// time - integration time of point in seconds
void Streamline::addPoint(const Eigen::Vector3d & pSph, float time) {
	addPoint(pSph, sphToCart(pSph), time);
}


// Adds a point to the steamline and updates total length and angle
//
// pSph - point to add (lat, long, altitude) in rads and mbars
// pCart - pSph in cartesian coordinates
// time - integration time of point in seconds
void Streamline::addPoint(const Eigen::Vector3d& pSph, const Eigen::Vector3d& pCart, float time) {

	sumAlt += mbarsToAlt(pSph.z());

	// Need two points for first distance
	if (size() > 0) {

		Eigen::Vector3d curr = points.back();
		totalLength += (curr - pCart).norm();

		// Need three points for first angle
		if (size() > 1) {

			Eigen::Vector3d prev = points[size() - 2];
			Eigen::Vector3d v0 = (curr - prev).normalized();
			Eigen::Vector3d v1 = (pCart - curr).normalized();

			totalAngle += acos(v0.transpose() * v1);
		}
	}
	points.push_back(pCart);
	localTimes.push_back(time);
	totalTime += time;
}


// Gets seed candidates from streamline
//
// sepDist - seperation distance seeds are from line
// return - list of candidate seed points in cartesian coordinates
std::vector<Eigen::Vector3d> Streamline::getSeeds(double sepDist) {

	std::vector<Eigen::Vector3d> seeds;

	// First point
	Eigen::Vector3d cart0 = points[0];
	Eigen::Vector3d tangent0 = (points[1] - cart0).normalized();
	Eigen::Vector3d norm0 = cart0.cross(tangent0).normalized();

	Eigen::Affine3d t10(Eigen::AngleAxis<double>(sepDist / cart0.norm(), tangent0));
	Eigen::Affine3d t20(Eigen::AngleAxis<double>(-sepDist / cart0.norm(), tangent0));

	seeds.push_back(t10 * cart0);
	seeds.push_back(t20 * cart0);
	seeds.push_back(cart0 + cart0.normalized() * (sepDist / RADIAL_DIST_SCALE));
	seeds.push_back(cart0 - cart0.normalized() * (sepDist / RADIAL_DIST_SCALE));

	// Middle points
	for (size_t i = 1; i < size() - 1; i+=50) {

		Eigen::Vector3d cart = points[i];
		Eigen::Vector3d tangent = (points[i + 1] - points[i - 1]).normalized();
		Eigen::Vector3d norm = cart.cross(tangent).normalized();

		Eigen::Affine3d t1(Eigen::AngleAxis<double>(sepDist / cart.norm(), tangent));
		Eigen::Affine3d t2(Eigen::AngleAxis<double>(-sepDist / cart.norm(), tangent));

		seeds.push_back(t1 * cart);
		seeds.push_back(t2 * cart);
		seeds.push_back(cart + cart.normalized() * (sepDist / RADIAL_DIST_SCALE));
		seeds.push_back(cart - cart.normalized() * (sepDist / RADIAL_DIST_SCALE));
	}

	// Last point
	Eigen::Vector3d cartE = points.back();
	Eigen::Vector3d tangentE = (cartE - points[size() - 2]).normalized();
	Eigen::Vector3d normE = cartE.cross(tangentE).normalized();

	Eigen::Affine3d t1E(Eigen::AngleAxis<double>(sepDist / cartE.norm(), tangentE));
	Eigen::Affine3d t2E(Eigen::AngleAxis<double>(-sepDist / cartE.norm(), tangentE));

	seeds.push_back(t1E * cartE);
	seeds.push_back(t2E * cartE);
	seeds.push_back(cartE + cartE.normalized() * (sepDist / RADIAL_DIST_SCALE));
	seeds.push_back(cartE - cartE.normalized() * (sepDist / RADIAL_DIST_SCALE));

	return seeds;
}


// Adds the streamline to a renderable object
//
// r - renderable to add steamline geometry to
void Streamline::createRenderable(const glm::vec3& c1, const glm::vec3& c2) {

	if (render != nullptr) {
		delete render;
	}
	render = new StreamlineRenderable();
	render->setDrawMode(GL_LINES);

	double maxAlt = mbarsToAlt(1.0);
	ColorSpace::Rgb lowRGB(c1.x, c1.y, c1.z);
	ColorSpace::Rgb highRGB(c2.x, c2.y, c2.z);

	ColorSpace::Lab lowLab;
	ColorSpace::Lab highLab;

	lowLab.Initialize(&lowRGB);
	highLab.Initialize(&highRGB);

	ColorSpace::Lab lab;
	ColorSpace::Rgb RGB;

	// First point
	Eigen::Vector3d cart0 = points[0];
	Eigen::Vector3d tangent0 = (points[1] - cart0).normalized();
	double n0 = (cart0.norm() - RADIUS_EARTH_M) / maxAlt;
	lab.l = (1.0 - n0) * lowLab.l + n0 * highLab.l;
	lab.a = (1.0 - n0) * lowLab.a + n0 * highLab.a;
	lab.b = (1.0 - n0) * lowLab.b + n0 * highLab.b;
	lab.ToRgb(&RGB);

	render->addVert(glm::dvec3(cart0.x(), cart0.y(), cart0.z()));
	render->addColour(glm::u8vec3(255 * (RGB.r / 1.0), 255 * (RGB.g / 1.0), 255 * (RGB.b / 1.0)));
	render->addTangent(glm::vec3(tangent0.x(), tangent0.y(), tangent0.z()));
	render->addLocalTime(localTimes[0]);

	// Non-end point geometry is duplicated for drawing lines
	for (size_t i = 1; i < size() - 1; i++) {

		Eigen::Vector3d cart = points[i];
		Eigen::Vector3d tangent = (points[i + 1] - points[i - 1]).normalized();
		double n = (cart.norm() - RADIUS_EARTH_M) / maxAlt;
		lab.l = (1.0 - n) * lowLab.l + n * highLab.l;
		lab.a = (1.0 - n) * lowLab.a + n * highLab.a;
		lab.b = (1.0 - n) * lowLab.b + n * highLab.b;
		lab.ToRgb(&RGB);

		render->addVert(glm::dvec3(cart.x(), cart.y(), cart.z()));
		render->addVert(glm::dvec3(cart.x(), cart.y(), cart.z()));
		render->addColour(glm::u8vec3(255 * (RGB.r / 1.0), 255 * (RGB.g / 1.0), 255 * (RGB.b / 1.0)));
		render->addColour(glm::u8vec3(255 * (RGB.r / 1.0), 255 * (RGB.g / 1.0), 255 * (RGB.b / 1.0)));
		render->addTangent(glm::vec3(tangent.x(), tangent.y(), tangent.z()));
		render->addTangent(glm::vec3(tangent.x(), tangent.y(), tangent.z()));
		render->addLocalTime(localTimes[i]);
		render->addLocalTime(localTimes[i]);
	}

	// Last point
	Eigen::Vector3d cartE = points.back();
	Eigen::Vector3d tangentE = (cartE - points[size() - 2]).normalized();
	double nE = (cartE.norm() - RADIUS_EARTH_M) / maxAlt;
	lab.l = (1.0 - nE) * lowLab.l + nE * highLab.l;
	lab.a = (1.0 - nE) * lowLab.a + nE * highLab.a;
	lab.b = (1.0 - nE) * lowLab.b + nE * highLab.b;
	lab.ToRgb(&RGB);

	render->addVert(glm::dvec3(cartE.x(), cartE.y(), cartE.z()));
	render->addColour(glm::u8vec3(255 * (RGB.r / 1.0), 255 * (RGB.g / 1.0), 255 * (RGB.b / 1.0)));
	render->addTangent(glm::vec3(tangentE.x(), tangentE.y(), tangentE.z()));
	render->addLocalTime(localTimes.back());
}