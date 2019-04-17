#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#define RAPIDJSON_NOMEMBERITERATORCLASS
#include <rapidjson/document.h>

#include <vector>


// Enumeration of different shaders
enum class Shader {
	DEFAULT,
	STREAMLINE
};


// Abstract class to represent object that can be rendered
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


// Class for renderable that uses double precision for vertex locations
class DoublePrecisionRenderable : public Renderable {

public: 
	virtual void addVert(const glm::dvec3& v);

	virtual size_t size() { return vertsHigh.size(); }

	virtual void assignBuffers();
	virtual void setBufferData();
	virtual void deleteBufferData();

protected:
	std::vector<glm::vec3> vertsHigh;
	std::vector<glm::vec3> vertsLow;

	GLuint vertexHighBuffer;
	GLuint vertexLowBuffer;
};


// Class for renderable that has colour data for each vertex
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


// Class for renderable that is used for streamlines. Each vertex has a tangent and integration time
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

private:
	std::vector<glm::vec3> tangents;
	std::vector<float> localTimes;

	GLuint tangentBuffer;
	GLuint timeBuffer;
};