#pragma once

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include "tilemap.hpp"
#include "context.hpp"

enum LayerType {
    LAYER_GROUND,
    LAYER_BUSH,

    LAYER_SIDES_TOP,
    LAYER_SIDES_BOT,
    LAYER_SIDES_LEFT,
    LAYER_SIDES_RIGHT,

    MAX_LAYERS
};

#define TILE_SCALE 4

class TileMapRenderer : public InContext
{
public:
    TileMapRenderer(const Context& context, TileMap* tileMap);

    void generateLayers();
    void updateLayers();

    void renderBeforeEntities(sf::RenderTexture& window);
    void renderAfterEntities(sf::RenderTexture& window);

    //size in pixels
    Vector2u getTotalSize() const;

private:
    Vector2u _getTextureCoords(TileType tile);
    void _setTileTextureCoords(size_t i, size_t j, LayerType layer, const Vector2u& textCoords);
    void _setTileAlpha(size_t i, size_t j, LayerType layer, float alpha);

    TileMap* m_tileMap;

    Vector2u m_size;
    u16 m_tileSize;
    u16 m_textureTileSize;

    sf::Texture* m_texture;

    sf::VertexArray m_layers[MAX_LAYERS];
};