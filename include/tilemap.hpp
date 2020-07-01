#pragma once

#include <SFML/Graphics/Image.hpp>
#include <vector>
#include <list>
#include "defines.hpp"
#include "bounding_body.hpp"

enum TileType {
    TILE_NONE  = 0b0001,
    TILE_BLOCK = 0b0010,
    TILE_WALL  = 0b0100,
    TILE_BUSH  = 0b1000,

    TILE_MAX_TYPES
};

#define DEFAULT_TILE_SIZE 16
#define TILE_SCALE 4

class TileMap
{
public:
    TileMap(u16 tileSize = DEFAULT_TILE_SIZE, u16 tileScale = TILE_SCALE);

    void loadFromFile(const std::string& filename);
    std::list<Vector2> loadSpawnPoints(const std::string& filename);

    bool isColliding(u16 tileFlags, const Circlef& circle) const;
    bool isContained(u16 tileFlags, const Circlef& circle) const;
    bool isOutsideMap(const Circlef& circle) const;

    u16 getCollidingTile(const Circlef& circle) const;

    bool getCollidingTileRect(u16 tileFlags, const Circlef& circle, sf::FloatRect& tileRect) const;

    TileType getTile(u16 i, u16 j) const;

    Vector2u getSize() const;
    u16 getTileSize() const;
    u16 getTileScale() const;

    //total size of the map in pixels
    Vector2u getWorldSize() const;

private:
    bool _collidingContained_impl(bool contained, u16 tileFlags, const Circlef& circle, sf::FloatRect* rect, u16* getTiles) const;

    Vector2u m_size;
    u16 m_tileSize;
    u16 m_tileScale;

    //we use u16 to keep it tight in memory
    std::vector<std::vector<u16>> m_tiles;
};
