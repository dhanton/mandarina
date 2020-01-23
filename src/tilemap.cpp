#include "tilemap.hpp"

#include <iostream>

TileMap::TileMap(size_t tileSize)
{
    m_tileSize = tileSize;
}

void TileMap::loadFromFile(const std::string& file)
{
    sf::Image image;

    if (!image.loadFromFile(file)) {
        std::cout << "TileMap::loadFromFile error - Invalid filename" << std::endl;
        return;
    }

    m_size = image.getSize();

    m_tiles.clear();
    m_tiles.resize(m_size.x);

    for (int i = 0; i < m_size.x; ++i) {
        m_tiles[i].resize(m_size.y);

        for (int j = 0; j < m_size.y; ++j) {
            sf::Color pixel = image.getPixel(i, j);

            m_tiles[i][j] = TILE_NONE;

            if (pixel == sf::Color::Black) {
                m_tiles[i][j] = TILE_BLOCK;

            } else if (pixel == sf::Color::Blue) {
                m_tiles[i][j] = TILE_DESTRUCTABLE_BLOCK;

            } else if (pixel == sf::Color::Green) {
                m_tiles[i][j] = TILE_BUSH;
            }
        }
    }
}

TileType TileMap::getTile(size_t i, size_t j) const
{
    return static_cast<TileType>(m_tiles[i][j]);
}

Vector2u TileMap::getSize() const
{
    return m_size;
}

size_t TileMap::getTileSize() const
{
    return m_tileSize;
}