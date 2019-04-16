#pragma once

#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <rapidjson/document.h>

#include <vector>


enum class Shader {
	DEFAULT,
	STREAMLINE
};

class Renderable {

public:
	virtual void assignBuffers() = 0;
	virtual void setBufferData() = 0;
	virtual void deleteBufferData() = 0;

	GLuint getVAO() const { return vao; }
	virtual Shader getShaderType() const = 0;
	virtual void render() const = 0;

protected:
	GLuint vao;
};


class DoublePrecisionRenderable : public Renderable {

public: 
	virtual void addVert(const glm::dvec3& v);

	virtual void assignBuffers();
	virtual void setBufferData();
	virtual void deleteBufferData();

protected:
	std::vector<glm::vec3> vertsHigh;
	std::vector<glm::vec3> vertsLow;

	GLuint vertexHighBuffer;
	GLuint vertexLowBuffer;
};


class ColourRenderable : public DoublePrecisionRenderable {

public:
	ColourRenderable() = default;
	ColourRenderable(const rapidjson::Document& d);

	virtual void addColour(const glm::vec3& c) { colours.push_back(c); }

	virtual void assignBuffers();
	virtual void setBufferData();
	virtual void deleteBufferData();

	virtual Shader getShaderType() const { return Shader::DEFAULT; }
	virtual void render() const;

	void setDrawMode(GLuint mode) { drawMode = mode; }

protected:
	GLuint drawMode;

	std::vector<glm::vec3> colours;

	GLuint colourBuffer;
};


class StreamlineRenderable : public ColourRenderable {

public:
	virtual void addTangent(const glm::vec3& t) { tangents.push_back(t); }
	virtual void addLocalTime(float localTime) { localTimes.push_back(localTime); }

	virtual void assignBuffers();
	virtual void setBufferData();
	virtual void deleteBufferData();

	virtual Shader getShaderType() const { return Shader::STREAMLINE; }

	void clear() {
		localTimes.clear();
		tangents.clear();
		colours.clear();
		vertsHigh.clear();
		vertsLow.clear();
	}

//private:
	std::vector<glm::vec3> tangents;
	std::vector<float> localTimes;

	GLuint tangentBuffer;
	GLuint timeBuffer;
};