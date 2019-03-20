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


std::vector<Eigen::Vector3i> SphericalVectorField::findCriticalPoints() {

	std::vector<Eigen::Vector3i> points;

	for (size_t lvl = 0; lvl < NUM_LEVELS - 1; lvl++) {
		for (size_t lat = 0; lat < NUM_LATS - 1; lat++) {
			for (size_t lng = 0; lng < NUM_LONGS - 1; lng++) {

				Eigen::Vector4d v0, v1, v2, v3, v4, v5, v6, v7;
				v0 << (*this)(lvl, lat, lng), 1.0;
				v1 << (*this)(lvl + 1, lat, lng), 1.0;
				v2 << (*this)(lvl + 1, lat + 1, lng), 1.0;
				v3 << (*this)(lvl + 1, lat, (lng + 1) % NUM_LONGS), 1.0;

				v4 << (*this)(lvl, lat + 1, (lng + 1) % NUM_LONGS), 1.0;
				v5 << (*this)(lvl + 1, lat + 1, (lng + 1) % NUM_LONGS), 1.0;
				v6 << (*this)(lvl, lat, (lng + 1) % NUM_LONGS), 1.0;
				v7 << (*this)(lvl, lat + 1, lng), 1.0;

				Eigen::Matrix4d a1, a2, a3, a4, a5;
				a1 << v0, v1, v2, v3;
				a2 << v4, v5, v3, v2;
				a3 << v0, v7, v4, v2;
				a4 << v0, v6, v4, v3;
				a5 << v2, v3, v0, v4;		
				
				Eigen::Vector4d b(0.0, 0.0, 0.0, 1.0);

				Eigen::Vector4d x1 = a1.partialPivLu().solve(b);
				Eigen::Vector4d x2 = a2.partialPivLu().solve(b);
				Eigen::Vector4d x3 = a3.partialPivLu().solve(b);
				Eigen::Vector4d x4 = a4.partialPivLu().solve(b);
				Eigen::Vector4d x5 = a5.partialPivLu().solve(b);

				if ((x1.array() >= 0.0).all()) {
					points.push_back(Eigen::Vector3i(lvl, lat, lng));
				}
				else if ((x2.array() >= 0.0).all()) {
					points.push_back(Eigen::Vector3i(lvl, lat, lng));
				}
				else if ((x3.array() >= 0.0).all()) {
					points.push_back(Eigen::Vector3i(lvl, lat, lng));
				}
				else if ((x4.array() >= 0.0).all()) {
					points.push_back(Eigen::Vector3i(lvl, lat, lng));
				}
				else if ((x5.array() >= 0.0).all()) {
					points.push_back(Eigen::Vector3i(lvl, lat, lng));
				}
			}
		}
	}
	return points;
}


Eigen::Vector3d& SphericalVectorField::operator()(size_t lvl, size_t lat, size_t lng) {
	return data[lng + NUM_LONGS * (lat + NUM_LATS * lvl)];
}


const Eigen::Vector3d& SphericalVectorField::operator()(size_t lvl, size_t lat, size_t lng) const {
	return data[lng + NUM_LONGS * (lat + NUM_LATS * lvl)];
}
