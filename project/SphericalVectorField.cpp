#define _USE_MATH_DEFINES
#include "SphericalVectorField.h"

#include "Conversions.h"

#include <algorithm>
#include <cmath>


// Construct vector field from data provided in NetCDF file
// Assumes data is of a certain format, does not work for general files
//
// file - netCDF file containing ERA5 wind data (u, v, w) at all levels at one time slice
SphericalVectorField::SphericalVectorField(const netCDF::NcFile& file) : 
	data(NUM_LONGS * NUM_LATS * NUM_LEVELS),
	levels(NUM_LEVELS), 
	lats(NUM_LATS),
	longs(NUM_LONGS) {

	// Get values for levels, latitude, and longitude
	file.getVar("level").getVar(levels.data());
	file.getVar("latitude").getVar(lats.data());
	file.getVar("longitude").getVar(longs.data());

	// Convert from degrees to radians
	for (size_t i = 0; i < NUM_LATS; i++) {
		lats[i] *= (M_PI / 180.0);
	}
	for (size_t i = 0; i < NUM_LONGS; i++) {
		longs[i] *= (M_PI / 180.0);
	}

	// Get wind components
	netCDF::NcVar uVar = file.getVar("u");
	netCDF::NcVar vVar = file.getVar("v");
	netCDF::NcVar wVar = file.getVar("w");

	double uScale, vScale, wScale, uOffset, vOffset, wOffset;

	uVar.getAtt("scale_factor").getValues(&uScale);
	vVar.getAtt("scale_factor").getValues(&vScale);
	wVar.getAtt("scale_factor").getValues(&wScale);
	uVar.getAtt("add_offset").getValues(&uOffset);
	vVar.getAtt("add_offset").getValues(&vOffset);
	wVar.getAtt("add_offset").getValues(&wOffset);

	// u, v, and w have same dimensions
	size_t size = NUM_LONGS * NUM_LATS * NUM_LEVELS;

	double* uArry = new double[size];
	double* vArry = new double[size];
	double* wArry = new double[size];

	uVar.getVar(uArry);
	vVar.getVar(vArry);
	wVar.getVar(wArry);

	maxMagSq = 0.0;

	// Apply scale and offset and add to data vector
	for (size_t i = 0; i < size; i++) {
		double u = uArry[i] * uScale + uOffset;
		double v = vArry[i] * vScale + vOffset;
		double w = wArry[i] * wScale + wOffset;

		data[i] = (Eigen::Vector3d(v, u, w));

		Eigen::Matrix<size_t, 3, 1> index = offsetToIndex(i);

		// Convert vertical to m/s to find max magnitude
		double r0 = mbarsToAlt(levels[index.z()]);
		double r1 = mbarsToAlt(levels[index.z()] + 0.01 * w);

		Eigen::Vector3d vel = data[i];
		vel.z() = r1 - r0;
		double magSq = vel.squaredNorm();

		maxMagSq = std::max(maxMagSq, magSq);
	}
	std::cout << maxMagSq << std::endl;

	delete[] uArry;
	delete[] vArry;
	delete[] wArry;
}


// Finds all critical points in the vector field and their Poincare index
//
// return - list of indicies of cells that contain critical points and their Poincare index
std::vector<std::pair<Eigen::Matrix<size_t, 3, 1>, int>> SphericalVectorField::findCriticalPoints() const {

	std::vector<std::pair<Eigen::Matrix<size_t, 3, 1>, int>> points;

	for (size_t lvl = 0; lvl < NUM_LEVELS - 1; lvl++) {
		for (size_t lat = 0; lat < NUM_LATS - 1; lat++) {
			for (size_t lng = 0; lng < NUM_LONGS; lng++) {

				// 8 vertices of hexahedron
				size_t i0 = indexToOffset(lat, lng, lvl);
				size_t i1 = indexToOffset(lat, lng, lvl + 1);
				size_t i2 = indexToOffset(lat + 1, lng, lvl + 1);
				size_t i3 = indexToOffset(lat, (lng + 1) % NUM_LONGS, lvl + 1);
				size_t i4 = indexToOffset(lat + 1, (lng + 1) % NUM_LONGS, lvl);
				size_t i5 = indexToOffset(lat + 1, (lng + 1) % NUM_LONGS, lvl + 1);
				size_t i6 = indexToOffset(lat, (lng + 1) % NUM_LONGS, lvl);
				size_t i7 = indexToOffset(lat + 1, lng, lvl);
				
				// Construct 5 tets from hex and test each one to see if it has a critical point
				int pi;
				if ((pi = criticalPointInTet(i0, i1, i2, i3)) != 0) {
					Eigen::Matrix<size_t, 3, 1> i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Matrix<size_t, 3, 1>, int>(i, pi));
				}
				else if ((pi = criticalPointInTet(i4, i5, i3, i2)) != 0) {
					Eigen::Matrix<size_t, 3, 1> i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Matrix<size_t, 3, 1>, int>(i, pi));
				}
				else if ((pi = criticalPointInTet(i0, i7, i4, i2)) != 0) {
					Eigen::Matrix<size_t, 3, 1> i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Matrix<size_t, 3, 1>, int>(i, pi));
				}
				else if ((pi = criticalPointInTet(i0, i6, i4, i3)) != 0) {
					Eigen::Matrix<size_t, 3, 1> i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Matrix<size_t, 3, 1>, int>(i, pi));
				}
				else if ((pi = criticalPointInTet(i2, i3, i0, i4)) != 0) {
					Eigen::Matrix<size_t, 3, 1> i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Matrix<size_t, 3, 1>, int>(i, pi));
				}
			}
		}
	}
	return points;
}


