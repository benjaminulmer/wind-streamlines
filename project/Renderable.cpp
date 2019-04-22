#define _USE_MATH_DEFINES
#include "Renderable.h"

#include "Conversions.h"


// Construct renderable from geometry specified in json document
//
// d - json data
ColourRenderable::ColourRenderable(const rapidjson::Document& d) {

	drawMode = GL_LINES;
	const rapidjson::Value& featuresArray = d["features"];

	// Loop over lines in data file
	for (rapidjson::SizeType i = 0; i < featuresArray.Size(); i++) {
		const rapidjson::Value& coordArray = featuresArray[i]["geometry"]["coordinates"];

		for (rapidjson::SizeType j = 0; j < coordArray.Size(); j++) {
			double lng = coordArray[j][0].GetDouble() * M_PI / 180.0;
			double lat = coordArray[j][1].GetDouble() * M_PI / 180.0;

			addVert(glm::dvec3(sin(lng)*cos(lat), sin(lat), cos(lng)*cos(lat)) * RADIUS_EARTH_M);
			colours.push_back(glm::u8vec3(0, 0, 0));

			if (j != 0 && j != coordArray.Size() - 1) {
				addVert(glm::dvec3(sin(lng)*cos(lat), sin(lat), cos(lng)*cos(lat)) * RADIUS_EARTH_M);
				colours.push_back(glm::u8vec3(0, 0, 0));
			}
		}
	}
}


// Add vertex to list of verts
//
// v - vertex to add
void DoublePrecisionRenderable::addVert(const glm::dvec3 & v) {

	// Split into high and low precision components
	glm::vec3 high = v;

	vertsHigh.push_back(high);
	vertsLow.push_back(v - (glm::dvec3)high);
}


// Assign GPU buffers for object
void DoublePrecisionRenderable::assignBuffers() {

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Vertex high buffer
	glGenBuffers(1, &vertexHighBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexHighBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	// Vertex low buffer
	glGenBuffers(1, &vertexLowBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexLowBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
}


// Set data in GPU buffers
void DoublePrecisionRenderable::setBufferData() {

	// Vertex high buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexHighBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertsHigh.size(), vertsHigh.data(), GL_DYNAMIC_DRAW);

	// Vertex low buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexLowBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertsLow.size(), vertsLow.data(), GL_DYNAMIC_DRAW);
}


// Delete GPU buffers for object
void DoublePrecisionRenderable::deleteBufferData() {

	glDeleteBuffers(1, &vertexHighBuffer);
	glDeleteBuffers(1, &vertexLowBuffer);

	glDeleteVertexArrays(1, &vao);
}


// Assign GPU buffers for object
void ColourRenderable::assignBuffers() {

	DoublePrecisionRenderable::assignBuffers();

	// Colour buffer
	glGenBuffers(1, &colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
	glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
	glEnableVertexAttribArray(2);
}


// Set data in GPU buffers
void ColourRenderable::setBufferData() {

	DoublePrecisionRenderable::setBufferData();

	// Colour buffer
	glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::u8vec3)*colours.size(), colours.data(), GL_DYNAMIC_DRAW);
}


// Delete GPU buffers for object
void ColourRenderable::deleteBufferData() {

	glDeleteBuffers(1, &colourBuffer);
	DoublePrecisionRenderable::deleteBufferData();

}


// Make the appropriate OpenGL render call for the object
void ColourRenderable::render() const {
	glDrawArrays(drawMode, 0, (GLsizei)vertsHigh.size());
}


// Assign GPU buffers for object
void StreamlineRenderable::assignBuffers() {

	ColourRenderable::assignBuffers();

	// Tangent buffer
	glGenBuffers(1, &tangentBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(3);

	// Time buffer
	glGenBuffers(1, &timeBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, timeBuffer);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(4);
}


// Set data in GPU buffers
void StreamlineRenderable::setBufferData() {

	ColourRenderable::setBufferData();

	// Tangent buffer
	glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*tangents.size(), tangents.data(), GL_DYNAMIC_DRAW);

	// Time buffer
	glBindBuffer(GL_ARRAY_BUFFER, timeBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*localTimes.size(), localTimes.data(), GL_DYNAMIC_DRAW);
}


// Delete GPU buffers for object
void StreamlineRenderable::deleteBufferData() {

	glDeleteBuffers(1, &timeBuffer);
	glDeleteBuffers(1, &tangentBuffer);
	ColourRenderable::deleteBufferData();
}