#pragma once

#include <SFML/Graphics/Image.hpp>
#include <vector>
#include "defines.hpp"

enum TileType {
    TILE_NONE,
    TILE_BLOCK,
    TILE_DESTRUCTABLE_BLOCK,
    TILE_BUSH,

    TILE_MAX_TYPES
};

#define DEFAULT_TILE_SIZE 16

class TileMap
{
public:
    TileMap(size_t tileSize = DEFAULT_TILE_SIZE);

    void loadFromFile(const std::string& file);

    TileType getTile(size_t i, size_t j) const;

    Vector2u getSize() const;
    size_t getTileSize() const;

private:
    Vector2u m_size;
    size_t m_tileSize;

    //we use u8 to keep it tight in memory
    std::vector<std::vector<u8>> m_tiles;
};