#pragma once

#include "bucket.hpp"
#include "defines.hpp"
#include "entities.hpp"

//@WIP: Use this?
#include "static_bucket.hpp"

class EntityManager
{
public:
    EntityManager();

    void update(sf::Time eTime);

    //@WIP: Make different functions for diffent entity types (unit, projectile, hero) (??)
    //or is it better to pass an enum (??) or two enums (??)
    int createEntity(EntityType type, const Vector2& pos);

    void takeSnapshot(EntityManager* snapshot) const;
    void packData(const EntityManager* snapshot, CRCPacket& outPacket) const;

    void allocateAll();
    
public:
    Bucket<TestCharacter> m_characters;

private:
    inline u32 _getNewUniqueId();

    u32 m_lastUniqueId;
};
