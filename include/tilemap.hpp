#pragma once

//WORK IN PROGRESS

enum TileType {
    NONE,
    BLOCK,
    DESTRUCTABLE_BLOCK,
    BUSH,

    TILE_MAX_TYPES
};

enum LayerType {
    GROUND,
    WALL,
    SIDES,
    CEILING,

    LAYER_NUM
};

class TileMap 
{
public:
    TileMap();

private:

    //array of layer_num
    //each element contains and array of u16 indicating position in texture

    //also array of tiletype

    //loadMapFromFile => loads from png file
    //generateLayers  => generates layers from that png file
    //renderLayer(layerType) => renders specific layer (called from client)
    //at(i, j) => returns TileType in that position
    //getSize() => returns total size of map
    //getTileSize() => returns size of each tile
};