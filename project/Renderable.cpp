#define _USE_MATH_DEFINES
#include "Renderable.h"

#include "Conversions.h"

#include <algorithm>
#include <cmath>
#include <limits>

// Creates a renderable of the geometry specified in json document
Renderable::Renderable(const rapidjson::Document& d) : Renderable() {
	drawMode = GL_LINES;

	const rapidjson::Value& featuresArray = d["features"];

	// Loop over lines in data file
	for (rapidjson::SizeType i = 0; i < featuresArray.Size(); i++) {
		const rapidjson::Value& coordArray = featuresArray[i]["geometry"]["coordinates"];

		for (rapidjson::SizeType j = 0; j < coordArray.Size(); j++) {
			double lng = coordArray[j][0].GetDouble() * M_PI / 180.0;
			double lat = coordArray[j][1].GetDouble() * M_PI / 180.0;

			verts.push_back(glm::dvec3(sin(lng)*cos(lat), sin(lat), cos(lng)*cos(lat)) * RADIUS_EARTH_M);
			colours.push_back(glm::vec3(0.f));

			if (j != 0 && j != coordArray.Size() - 1) {
				verts.push_back(glm::dvec3(sin(lng)*cos(lat), sin(lat), cos(lng)*cos(lat)) * RADIUS_EARTH_M);
				colours.push_back(glm::vec3(0.f));
			}
		}
	}
}

void Renderable::doubleToFloats() {

	for (const glm::dvec3& v : verts) {

		glm::vec3 high = v;

		vertsHigh.push_back(high);
		vertsLow.push_back(v - (glm::dvec3)high);
	}
}