#pragma once

#include "bucket.hpp"
#include "defines.hpp"
#include "entities.hpp"

#include "managers_context.hpp"

//@WIP: Use this?
#include "static_bucket.hpp"

class EntityManager
{
public:
    EntityManager();

    void update(sf::Time eTime);

    int createUnit(UnitType type, const Vector2& pos, u8 teamId);

    void takeSnapshot(EntityManager* snapshot) const;
    void packData(const EntityManager* snapshot, CRCPacket& outPacket) const;

    void allocateAll();

    void setCollisionManager(CollisionManager* collisionManager);

public:
    Bucket<Unit> units;

private:
    inline u32 _getNewUniqueId();

    CollisionManager* m_collisionManager;

    u32 m_lastUniqueId;
};