// Returns the sign of a tetrahedron
// Algorithm from "Detection and classification of critical points in piecewise linear vector fields" Wang et al. 2018
//
// v0 - vertex 0
// v1 - vertex 1
// v2 - vertex 2
// v3 - vertex 3
// i0 - index of v0
// i1 - index of v1
// i2 - index of v2
// i3 - index of v3
// return - sign which is +1 or -1
int SphericalVectorField::signTet(const Eigen::Vector4d& v0, const Eigen::Vector4d& v1,
                                  const Eigen::Vector4d& v2, const Eigen::Vector4d& v3,
                                  size_t i0, size_t i1, size_t i2, size_t i3) const {
	
	// Inversion count tells us orientation of tet. Loop unwrapped
	int invCount = 0;
	if (i0 < i1) invCount++;
	if (i0 < i2) invCount++;
	if (i0 < i3) invCount++;
	if (i1 < i2) invCount++;
	if (i1 < i3) invCount++;
	if (i2 < i3) invCount++;

	Eigen::Matrix4d m;
	m << v0, v1, v2, v3;

	double det = m.determinant();
	int detSign = (det <= 0) ? -1 : 1;

	return (invCount % 2 == 0) ? detSign : -detSign;
}


// Returns if the tet contains a critical point. If yes, return the Poincare index
// Algorithm from "Detection and classification of critical points in piecewise linear vector fields" Wang et al. 2018
//
// i0 - index 0
// i1 - index 1
// i2 - index 2
// i3 - index 3
// return - 0 for no critical point, otherwise Poincare index which is +1 or -1
int SphericalVectorField::criticalPointInTet(size_t i0, size_t i1, size_t i2, size_t i3) const {

	// Get vector field values
	Eigen::Vector4d v0, v1, v2, v3;
	v0 << data[i0], 1.0;
	v1 << data[i1], 1.0;
	v2 << data[i2], 1.0;
	v3 << data[i3], 1.0;

	// Test if tet contains critical point
	Eigen::Vector4d zeroPoint(0.0, 0.0, 0.0, 1.0);
	int simplexSign = signTet(zeroPoint, v1, v2, v3, i0, i1, i2, i3);

	if (signTet(v0, zeroPoint, v2, v3, i0, i1, i2, i3) != simplexSign) {
		return 0;
	}
	if (signTet(v0, v1, zeroPoint, v3, i0, i1, i2, i3) != simplexSign) {
		return 0;
	}
	if (signTet(v0, v1, v2, zeroPoint, i0, i1, i2, i3) != simplexSign) {
		return 0;
	}

	// Contains critical point, check Poincare index
	Eigen::Vector4d p0, p1, p2, p3;
	p0 << sphCoords(i0), 1.0;
	p1 << sphCoords(i1), 1.0;
	p2 << sphCoords(i2), 1.0;
	p3 << sphCoords(i3), 1.0;

	if (signTet(p0, p1, p2, p3, i0, i1, i2, i3) != simplexSign) {
		return -1;
	}
	else {
		return 1;
	}
}


