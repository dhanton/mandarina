#include "tilemap.hpp"

#include <iostream>

TileMap::TileMap(u16 tileSize, u16 tileScale)
{
    m_tileSize = tileSize;
    m_tileScale = tileScale;
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
                m_tiles[i][j] = TILE_WALL;

            } else if (pixel == sf::Color::Red) {
                m_tiles[i][j] = TILE_BLOCK;

            } else if (pixel == sf::Color::Green) {
                m_tiles[i][j] = TILE_BUSH;
            }
        }
    }
}

bool TileMap::isColliding(u16 tileFlags, const Circlef& circle) const
{
    return _colliding_impl(tileFlags, circle, false, nullptr);
}

bool TileMap::isContained(u16 tileFlags, const Circlef& circle) const
{
    //@WIP
}

bool TileMap::getCollidingTile(u16 tileFlags, const Circlef& circle, sf::FloatRect& tileRect) const
{
    return _colliding_impl(tileFlags, circle, true, &tileRect);
}

TileType TileMap::getTile(u16 i, u16 j) const
{
    return static_cast<TileType>(m_tiles[i][j]);
}

Vector2u TileMap::getSize() const
{
    return m_size;
}

Vector2u TileMap::getWorldSize() const
{
    return {m_size.x * m_tileSize * m_tileScale, m_size.y * m_tileSize * m_tileScale};
}

u16 TileMap::getTileSize() const
{
    return m_tileSize;
}

u16 TileMap::getTileScale() const
{
    return m_tileScale;
}

bool TileMap::_colliding_impl(u16 tileFlags, const Circlef& circle, bool getTile, sf::FloatRect* tileRect) const
{
    size_t tileSize = m_tileSize * m_tileScale;

    Vector2i min = {(int) (circle.center.x - circle.radius)/tileSize, (int) (circle.center.y - circle.radius)/tileSize};
    Vector2i max = {(int) (circle.center.x + circle.radius)/tileSize, (int) (circle.center.y + circle.radius)/tileSize};

    //return false if the circle is outside the map
    if (min.x < 0 || min.x >= getSize().x) return false;
    if (min.y < 0 || min.y >= getSize().y) return false;
    if (max.x < 0 || max.x >= getSize().x) return false;
    if (max.y < 0 || max.y >= getSize().y) return false;

    sf::FloatRect rect;

    rect.height = tileSize;
    rect.width = tileSize;

    for (size_t i = min.x; i <= max.x; ++i) {
        for (size_t j = min.y; j <= max.y; ++j) {
            u16 tile = m_tiles[i][j];

            if ((tile & tileFlags) == 0) continue;

            rect.left = i * tileSize;
            rect.top = j * tileSize;

            if (circle.intersects(rect)) {
                if (getTile) {
                    tileRect->left = rect.left;
                    tileRect->top = rect.top;
                    tileRect->width = rect.width;
                    tileRect->height = rect.height;
                }

                return true;
            }
        }
    }

    return false;
}
