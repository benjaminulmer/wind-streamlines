#pragma once

class ColourRenderable;

#define RAPIDJSON_NOMEMBERITERATORCLASS
#include <rapidjson/document.h>


// Namespace for reading and writing files of different formats
namespace ContentReadWrite {

	rapidjson::Document readJSON(const char* path);
	bool loadOBJ(const char* path, ColourRenderable& r);
};

