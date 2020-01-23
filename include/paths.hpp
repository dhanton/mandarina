#pragma once

//we use strings instead of defines to be able to concatenate between them
#include <string>

const std::string DATA_PATH = "../data/";

const std::string TEXTURES_PATH = DATA_PATH + "textures/";
const std::string JSON_PATH = DATA_PATH + "json/";
const std::string MAPS_PATH = DATA_PATH + "maps/";

#ifdef MANDARINA_DEBUG
    const std::string MAP_FILENAME_EXT =  "png";
#else
    const std::string MAP_FILENAME_EXT =  "map";
#endif
