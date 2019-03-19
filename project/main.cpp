#include "SphericalVectorField.h"

#include <iostream>


int main() {

	// Playing with netCDF format
	netCDF::NcFile file("data/2018-05-27T12.nc", netCDF::NcFile::read);

	SphericalVectorField f(file);



	// Playing with eigen3
	//Eigen::MatrixXd m(2, 2);
	//m(0, 0) = 3;
	//m(1, 0) = 2.5;
	//m(0, 1) = -1;
	//m(1, 1) = m(1, 0) + m(0, 1);
	//std::cout << m << std::endl;

	//Eigen::Matrix4d a;
	//a << 0.0, -0.1, 0.2, 0.0,
	//	 1.0, -1.0, -0.5, -0.5,
	//	 0.0, 0.2, 0.1, -0.3,
	//	 1.0, 1.0, 1.0, 1.0;
	//Eigen::Vector4d b(0.0, 0.0, 0.0, 1.0);

	//Eigen::Vector4d x = a.fullPivLu().solve(b);
	//std::cout << x << std::endl;

	system("pause");
}

