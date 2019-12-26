#pragma once

#include <functional>
#include <vector>
#include "components.hpp"

struct Entity {
    int entityId = -1;
    int typeId = -1;

    //Main components
    int collisionIndex = -1;
    int renderIndex = -1;
};

class EntityManager;

struct TestCharacter
{
    static void setupComponentFlags(ComponentFlags& flags);
    static void assignDefaultValues(EntityManager* manager, int entityId);
    
    static int typeId;

    //component with callback example
    // static void onDeath();
};