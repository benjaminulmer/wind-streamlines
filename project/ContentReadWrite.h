#pragma once

#include <rapidjson/document.h>


// Namespace for reading and writing files of different formats
namespace ContentReadWrite {

	rapidjson::Document readJSON(const char* path);
};

