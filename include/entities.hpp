#pragma once

#include "components.hpp"

class EntityManager;

struct TestCharacter
{
    TestCharacter();

    static ComponentFlags defaultComponentFlags;
    static void asignDefaultValues(EntityManager* manager, int entityId);

    //component with callback example
    // static void onDeath();
};
