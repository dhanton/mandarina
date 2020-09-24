#pragma once

//we use strings instead of defines to be able to concatenate between them
#include <string>

//strings cannot be constexpr as of C++11

const std::string DATA_PATH = "../data/";

const std::string TEXTURES_PATH = DATA_PATH + "textures/";
const std::string ICONS_PATH = TEXTURES_PATH + "icons/";
const std::string FONTS_PATH = DATA_PATH + "fonts/";
const std::string JSON_PATH = DATA_PATH + "json/";
const std::string MAPS_PATH = DATA_PATH + "maps/";

// #ifdef MANDARINA_DEBUG

    const std::string MAP_FILENAME_EXT =  "png";

// #else
//Is it possible to load png with a different extension name in SFML?
//     const std::string MAP_FILENAME_EXT =  "map";
// #endif
