#pragma once

class EntityManager;
class CollisionManager;
class TileMap;

struct ManagersContext {
    EntityManager* entityManager = nullptr;
    CollisionManager* collisionManager = nullptr;
    TileMap* tileMap = nullptr;

    ManagersContext(EntityManager* e, CollisionManager* c, TileMap* t)
    {
        entityManager = e;
        collisionManager = c;
        tileMap = t;
    }
};

class C_EntityManager;

struct C_ManagersContext {
    C_EntityManager* entityManager = nullptr;
    TileMap* tileMap = nullptr;

    C_ManagersContext(C_EntityManager* e, TileMap* t)
    {
        entityManager = e;
        tileMap = t;
    }
};