// Forward integrates streamline starting at given seed
//
// seed - starting seed point (lat, long, rad) in rads and mbars
// totalTime - total amount of time to integrate forwards and backwards
// tol - error tolerance
// return - list of points in streamline in coordinates (lat, long, rad) in rads and mbars
Streamline SphericalVectorField::streamline(const Eigen::Vector3d& seed, double maxDist, double tol, double maxStep,
                                            const VoxelGrid& vg) const {

	Streamline forw(*this);
	Streamline back(*this);

	Eigen::Vector3d currPos = seed;
	double timeStep = maxStep;
	double length = 0.0;

	// Forward integrate in time
	forw.addPoint(currPos);
	while (length < maxDist) {

		currPos = RKF45Adaptive(currPos, timeStep, tol, maxStep);
		if (!vg.testPoint(sphToCart(currPos))) {
			break;
		}
		forw.addPoint(currPos);

		if (forw.getTotalLength() - length < 10.0 || timeStep == 0.0) {
			break;
		}
		length = forw.getTotalLength();
	}
	currPos = seed;
	timeStep = -maxStep;
	length = 0.0;

	// Backward integrate in time
	back.addPoint(currPos);
	while (back.getTotalLength() < maxDist) {

		currPos = RKF45Adaptive(currPos, timeStep, tol, maxStep);
		if (!vg.testPoint(sphToCart(currPos))) {
			break;
		}
		back.addPoint(currPos);

		if (back.getTotalLength() - length < 10.0 || timeStep == 0.0) {
			break;
		}
		length = back.getTotalLength();
	}

	// Combine forward and backward paths into one chronological path
	Streamline streamline(back, forw, *this);
	return streamline;
}


// Performs one step of RKF45 integration with adaptive step size 
// https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta%E2%80%93Fehlberg_method
//
// currPos - current position (lat, long, rad) in rads and mbars
// timeStep - current step size in and updated step size out. Gets set to 0 if it becomes prohibitively small
// tol - error tolerance 
// return - next position (lat, long, rad) in rads and mbars
Eigen::Vector3d SphericalVectorField::RKF45Adaptive(const Eigen::Vector3d& currPos, double& timeStep, double tol, double maxStep) const {

	// Loop until error is low enough, almost always <= 2 itterations
	while (true) {
		double scaledStep = (param) ? timeStep * cos(currPos.x()) : timeStep;

		Eigen::Vector3d k1 = scaledStep * velocityAt(currPos);
		Eigen::Vector3d k2 = scaledStep * velocityAt(newPos(currPos, 1.0 / 4.0       * k1));
		Eigen::Vector3d k3 = scaledStep * velocityAt(newPos(currPos, 3.0 / 32.0      * k1 + 9.0 / 32.0      * k2));
		Eigen::Vector3d k4 = scaledStep * velocityAt(newPos(currPos, 1932.0 / 2197.0 * k1 - 7200.0 / 2197.0 * k2 + 7296.0 / 2197.0 * k3));
		Eigen::Vector3d k5 = scaledStep * velocityAt(newPos(currPos, 439.0 / 216.0   * k1 - 8.0             * k2 + 3680.0 / 513.0  * k3 - 845.0 / 4104.0  * k4));
		Eigen::Vector3d k6 = scaledStep * velocityAt(newPos(currPos, -8.0 / 27.0     * k1 + 2.0             * k2 - 3544.0 / 2565.0 * k3 + 1859.0 / 4104.0 * k4 - 11.0 / 40.0 * k5));

		Eigen::Vector3d highOrder = 16.0 / 135.0 * k1 + 6656.0 / 12825.0 * k3 + 28561.0 / 56430.0 * k4 - 9.0 / 50.0 * k5 + 2.0 / 55.0 * k6;
		Eigen::Vector3d lowOrder = 25.0 / 216.0 * k1 + 1408.0 / 2665.0  * k3 + 2197.0 / 4104.0   * k4 - 1.0 / 5.0  * k5;

		double error = (highOrder - lowOrder).norm();

		timeStep *= 0.9 * std::min(std::max((tol / error), 0.3), 2.0);
		if (timeStep > maxStep) timeStep = maxStep;
		if (timeStep < -maxStep) timeStep = -maxStep;

		// Return if error is low enough
		if (error < tol) {
			return newPos(currPos, highOrder);
		}

		// If time step is too small, terminate and indicate by setting timeStep to 0
		else if (abs(timeStep) < 0.01) {

			std::cout << "small step" << std::endl;
			std::cout << velocityAt(currPos) << std::endl;
			timeStep = 0.0;
			return currPos;
		}
	}
}


