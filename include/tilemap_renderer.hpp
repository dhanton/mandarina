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

    LAYER_CEILING,

    MAX_LAYERS
};

class TileMapRenderer : public InContext
{
public:
    TileMapRenderer(const Context& context, TileMap* tileMap);

    void generateLayers();
    void updateLayers();

    void renderBeforeEntities(sf::RenderTexture& window) const;
    void renderAfterEntities(sf::RenderTexture& window) const;

private:
    Vector2u _getTextureCoords(TileType tile);
    void _setTileCoords(u16 i, u16 j, LayerType layer);
    void _setTileCoordsDivided(u16 i, u16 j, LayerType layer1, LayerType layer2);
    void _setTileTextureCoords(u16 i, u16 j, LayerType layer, const Vector2u& texCoords);
    void _setTileTextureCoordsDivided(u16 i, u16 j, LayerType layer1, LayerType layer2, const Vector2u& texCoords);
    void _setTileAlpha(u16 i, u16 j, LayerType layer, float alpha);

    TileMap* m_tileMap;

    Vector2u m_size;
    u16 m_tileSize;
    u16 m_textureTileSize;

    sf::Texture* m_texture;

    sf::VertexArray m_layers[MAX_LAYERS];
};
