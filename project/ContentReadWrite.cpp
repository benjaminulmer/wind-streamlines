#include "ContentReadWrite.h"

#include <fstream>
#include <iostream>


// Reads in json data from file
//
// path - path of json file to read
// return - rapidjson document containing file data
rapidjson::Document ContentReadWrite::readJSON(const char* path) {

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