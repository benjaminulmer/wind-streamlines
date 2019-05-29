#include "Frustum.h"

#include "Camera.h"
#include "Conversions.h"
#include "RenderEngine.h"


// Construct frustum from camera and projection matrix information
//
// c - camera
// r - render engine with current projection matrix
Frustum::Frustum(const Camera& c, const RenderEngine& r) :
	near(r.getNear()),
	far(r.getFar()),
	tanAng(tan(r.getFovY() * 0.5)),
	aspectRatio(r.getAspectRatio()) {

	glm::dvec3 eyeG = c.getPosition();
	glm::dvec3 dirG = c.getLookDir();
	glm::dvec3 upG = c.getUp();

	eye = Eigen::Vector3d(eyeG.x, eyeG.y, eyeG.z);
	forw = Eigen::Vector3d(dirG.x, dirG.y, dirG.z);
	up = Eigen::Vector3d(upG.x, upG.y, upG.z);
	right = forw.cross(up);
}


// Test if point is inside frustum
//
// p - point in cartesian coordinates
bool Frustum::pointInside(const Eigen::Vector3d& p) const {
	
	Eigen::Vector3d v = p - eye;

	// Test to see if between near and far
	double vProjForw = v.dot(forw);
	if (vProjForw > far || vProjForw < near) {
		return false;
	}

	// Test to see if withing up/down planes
	double vProjUp = v.dot(up);
	double height = vProjForw * tanAng;
	if (vProjUp > height || vProjUp < -height) {
		return false;
	}

	// Test to see if within right/left planes
	double vProjRight = v.dot(right);
	double width = height * aspectRatio;
	if (vProjRight > width || vProjRight < -width) {
		return false;
	}
	
	return true;
}


// Test if list of points has any overlap with frustum
//
// points - list of points in spherical coordinates
bool Frustum::overlap(const std::vector<Eigen::Vector3d>& points) const {
	
	for (const Eigen::Vector3d& p : points) {
		if (pointInside(p)) {
			return true;
		}
	}
	return false;
}