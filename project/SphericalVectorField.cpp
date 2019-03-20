#include "SphericalVectorField.h"

// TODO this better
SphericalVectorField::SphericalVectorField(const netCDF::NcFile& file) {

	// Get list of variables in file
	std::multimap<std::string, netCDF::NcVar> vars = file.getVars();

	std::vector<std::vector<double>> varDatas;
	for (std::pair<std::string, netCDF::NcVar> p : vars) {

		// Get all the dimensions and attributes
		std::vector<netCDF::NcDim> dims = p.second.getDims();
		std::map<std::string, netCDF::NcVarAtt> atts = p.second.getAtts();

		// Calculate total size (multiply size of each dim)
		size_t size = 1;
		for (netCDF::NcDim dim : dims) {
			size *= dim.getSize();
		}

		// Grab raw data from file
		double* dataArry = new double[size];
		p.second.getVar(dataArry);

		// Get scale factor and add offset if they exist
		double scaleFactor = 1.0;
		double addOffset = 0.0;

		auto scaleFactorIt = atts.find("scale_factor");
		if (scaleFactorIt != atts.end()) {
			(*scaleFactorIt).second.getValues(&scaleFactor);
		}

		auto addOffsetIt = atts.find("add_offset");
		if (addOffsetIt != atts.end()) {
			(*addOffsetIt).second.getValues(&addOffset);
		}

		// Apply scale factor and add offset
		for (int i = 0; i < size; i++) {
			dataArry[i] = dataArry[i] * scaleFactor + addOffset;
		}

		varDatas.push_back(std::vector<double>(dataArry, dataArry + size));
		delete[] dataArry;
	}

	for (size_t i = 0; i < varDatas[4].size(); i++) {
		data.push_back(Eigen::Vector3d(varDatas[4][i], varDatas[5][i], varDatas[6][i]));
	}
	lats = varDatas[0];
	levels = varDatas[1];
	longs = varDatas[2];
}


void SphericalVectorField::loopOverCells() {

	int found = 0;
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


				Eigen::Vector4d x1 = a1.fullPivLu().solve(b);
				Eigen::Vector4d x2 = a2.fullPivLu().solve(b);
				Eigen::Vector4d x3 = a3.fullPivLu().solve(b);
				Eigen::Vector4d x4 = a4.fullPivLu().solve(b);

				if ((x1.array() >= 0.0).all() || (x2.array() >= 0.0).all() || (x3.array() >= 0.0).all() || (x4.array() >= 0.0).all()) {
					//std::cout << x << std::endl << std::endl;
					found++;
				} 
			}
		}
	}
	int cells = (NUM_LEVELS - 1) * (NUM_LATS - 1) * NUM_LONGS;
	std::cout << "found: " << found << std::endl << "cells " << cells << std::endl;

}


Eigen::Vector3d & SphericalVectorField::operator()(size_t lvl, size_t lat, size_t lng) {
	return data[lng + NUM_LONGS * (lat + NUM_LATS * lvl)];
}


const Eigen::Vector3d & SphericalVectorField::operator()(size_t lvl, size_t lat, size_t lng) const {
	return data[lng + longs.size() * (lat + lats.size() * lvl)];
}
