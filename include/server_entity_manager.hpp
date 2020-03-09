#pragma once

#include "json_parser.hpp"
#include "bucket.hpp"
#include "defines.hpp"
// #include "units.hpp"
#include "projectiles.hpp"

#include "managers_context.hpp"

//@TODO: Use this?
#include "static_bucket.hpp"

#include "entity.hpp"
#include "entity_table.hpp"
#include "unit.hpp"

class EntityManager
{
public:
    EntityManager(const JsonParser* jsonParser);
    EntityManager();

    void update(sf::Time eTime);

    Projectile* createProjectile(u8 projectileType, const Vector2& pos, float aimAngle, u8 teamId);
    Entity* createEntity(u8 entityType, const Vector2& pos, u8 teamId);

    void takeSnapshot(EntityManager* snapshot) const;
    void packData(const EntityManager* snapshot, u8 teamId, CRCPacket& outPacket) const;

    void allocateAll();

    void setCollisionManager(CollisionManager* collisionManager);
    void setTileMap(TileMap* tileMap);

    static const Entity* getEntityData(u8 type);

public:
    // Bucket<Unit> units;
    EntityTable<Entity> entities;
    Bucket<Projectile> projectiles;

private:
    inline u32 _getNewUniqueId();

    CollisionManager* m_collisionManager;
    TileMap* m_tileMap;

    u32 m_lastUniqueId;

    static bool m_entitiesJsonLoaded;
    static std::unique_ptr<Entity> m_entityData[ENTITY_MAX_TYPES];

    static void loadEntityData(const JsonParser* jsonParser);
};
