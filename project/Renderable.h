#pragma once

#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <rapidjson/document.h>

#include <vector>

struct IndexedLists {
	std::vector<GLushort> vertIndices;
	std::vector<GLushort> normalIndices;
	std::vector<GLushort> uvIndices;

	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
};

class Renderable {

public:
	Renderable() : lineColour(glm::vec3(1.f, 1.f, 1.f)), vao(0), vertexHighBuffer(0), vertexLowBuffer(0), colourBuffer(0), uvBuffer(0), textureID(0), fade(true), drawMode(0) {}
	Renderable(const rapidjson::Document& d);

	void doubleToFloats();

	std::vector<glm::dvec3> verts;

	std::vector<glm::vec3> vertsHigh;
	std::vector<glm::vec3> vertsLow;
	std::vector<glm::vec3> colours;
	std::vector<glm::vec2> uvs;

	//glm::mat4 model;

	glm::vec3 lineColour;

	GLuint vao;
	GLuint vertexHighBuffer;
	GLuint vertexLowBuffer;
	GLuint colourBuffer;
	GLuint uvBuffer;
	GLuint textureID;

	bool fade;

	GLuint drawMode;
};

