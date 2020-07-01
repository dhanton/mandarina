#pragma once

#include "json_parser.hpp"
#include "bucket.hpp"
#include "defines.hpp"
#include "projectiles.hpp"

#include "managers_context.hpp"

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
    Entity* createEntity(u8 entityType, const Vector2& pos, u8 teamId, u32 forcedUniqueId = 0);

    void takeSnapshot(EntityManager* snapshot) const;
    void packData(const EntityManager* snapshot, u8 teamId, CRCPacket& outPacket) const;

    void allocateAll();

    void setManagersContext(const ManagersContext& managers);

    static const Entity* getEntityData(u8 type);
    static void loadEntityData(const JsonParser* jsonParser);

public:
    // Bucket<Unit> units;
    EntityTable<Entity> entities;
    Bucket<Projectile> projectiles;

private:
    inline u32 _getNewUniqueId();

    ManagersContext m_managers;

    u32 m_lastUniqueId;

    static bool m_entitiesJsonLoaded;
    static std::unique_ptr<Entity> m_entityData[ENTITY_MAX_TYPES];
};
