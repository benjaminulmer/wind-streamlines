#include "SphericalVectorField.h"


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
	int* levelArry = new int[NUM_LEVELS];
	float* latArry = new float[NUM_LATS];
	float* longArry = new float[NUM_LONGS];

	file.getVar("level").getVar(levelArry);
	file.getVar("latitude").getVar(latArry);
	file.getVar("longitude").getVar(longArry);

	levels.assign(levelArry, levelArry + NUM_LEVELS);
	lats.assign(latArry, latArry + NUM_LATS);
	longs.assign(longArry, longArry + NUM_LONGS);

	delete[] levelArry;
	delete[] latArry;
	delete[] longArry;

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

	// Apply scale and offset and add to data vector
	for (size_t i = 0; i < size; i++) {
		double u = uArry[i] * uScale + uOffset;
		double v = vArry[i] * vScale + vOffset;
		double w = wArry[i] * wScale + wOffset;

		data[i] = (Eigen::Vector3d(u, v, w));
	}

	delete[] uArry;
	delete[] vArry;
	delete[] wArry;
}


// Finds all critical points in the vector field and their Poincare index
//
// return - list of indicies of cells that contain critical points and their Poincare index
std::vector<std::pair<Eigen::Vector3i, int>> SphericalVectorField::findCriticalPoints() {

	std::vector<std::pair<Eigen::Vector3i, int>> points;

	for (size_t lvl = 0; lvl < NUM_LEVELS - 1; lvl++) {
		for (size_t lat = 0; lat < NUM_LATS - 1; lat++) {
			for (size_t lng = 0; lng < NUM_LONGS - 1; lng++) {

				// 8 vertices of hex
				int i0 = indexToOffset(lat, lng, lvl);
				int i1 = indexToOffset(lat, lng, lvl + 1);
				int i2 = indexToOffset(lat + 1, lng, lvl + 1);
				int i3 = indexToOffset(lat, (lng + 1) % NUM_LONGS, lvl + 1);
				int i4 = indexToOffset(lat + 1, (lng + 1) % NUM_LONGS, lvl);
				int i5 = indexToOffset(lat + 1, (lng + 1) % NUM_LONGS, lvl + 1);
				int i6 = indexToOffset(lat, (lng + 1) % NUM_LONGS, lvl);
				int i7 = indexToOffset(lat + 1, lng, lvl);
				
				// Construct 5 tets from hex and test each one to see if it has a critical point
				int pi;
				if ((pi = criticalPointInTet(i0, i1, i2, i3)) != 0) {
					Eigen::Vector3i i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
				}
				else if ((pi = criticalPointInTet(i4, i5, i3, i2)) != 0) {
					Eigen::Vector3i i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
				}
				else if ((pi = criticalPointInTet(i0, i7, i4, i2)) != 0) {
					Eigen::Vector3i i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
				}
				else if ((pi = criticalPointInTet(i0, i6, i4, i3)) != 0) {
					Eigen::Vector3i i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
				}
				else if ((pi = criticalPointInTet(i2, i3, i0, i4)) != 0) {
					Eigen::Vector3i i(lat, lng, lvl);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
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
int SphericalVectorField::sign(const Eigen::Vector4d& v0, const Eigen::Vector4d& v1,
                               const Eigen::Vector4d& v2, const Eigen::Vector4d& v3,
                               size_t i0, size_t i1, size_t i2, size_t i3) {
	
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
int SphericalVectorField::criticalPointInTet(size_t i0, size_t i1, size_t i2, size_t i3) {

	// Get vector field values
	Eigen::Vector4d v0, v1, v2, v3;
	v0 << data[i0], 1.0;
	v1 << data[i1], 1.0;
	v2 << data[i2], 1.0;
	v3 << data[i3], 1.0;

	// Test if tet contains critical point
	Eigen::Vector4d zeroPoint(0.0, 0.0, 0.0, 1.0);
	int simplexSign = sign(zeroPoint, v1, v2, v3, i0, i1, i2, i3);

	if (sign(v0, zeroPoint, v2, v3, i0, i1, i2, i3) != simplexSign) {
		return 0;
	}
	if (sign(v0, v1, zeroPoint, v3, i0, i1, i2, i3) != simplexSign) {
		return 0;
	}
	if (sign(v0, v1, v2, zeroPoint, i0, i1, i2, i3) != simplexSign) {
		return 0;
	}

	// Contains critical point, check Poincare index
	Eigen::Vector4d p0, p1, p2, p3;
	p0 << sphCoords(i0), 1.0;
	p1 << sphCoords(i1), 1.0;
	p2 << sphCoords(i2), 1.0;
	p3 << sphCoords(i3), 1.0;

	if (sign(p0, p1, p2, p3, i0, i1, i2, i3) != simplexSign) {
		return -1;
	}
	else {
		return 1;
	}
}


// Returns the spherical coordinates of the grid point at absolute index i
//
// i - absolute 1D index
// return - (lat, long, altitude) in degrees and meters
Eigen::Vector3d SphericalVectorField::sphCoords(size_t i) const {
	return sphCoords(offsetToIndex(i));
}


// Returns the spherical coordinates of the grid point at lat, long, level index
//
// lat - latitude index
// lng - longitude index
// lvl - level index
// return - (lat, long, altitude) in degrees and meters
Eigen::Vector3d SphericalVectorField::sphCoords(size_t lat, size_t lng, size_t lvl) const {
	return Eigen::Vector3d(lats[lat], longs[lng], mbToMeters(levels[lvl]));
}


// Returns the spherical coordinates of the grid at (lat, long, level) index
//
// i - (lat, long, level) indices
// return - (lat, long, altitude) in degrees and meters
Eigen::Vector3d SphericalVectorField::sphCoords(const Eigen::Vector3i& i) const {
	return sphCoords(i(0), i(1), i(2));
}


// Converts absolute index to lat, long, level index
//
// i - absolute 1D index
// return - (lat, long, level) index
Eigen::Vector3i SphericalVectorField::offsetToIndex(size_t i) const {

	Eigen::Vector3i v;
	v(0) = (i / NUM_LONGS) % NUM_LATS;
	v(1) = i % NUM_LONGS;
	v(2) = (i / NUM_LONGS) / NUM_LATS;

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
size_t SphericalVectorField::indexToOffset(const Eigen::Vector3i& i) const {
	return indexToOffset(i(0), i(1), i(2));
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
Eigen::Vector3d& SphericalVectorField::operator()(const Eigen::Vector3i& i) {
	return operator()(i(0), i(1), i(2));
}


// Returns vector data at lat, long, level index
//
// i - (lat, long, level) indices
// return - vector at index
const Eigen::Vector3d& SphericalVectorField::operator()(const Eigen::Vector3i& i) const {
	return operator()(i(0), i(1), i(2));
}