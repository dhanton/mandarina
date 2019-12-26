#pragma once

#include <SFML/Graphics/Rect.hpp>

#include "defines.hpp"

//Used by entities to tell EntityManager
//what components to use
struct ComponentFlags
{
    bool collision = false;
    bool render = false;
};

struct BaseComponent
{
    Vector2 pos;
    Vector2 vel;
};

struct RenderComponent
{
    std::string textureId;
    sf::IntRect textureRect;
    float rotation;
    float scale;
};

struct CollisionComponent 
{
    int width;
    int height;
    //TODO: Make this a CollisionRect
};
