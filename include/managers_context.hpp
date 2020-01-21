#pragma once

class EntityManager;
class CollisionManager;

struct ManagersContext {
    EntityManager* entityManager = nullptr;
    CollisionManager* collisionManager = nullptr;

    ManagersContext(EntityManager* e, CollisionManager* c)
    {
        entityManager = e;
        collisionManager = c;
    }
};

class C_EntityManager;

struct C_ManagersContext {
    C_EntityManager* entityManager = nullptr;

    C_ManagersContext(C_EntityManager* e)
    {
        entityManager = e;
    }
};
