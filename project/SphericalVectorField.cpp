#include "SphericalVectorField.h"

// TODO this better
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


std::vector<std::pair<Eigen::Vector3i, int>> SphericalVectorField::findCriticalPoints() {

	std::vector<std::pair<Eigen::Vector3i, int>> points;

	for (size_t lvl = 0; lvl < NUM_LEVELS - 1; lvl++) {
		for (size_t lat = 0; lat < NUM_LATS - 1; lat++) {
			for (size_t lng = 0; lng < NUM_LONGS - 1; lng++) {

				int i0 = multiIndexToIndex(lvl, lat, lng);
				int i1 = multiIndexToIndex(lvl + 1, lat, lng);
				int i2 = multiIndexToIndex(lvl + 1, lat + 1, lng);
				int i3 = multiIndexToIndex(lvl + 1, lat, (lng + 1) % NUM_LONGS);
				int i4 = multiIndexToIndex(lvl, lat + 1, (lng + 1) % NUM_LONGS);
				int i5 = multiIndexToIndex(lvl + 1, lat + 1, (lng + 1) % NUM_LONGS);
				int i6 = multiIndexToIndex(lvl, lat, (lng + 1) % NUM_LONGS);
				int i7 = multiIndexToIndex(lvl, lat + 1, lng);
				
				int pi;
				if ((pi = criticalPointInSimplex(i0, i1, i2, i3)) != 0) {
					Eigen::Vector3i i(lvl, lat, lng);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
					//points.push_back(Eigen::Vector3i(lvl, lat, lng));
				}
				else if ((pi = criticalPointInSimplex(i4, i5, i3, i2)) != 0) {
					Eigen::Vector3i i(lvl, lat, lng);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
				}
				else if ((pi = criticalPointInSimplex(i0, i7, i4, i2)) != 0) {
					Eigen::Vector3i i(lvl, lat, lng);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
				}
				else if ((pi = criticalPointInSimplex(i0, i6, i4, i3)) != 0) {
					Eigen::Vector3i i(lvl, lat, lng);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
				}
				else if ((pi = criticalPointInSimplex(i2, i3, i0, i4)) != 0) {
					Eigen::Vector3i i(lvl, lat, lng);
					points.push_back(std::pair<Eigen::Vector3i, int>(i, pi));
				}
			}
		}
	}
	return points;
}

int SphericalVectorField::sign(const Eigen::Vector4d& v0, const Eigen::Vector4d& v1,
                               const Eigen::Vector4d& v2, const Eigen::Vector4d& v3,
                               size_t i0, size_t i1, size_t i2, size_t i3) {
	
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
	if (det == 0.0) std::cout << "zero" << std::endl;
	int detSign = (det <= 0) ? -1 : 1;

	return (invCount % 2 == 0) ? detSign : -detSign;
}

int SphericalVectorField::criticalPointInSimplex(size_t i0, size_t i1, size_t i2, size_t i3) {

	Eigen::Vector4d v0, v1, v2, v3;
	v0 << data[i0], 1.0;
	v1 << data[i1], 1.0;
	v2 << data[i2], 1.0;
	v3 << data[i3], 1.0;

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

	size_t i0lng = i0 % NUM_LONGS;
	size_t i1lng = i1 % NUM_LONGS;
	size_t i2lng = i2 % NUM_LONGS;
	size_t i3lng = i3 % NUM_LONGS;

	size_t i0lat = (i0 / NUM_LONGS) % NUM_LATS;
	size_t i1lat = (i1 / NUM_LONGS) % NUM_LATS;
	size_t i2lat = (i2 / NUM_LONGS) % NUM_LATS;
	size_t i3lat = (i3 / NUM_LONGS) % NUM_LATS;

	size_t i0lvl = ((i0 / NUM_LONGS) / NUM_LATS);
	size_t i1lvl = ((i1 / NUM_LONGS) / NUM_LATS);
	size_t i2lvl = ((i2 / NUM_LONGS) / NUM_LATS);
	size_t i3lvl = ((i3 / NUM_LONGS) / NUM_LATS);

	Eigen::Vector4d p0, p1, p2, p3;
	p0 << indexToCoords(i0lvl, i0lat, i0lng), 1.0;
	p1 << indexToCoords(i1lvl, i1lat, i1lng), 1.0;
	p2 << indexToCoords(i2lvl, i2lat, i2lng), 1.0;
	p3 << indexToCoords(i3lvl, i3lat, i3lng), 1.0;

	if (sign(p0, p1, p2, p3, i0, i1, i2, i3) != simplexSign) {
		return -1;
	}
	else {
		return 1;
	}
}