// Returns the velocity at the given position
//
// pos - (lat, long, altitude) in rads and mbars
// return - (north, east, vertical) in m/s and Pa/s
Eigen::Vector3d SphericalVectorField::velocityAt(const Eigen::Vector3d& pos) const {

	size_t latIndex = (size_t)(720.0 - (pos.x() + M_PI_2) * 4.0 * (180.0 / M_PI));
	size_t longIndex = (size_t)(fmod(pos.y(), 2.0 * M_PI) * 4.0 * (180.0 / M_PI));
	size_t levelIndex = 0;

	// Levels are non-uniform, use binary search to find appropriate index
	size_t endIndex = NUM_LEVELS - 1;
	while (pos.z() >= levels[levelIndex + 1]) {

		size_t mid = (levelIndex + endIndex) / 2;
		if (pos.z() >= levels[mid]) {
			levelIndex = mid;
		}
		else {
			endIndex = mid;
		}
		if (levelIndex == NUM_LEVELS - 2) {
			if (pos.z() >= levels[NUM_LEVELS - 1]) {
				levelIndex++;
			}
			break;
		}
	}

	double latPerc, longPerc, levelPerc;
	int latInc, levelInc;

	// Handle lat beyond end of grid
	if (latIndex == NUM_LATS - 1) {
		latPerc = 0.0;
		latInc = 0;
	}
	else {
		latPerc = (pos.x() - lats[latIndex]) / (lats[latIndex + 1] - lats[latIndex]);
		latInc = 1;
	}

	// Handle long wrap around
	if (longIndex == NUM_LONGS - 1) {
		longPerc = (pos.y() - longs[longIndex]) / (2.0 * M_PI - longs[longIndex]);
	}
	else {
		longPerc = (pos.y() - longs[longIndex]) / (longs[longIndex + 1] - longs[longIndex]);
	}

	// Handle level at end of grid
	if (levelIndex == NUM_LEVELS - 1) {
		levelPerc = 0.0;
		levelInc = 0;
	}
	else {
		levelPerc = (pos.z() - levels[levelIndex]) / (levels[levelIndex + 1] - levels[levelIndex]);
		levelInc = 1;
	}

	// 8 corners of hexahedron
	Eigen::Vector3d _000 = (*this)(latIndex, longIndex, levelIndex);
	Eigen::Vector3d _001 = (*this)(latIndex, longIndex, levelIndex + levelInc);
	Eigen::Vector3d _010 = (*this)(latIndex, (longIndex + 1) % NUM_LONGS, levelIndex);
	Eigen::Vector3d _011 = (*this)(latIndex, (longIndex + 1) % NUM_LONGS, levelIndex + levelInc);
	Eigen::Vector3d _100 = (*this)(latIndex + latInc, longIndex, levelIndex);
	Eigen::Vector3d _101 = (*this)(latIndex + latInc, longIndex, levelIndex + levelInc);
	Eigen::Vector3d _110 = (*this)(latIndex + latInc, (longIndex + 1) % NUM_LONGS, levelIndex);
	Eigen::Vector3d _111 = (*this)(latIndex + latInc, (longIndex + 1) % NUM_LONGS, levelIndex + levelInc);
	
	// Multiply each point by its total contribution
	_000 *= (1.0 - latPerc) * (1.0 - longPerc) * (1.0 - levelPerc);
	_001 *= (1.0 - latPerc) * (1.0 - longPerc) * levelPerc;
	_010 *= (1.0 - latPerc) * longPerc * (1.0 - levelPerc);
	_011 *= (1.0 - latPerc) * longPerc * levelPerc;
	_100 *= latPerc * (1.0 - longPerc) * (1.0 - levelPerc);
	_101 *= latPerc * (1.0 - longPerc) * levelPerc;
	_110 *= latPerc * longPerc * (1.0 - levelPerc);
	_111 *= latPerc * longPerc * levelPerc;

	return _000 + _001 + _010 + _011 + _100 + _101 + _110 + _111;
}


// Returns the velocity at the given position with vertical converted to m/s
//
// pos - (lat, long, altitude) in rads and mbars
// return - (north, east, vertical) in m/s
Eigen::Vector3d SphericalVectorField::velocityAtM(const Eigen::Vector3d& pos) const {

	Eigen::Vector3d vel = velocityAt(pos);

	double r0 = mbarsToAlt(pos.z());
	double r1 = mbarsToAlt(pos.z() + 0.01 * vel.z());

	vel.x() = r1 - r0;

	return vel;
}


