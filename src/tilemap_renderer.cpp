#include "tilemap_renderer.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <iostream>
#include "texture_ids.hpp"

TileMapRenderer::TileMapRenderer(const Context& context, TileMap* tileMap):
    InContext(context)
{
    m_tileMap = tileMap;

    m_texture = &context.textures->getResource(TextureId::TEST_TILESET);
}

void TileMapRenderer::generateLayers()
{
    //clear current layers and generate them again based on tile map
    if (!m_tileMap) {
        std::cout << "TileMapRenderer::generateLayers error - Missing pointer to TileMap" << std::endl;
        return;
    }

    m_size = m_tileMap->getSize();
    m_textureTileSize = m_tileMap->getTileSize();
    m_tileSize = m_tileMap->getTileSize() * TILE_SCALE;

    //allocate appropiate space for each layer
    for (int i = 0; i < MAX_LAYERS; ++i) {
        sf::VertexArray& vertices = m_layers[i];

        vertices.clear();
        vertices.setPrimitiveType(sf::Quads);
        vertices.resize(m_size.x * m_size.y * 4);

        //setup the positions
        for (int i = 0; i < m_size.x; ++i) {
            for (int j = 0; j < m_size.y; ++j) {
                sf::Vertex* quad = &vertices[(i + j * m_size.x) * 4];

                quad[0].position = sf::Vector2f(i * m_tileSize, j * m_tileSize);
                quad[1].position = sf::Vector2f((i + 1) * m_tileSize, j * m_tileSize);
                quad[2].position = sf::Vector2f((i + 1) * m_tileSize, (j + 1) * m_tileSize);
                quad[3].position = sf::Vector2f(i * m_tileSize, (j + 1) * m_tileSize);
            }
        }
    }

    for (int i = 0; i < m_size.x; ++i) {
        for (int j = 0; j < m_size.y; ++j) {
            const TileType tile = m_tileMap->getTile(i, j);

            if (tile == TILE_BUSH) {
                //darker ground below bushes
                _setTileTextureCoords(i, j, LAYER_GROUND, Vector2u(3, 4));

            } else {
                //basic ground texture
                _setTileTextureCoords(i, j, LAYER_GROUND, _getTextureCoords(TILE_NONE));
            }
        }
    }

    updateLayers();
}

void TileMapRenderer::updateLayers()
{
    if (!m_tileMap) {
        std::cout << "TileMapRenderer::updateLayers error - Missing pointer to TileMap" << std::endl;
        return;
    }

    //@WIP: Make it so you can update the layers for specific tiles (the ones that changed)
    //or force it to update all (used when the layers are generated for the first time)

    for (int i = 0; i < m_size.x; ++i) {
        for (int j = 0; j < m_size.y; ++j) {
            const TileType tile = m_tileMap->getTile(i, j);

            if (tile == TILE_BUSH) {
                _setTileTextureCoords(i, j, LAYER_BUSH, _getTextureCoords(TILE_BUSH));

                Vector2u sidesTextCoord;

                if (j < m_size.y && m_tileMap->getTile(i, j + 1) != TILE_BUSH) {
                    _setTileTextureCoords(i, j + 1, LAYER_SIDES_BOT, rand() % 2 == 1 ? Vector2u(2, 0) : Vector2u(3, 0));
                }

                if (j > 0 && m_tileMap->getTile(i, j - 1) != TILE_BUSH) {
                    _setTileTextureCoords(i, j - 1, LAYER_SIDES_TOP, rand() % 2 == 1 ? Vector2u(0, 0) : Vector2u(1, 0));
                }

                if (i < m_size.x && m_tileMap->getTile(i + 1, j) != TILE_BUSH) {
                    _setTileTextureCoords(i + 1, j, LAYER_SIDES_LEFT, rand() % 2 == 1 ? Vector2u(4, 0) : Vector2u(5, 0));
                }

                if (i > 0 && m_tileMap->getTile(i - 1, j) != TILE_BUSH) {
                    _setTileTextureCoords(i - 1, j, LAYER_SIDES_RIGHT, rand() % 2 == 1 ? Vector2u(0, 1) : Vector2u(1, 1));
                }
            }
        }
    }
}

void TileMapRenderer::renderBeforeEntities(sf::RenderTexture& renderTexture)
{
    renderTexture.draw(m_layers[LAYER_GROUND], m_texture);
    renderTexture.draw(m_layers[LAYER_BUSH], m_texture);
    renderTexture.draw(m_layers[LAYER_SIDES_LEFT], m_texture);
    renderTexture.draw(m_layers[LAYER_SIDES_RIGHT], m_texture);
    renderTexture.draw(m_layers[LAYER_SIDES_BOT], m_texture);
    renderTexture.draw(m_layers[LAYER_SIDES_TOP], m_texture);
}

void TileMapRenderer::renderAfterEntities(sf::RenderTexture& renderTexture)
{

}

Vector2u TileMapRenderer::getTotalSize() const
{
    return {m_size.x * m_tileSize, m_size.y * m_tileSize};
}

Vector2u TileMapRenderer::_getTextureCoords(TileType tile)
{
    switch (tile) 
    {
        case TILE_NONE:
        {
            return (rand() % 2) == 0 ? Vector2u(2, 3) : Vector2u(2, 4);
        }

        case TILE_BUSH:
        {
            return Vector2u(0, 4);
        }
    }

    return Vector2u();
}

void TileMapRenderer::_setTileTextureCoords(size_t i, size_t j, LayerType layer, const Vector2u& textCoords)
{
    sf::Vertex* quad = &m_layers[layer][(i + j * m_size.x) * 4];

    quad[0].texCoords = sf::Vector2f(textCoords.x * m_textureTileSize, textCoords.y * m_textureTileSize);
    quad[1].texCoords = sf::Vector2f((textCoords.x + 1) * m_textureTileSize, textCoords.y * m_textureTileSize);
    quad[2].texCoords = sf::Vector2f((textCoords.x + 1) * m_textureTileSize, (textCoords.y + 1) * m_textureTileSize);
    quad[3].texCoords = sf::Vector2f(textCoords.x * m_textureTileSize, (textCoords.y + 1) * m_textureTileSize);
}

void TileMapRenderer::_setTileAlpha(size_t i, size_t j, LayerType layer, float alpha)
{
    sf::Vertex* quad = &m_layers[layer][(i + j * m_size.x) * 4];

    quad[0].color = sf::Color(quad[0].color.r, quad[0].color.g, quad[0].color.b, alpha);
    quad[1].color = sf::Color(quad[1].color.r, quad[1].color.g, quad[1].color.b, alpha);
    quad[2].color = sf::Color(quad[2].color.r, quad[2].color.g, quad[2].color.b, alpha);
    quad[3].color = sf::Color(quad[3].color.r, quad[3].color.g, quad[3].color.b, alpha);
}