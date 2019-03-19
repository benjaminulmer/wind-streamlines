#include "SphericalVectorField.h"

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
			std::cout << dim.getName() << std::endl;
			size *= dim.getSize();
		}
		std::cout << std::endl;

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


Eigen::Vector3d & SphericalVectorField::operator()(size_t level, size_t lat, size_t lng) {
	return data[lng + longs.size() * (lat + lats.size() * level)];
}

const Eigen::Vector3d & SphericalVectorField::operator()(size_t level, size_t lat, size_t lng) const {
	return data[lng + longs.size() * (lat + lats.size() * level)];
}