// Calculates new position from current position and velocity
//
// currPos - (lat, long, altitude) in rads and mbars
// velocity - (north, east, vertical) in m/s and Pa/s
// return - (lat, long, altitude) in rads and mbars
Eigen::Vector3d SphericalVectorField::newPos(const Eigen::Vector3d& currPos, const Eigen::Vector3d& velocity) const {

	Eigen::Vector3d newPos;
	double absRadius = mbarsToAbs(currPos.z());

	double cosLat = cos(currPos.x());

	// TODO singularities at poles, how to account for this? Doing in physical space is not stable (trig on small angles)
	newPos.x() = currPos.x() + velocity.x() / absRadius;
	newPos.y() = (cosLat > 0.0001) ? currPos.y() + velocity.y() / (cos(currPos.x()) * absRadius) : currPos.y();
	newPos.z() = currPos.z();// +0.01 * velocity.z();

	// TODO properly account for edge cases as opposed to this hacky method
	if (newPos.x() > M_PI_2) newPos.x() = M_PI_2;
	if (newPos.x() < -M_PI_2) newPos.x() = -M_PI_2;
	if (newPos.y() < 0.0) newPos.y() += 2.0 * M_PI;
	if (newPos.y() > 2.0 * M_PI) newPos.y() -= 2.0 * M_PI;
	if (newPos.z() > levels[NUM_LEVELS - 1]) newPos.z() = levels[NUM_LEVELS - 1];
	if (newPos.z() < levels[0]) newPos.z() = levels[0];
	return newPos;
}


// Returns the spherical coordinates of the grid point at absolute index i
//
// i - absolute 1D index
// return - (lat, long, altitude) in rads and meters
Eigen::Vector3d SphericalVectorField::sphCoords(size_t i) const {
	return sphCoords(offsetToIndex(i));
}


// Returns the spherical coordinates of the grid point at lat, long, level index
//
// lat - latitude index
// lng - longitude index
// lvl - level index
// return - (lat, long, altitude) in rads and meters
Eigen::Vector3d SphericalVectorField::sphCoords(size_t lat, size_t lng, size_t lvl) const {
	return Eigen::Vector3d(lats[lat], longs[lng], levels[lvl]);
}


// Returns the spherical coordinates of the grid at (lat, long, level) index
//
// i - (lat, long, level) indices
// return - (lat, long, altitude) in rads and meters
Eigen::Vector3d SphericalVectorField::sphCoords(const Eigen::Matrix<size_t, 3, 1>& i) const {
	return sphCoords(i.x(), i.y(), i.z());
}


// Converts absolute index to lat, long, level index
//
// i - absolute 1D index
// return - (lat, long, level) index
Eigen::Matrix<size_t, 3, 1> SphericalVectorField::offsetToIndex(size_t i) const {

	Eigen::Matrix<size_t, 3, 1> v;
	v.x() = (i / NUM_LONGS) % NUM_LATS;
	v.y() = i % NUM_LONGS;
	v.z() = (i / NUM_LONGS) / NUM_LATS;

	return v;
}


// Converts lat, long, level index to absolulte index
//
// lat - latitude index
// lng - longitude index
// lvl - level index
// return - absolute 1D index
size_t SphericalVectorField::indexToOffset(size_t lat, size_t lng, size_t lvl) const {
	return lng + NUM_LONGS * (lat + NUM_LATS * lvl);
}


// Converts lat, long, level index to absolulte index
//
// i - (lat, long, level) indices
// return - absolute 1D index
size_t SphericalVectorField::indexToOffset(const Eigen::Matrix<size_t, 3, 1>& i) const {
	return indexToOffset(i.x(), i.y(), i.z());
}


// Returns vector data at absolute index
//
// i - absolute 1D index
// return - vector at index
Eigen::Vector3d& SphericalVectorField::operator()(size_t i) {
	return data[i];
}


// Returns vector data at absolute index
//
// i - absolute 1D index
// return - vector at index
const Eigen::Vector3d& SphericalVectorField::operator()(size_t i) const {
	return data[i];
}


// Returns vector data at lat, long, level index
//
// lat - latitude index
// lng - longitude index
// lvl - level index
// return - vector at index
Eigen::Vector3d& SphericalVectorField::operator()(size_t lat, size_t lng, size_t lvl) {
	return data[indexToOffset(lat, lng, lvl)];
}


// Returns vector data at lat, long, level index
//
// lat - latitude index
// lng - longitude index
// lvl - level index
// return - vector at index
const Eigen::Vector3d& SphericalVectorField::operator()(size_t lat, size_t lng, size_t lvl) const {
	return data[indexToOffset(lat, lng, lvl)];
}


// Returns vector data at lat, long, level index
//
// i - (lat, long, level) indices
// return - vector at index
Eigen::Vector3d& SphericalVectorField::operator()(const Eigen::Matrix<size_t, 3, 1>& i) {
	return operator()(i.x(), i.y(), i.z());
}


// Returns vector data at lat, long, level index
//
// i - (lat, long, level) indices
// return - vector at index
const Eigen::Vector3d& SphericalVectorField::operator()(const Eigen::Matrix<size_t, 3, 1>& i) const {
	return operator()(i.x(), i.y(), i.z());
}