#pragma once

#include <GL/glew.h>

#include <fstream>


// Class for compiling and managing GLSL shaders
class ShaderTools {

public:
	static GLuint compileShaders(const char* vertexFilename, const char* fragmentFilename);
	static GLuint compileShaders(const char* vertexFilename, const char* geometryFilename, const char* fragmentFilename);

private:
	static unsigned long getFileLength(std::ifstream& file);
	static GLchar* loadshader(std::string filename);
	static void unloadshader(GLchar** shaderSource );
};
