#pragma once

class EntityManager;
class CollisionManager;
class TileMap;
class GameMode;

struct ManagersContext {
    EntityManager* entityManager;
    CollisionManager* collisionManager;
    TileMap* tileMap;
    GameMode* gameMode;

    ManagersContext()
    {
        entityManager = nullptr;
        collisionManager = nullptr;
        tileMap = nullptr;
        gameMode = nullptr;
    }

    ManagersContext(EntityManager* e, CollisionManager* c, TileMap* t, GameMode* g)
    {
        entityManager = e;
        collisionManager = c;
        tileMap = t;
        gameMode = g;
    }
};

class C_EntityManager;

struct C_ManagersContext {
    C_EntityManager* entityManager;
    TileMap* tileMap;
    GameMode* gameMode;

    C_ManagersContext()
    {
        entityManager = nullptr;
        tileMap = nullptr;
        gameMode = nullptr;
    }

    C_ManagersContext(C_EntityManager* e, TileMap* t, GameMode* g)
    {
        entityManager = e;
        tileMap = t;
        gameMode = g;
    }
};
