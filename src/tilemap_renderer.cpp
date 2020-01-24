#include "tilemap_renderer.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <iostream>
#include "texture_ids.hpp"

TileMapRenderer::TileMapRenderer(const Context& context, TileMap* tileMap):
    InContext(context)
{
    m_tileMap = tileMap;

    //@TODO: Load map settings from json file
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
    m_tileSize = m_tileMap->getTileSize() * m_tileMap->getTileScale();

    //allocate appropiate space for each layer
    for (int layer = 0; layer < MAX_LAYERS; ++layer) {
        sf::VertexArray& vertices = m_layers[layer];

        vertices.clear();
        vertices.setPrimitiveType(sf::Quads);
        vertices.resize(m_size.x * m_size.y * 4);

        //setup tile positions
        for (int i = 0; i < m_size.x; ++i) {
            for (int j = 0; j < m_size.y; ++j) {
                _setTileCoords(i, j, (LayerType) layer);
            }
        }
    }

    //setup textures
    for (int i = 0; i < m_size.x; ++i) {
        for (int j = 0; j < m_size.y; ++j) {
            const TileType tile = m_tileMap->getTile(i, j);

            if (tile == TILE_BUSH || tile == TILE_BLOCK) {
                //darker ground below bushes and blocks
                _setTileTextureCoords(i, j, LAYER_GROUND, Vector2u(3, 4));

            } else if (tile == TILE_WALL) {
                //indestructible walls
                if (j == m_size.y - 1) {
                    _setTileTextureCoords(i, j, LAYER_CEILING, _getTextureCoords(TILE_WALL));
                } else {
                    _setTileTextureCoords(i, j, LAYER_GROUND, _getTextureCoords(TILE_WALL));
                }

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
    //vector<Vector2u> changedTiles and m_tileMap->getTile(...) and bool updateAllTiles

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

            if (tile == TILE_BLOCK) {
                _setTileCoordsDivided(i, j, LAYER_CEILING, LAYER_BUSH);
                _setTileTextureCoordsDivided(i, j, LAYER_CEILING, LAYER_BUSH, _getTextureCoords(TILE_BLOCK));
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
    renderTexture.draw(m_layers[LAYER_CEILING], m_texture);
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

        case TILE_WALL:
        case TILE_BLOCK:
        {
            return (rand() % 2) == 0 ? Vector2u(0, 2) : Vector2u(2, 2);
        }
    }

    return Vector2u();
}

void TileMapRenderer::_setTileCoords(u16 i, u16 j, LayerType layer)
{
    sf::Vertex* quad = &m_layers[layer][(i + j * m_size.x) * 4];

    quad[0].position = Vector2(i * m_tileSize, j * m_tileSize);
    quad[1].position = Vector2((i + 1) * m_tileSize, j * m_tileSize);
    quad[2].position = Vector2((i + 1) * m_tileSize, (j + 1) * m_tileSize);
    quad[3].position = Vector2(i * m_tileSize, (j + 1) * m_tileSize);
}

void TileMapRenderer::_setTileCoordsDivided(u16 i, u16 j, LayerType layer1, LayerType layer2)
{
    sf::Vertex* quad1 = &m_layers[layer1][(i + j * m_size.x) * 4];
    sf::Vertex* quad2 = &m_layers[layer2][(i + j * m_size.x) * 4];

    //divide the tile vertically into two layers

    quad1[0].position = Vector2(i * m_tileSize,              j * m_tileSize);
    quad1[1].position = Vector2(i * m_tileSize + m_tileSize, j * m_tileSize);
    quad1[2].position = Vector2(i * m_tileSize + m_tileSize, j * m_tileSize + m_tileSize/2);
    quad1[3].position = Vector2(i * m_tileSize,              j * m_tileSize + m_tileSize/2);

    quad2[0].position = Vector2(i * m_tileSize,              j * m_tileSize + m_tileSize/2);
    quad2[1].position = Vector2(i * m_tileSize + m_tileSize, j * m_tileSize + m_tileSize/2);
    quad2[2].position = Vector2(i * m_tileSize + m_tileSize, j * m_tileSize + m_tileSize);
    quad2[3].position = Vector2(i * m_tileSize,              j * m_tileSize + m_tileSize);
}

void TileMapRenderer::_setTileTextureCoords(u16 i, u16 j, LayerType layer, const Vector2u& texCoords)
{
    sf::Vertex* quad = &m_layers[layer][(i + j * m_size.x) * 4];

    quad[0].texCoords = Vector2(texCoords.x * m_textureTileSize, texCoords.y * m_textureTileSize);
    quad[1].texCoords = Vector2((texCoords.x + 1) * m_textureTileSize, texCoords.y * m_textureTileSize);
    quad[2].texCoords = Vector2((texCoords.x + 1) * m_textureTileSize, (texCoords.y + 1) * m_textureTileSize);
    quad[3].texCoords = Vector2(texCoords.x * m_textureTileSize, (texCoords.y + 1) * m_textureTileSize);
}

void TileMapRenderer::_setTileTextureCoordsDivided(u16 i, u16 j, LayerType layer1, LayerType layer2, const Vector2u& texCoords)
{
    sf::Vertex* quad1 = &m_layers[layer1][(i + j * m_size.x) * 4];
    sf::Vertex* quad2 = &m_layers[layer2][(i + j * m_size.x) * 4];

    quad1[0].texCoords = Vector2(texCoords.x * m_textureTileSize,                     texCoords.y * m_textureTileSize);
    quad1[1].texCoords = Vector2(texCoords.x * m_textureTileSize + m_textureTileSize, texCoords.y * m_textureTileSize);
    quad1[2].texCoords = Vector2(texCoords.x * m_textureTileSize + m_textureTileSize, texCoords.y * m_textureTileSize + m_textureTileSize/2);
    quad1[3].texCoords = Vector2(texCoords.x * m_textureTileSize,                     texCoords.y * m_textureTileSize + m_textureTileSize/2);

    quad2[0].texCoords = Vector2(texCoords.x * m_textureTileSize,                     texCoords.y * m_textureTileSize + m_textureTileSize/2);
    quad2[1].texCoords = Vector2(texCoords.x * m_textureTileSize + m_textureTileSize, texCoords.y * m_textureTileSize + m_textureTileSize/2);
    quad2[2].texCoords = Vector2(texCoords.x * m_textureTileSize + m_textureTileSize, texCoords.y * m_textureTileSize + m_textureTileSize);
    quad2[3].texCoords = Vector2(texCoords.x * m_textureTileSize,                     texCoords.y * m_textureTileSize + m_textureTileSize);
}

void TileMapRenderer::_setTileAlpha(u16 i, u16 j, LayerType layer, float alpha)
{
    sf::Vertex* quad = &m_layers[layer][(i + j * m_size.x) * 4];

    quad[0].color = sf::Color(quad[0].color.r, quad[0].color.g, quad[0].color.b, alpha);
    quad[1].color = sf::Color(quad[1].color.r, quad[1].color.g, quad[1].color.b, alpha);
    quad[2].color = sf::Color(quad[2].color.r, quad[2].color.g, quad[2].color.b, alpha);
    quad[3].color = sf::Color(quad[3].color.r, quad[3].color.g, quad[3].color.b, alpha);
}
