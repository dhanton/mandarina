#pragma once

//Used by entities to tell EntityManager
//what components to use
struct ComponentFlags
{
    bool collision = false;
    bool render = false;
};

struct RenderComponent
{
    std::string textureId;
    int width;
    int height;
    float rotation;
};

struct CollisionComponent 
{
    int width;
    int height;
};
