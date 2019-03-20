#include "ContentReadWrite.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

// Load obj file from path into indexed list data structure
bool ContentReadWrite::loadOBJ(const char* path, Renderable& r) {
	printf("Loading OBJ file %s...\n", path);

	FILE * file; 
	file = fopen(path, "r");
	if( file == NULL ){
		printf("Cannot open file. Check path.");
		getchar();
		return false;
	}

	std::vector<unsigned int> vertIndices;
	std::vector<unsigned int> normalIndices;
	std::vector<unsigned int> uvIndices;

	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;

	while(true){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			verts.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; 
			uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );

			vertIndices.push_back(vertexIndex[0]-1);
			vertIndices.push_back(vertexIndex[1]-1);
			vertIndices.push_back(vertexIndex[2]-1);
			normalIndices.push_back(normalIndex[0]-1);
			normalIndices.push_back(normalIndex[1]-1);
			normalIndices.push_back(normalIndex[2]-1);
			uvIndices.push_back(uvIndex[0] - 1);
			uvIndices.push_back(uvIndex[1] - 1);
			uvIndices.push_back(uvIndex[2] - 1);

		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}
	
	for( unsigned int i=0; i<vertIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertIndices[i];
		unsigned int normalIndex = normalIndices[i];
		unsigned int uvIndex = uvIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = verts[ vertexIndex ];
		glm::vec3 normal = normals[ normalIndex ];
		glm::vec2 uv = uvs[ uvIndex ];

		// Put the attributes in buffers
		r.verts.push_back(vertex);
		r.colours.push_back(normal);
		r.uvs.push_back(uv);
	}
	r.drawMode = GL_TRIANGLES;
	fclose(file);
	return true;
}

// Reads in explosion graph from file
rapidjson::Document ContentReadWrite::readJSON(std::string path) {

	// Open file
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "Could not open file" << std::endl;
		return nullptr;
	}

	// Get file length
	file.seekg(0, file.end);
	int length = (int) file.tellg();
	file.seekg(0, file.beg);

	// Read file into buffer and parse
	char* buffer = new char[length + 1];
	file.read(buffer, length);
	file.close();
	buffer[length] = 0;

	// Create JSON document
	rapidjson::Document d;
	rapidjson::ParseResult ok = d.Parse<rapidjson::kParseStopWhenDoneFlag>(buffer);

	if (!ok) {
		rapidjson::ParseErrorCode error = ok.Code();
		std::cout << "error parsing JSON file: " << error << std::endl;
	}
	delete[] buffer;

	return d;
}