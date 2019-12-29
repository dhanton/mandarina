#pragma once

#include "bucket.hpp"
#include "defines.hpp"
#include "entities.hpp"

class EntityManager
{
public:
    EntityManager();

    void update(sf::Time eTime);

    int createEntity(EntityType type, const Vector2& pos);

    void takeSnapshot(EntityManager* snapshot) const;
    void packData(const EntityManager* snapshot, CRCPacket& outPacket) const;

    void allocate();

private:
    inline u32 _getNewUniqueId();

    u32 m_lastUniqueId;

public:
    Bucket<TestCharacter> m_characters;
};
