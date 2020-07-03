#include "tilemap.hpp"

#include <iostream>
#include "paths.hpp"

TileMap::TileMap(u16 tileSize, u16 tileScale)
{
    m_tileSize = tileSize;
    m_tileScale = tileScale;
}

void TileMap::loadFromFile(const std::string& filename)
{
    sf::Image image;

    if (!image.loadFromFile(MAPS_PATH + filename + "." + MAP_FILENAME_EXT)) {
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

std::list<Vector2> TileMap::loadSpawnPoints(const std::string& filename, const sf::Color& color)
{
    sf::Image image;
    std::list<Vector2> points;

    if (!image.loadFromFile(MAPS_PATH + filename + "." + MAP_FILENAME_EXT)) {
        std::cout << "TileMap::loadFromFile error - Invalid filename" << std::endl;
        return points;
    }


    float tileSize = m_tileSize * m_tileScale;

    for (int i = 0; i < image.getSize().x; ++i) {
        for (int j = 0; j < image.getSize().y; ++j) {
            if (image.getPixel(i, j) == color) {
                points.emplace_back(i * tileSize + tileSize/2.f, j * tileSize + tileSize/2.f);
            }
        }
    }

    return points;
}

bool TileMap::isColliding(u16 tileFlags, const Circlef& circle) const
{
    return _collidingContained_impl(false, tileFlags, circle, nullptr, nullptr);
}

bool TileMap::isContained(u16 tileFlags, const Circlef& circle) const
{
    return _collidingContained_impl(true, tileFlags, circle, nullptr, nullptr);
}

bool TileMap::isOutsideMap(const Circlef& circle) const
{
    size_t tileSize = m_tileSize * m_tileScale;
    Vector2i min = {(int) (circle.center.x - circle.radius)/tileSize, (int) (circle.center.y - circle.radius)/tileSize};
    Vector2i max = {(int) (circle.center.x + circle.radius)/tileSize, (int) (circle.center.y + circle.radius)/tileSize};

    if (min.x < 0 || min.x >= getSize().x) return true;
    if (min.y < 0 || min.y >= getSize().y) return true;
    if (max.x < 0 || max.x >= getSize().x) return true;
    if (max.y < 0 || max.y >= getSize().y) return true;

    return false;
}

u16 TileMap::getCollidingTile(const Circlef& circle) const
{
    u16 tile = 0;
    _collidingContained_impl(false, -1, circle, nullptr, &tile);
    return tile;
}

bool TileMap::getCollidingTileRect(u16 tileFlags, const Circlef& circle, sf::FloatRect& tileRect) const
{
    return _collidingContained_impl(false, tileFlags, circle, &tileRect, nullptr);
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

bool TileMap::_collidingContained_impl(bool contained, u16 tileFlags, const Circlef& circle, sf::FloatRect* tileRect, u16* getTiles) const
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

            rect.left = i * tileSize;
            rect.top = j * tileSize;

            if ((tile & tileFlags) == 0) {
                //for a circle to be contained it has to intersect only
                //tiles of that type
                if (contained && circle.intersects(rect)) {
                    return false;
                } else {
                    continue;
                }
            }

            if (!contained && circle.intersects(rect)) {
                if (tileRect) {
                    tileRect->left = rect.left;
                    tileRect->top = rect.top;
                    tileRect->width = rect.width;
                    tileRect->height = rect.height;
                }

                if (getTiles) {
                    //get all tiles colliding
                    *getTiles |= tile;
                } else {
                    return true;
                }
            }
        }
    }

    return contained;
}
