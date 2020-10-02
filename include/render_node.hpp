#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include "defines.hpp"

#ifdef MANDARINA_DEBUG
#include <string>
#endif

struct RenderNode {
    //the RenderNode can have ownership of the drawabale
    sf::Sprite sprite;

    //or it can be delegated somewhere else
    const sf::Drawable* drawable;

    //flag that's true if the node has ownership of its drawable
    bool usingSprite;

    //all these parameters are used to sort the entities
    float height;
    u32 uniqueId;
    int manualFilter;

    bool takingDamage;

#ifdef MANDARINA_DEBUG
    std::string debugDisplayData;

    //Used to properly position text and debug collision shapes
    //(since sf::Drawable doesn't store position)
    Vector2 position;

    //used to render debug collision shapes
    float collisionRadius;
#endif

    RenderNode(u32 uniqueId);

    bool operator<(const RenderNode& other);
};
