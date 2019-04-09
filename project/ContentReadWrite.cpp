#include "ContentReadWrite.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

// Load obj file from path into indexed list data structure
bool ContentReadWrite::loadOBJ(const char* path, Renderable& r) {
	return false;
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