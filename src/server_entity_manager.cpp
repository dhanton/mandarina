#include "server_entity_manager.hpp"

EntityManager::EntityManager()
{
    m_lastUniqueId = 0;
}

void EntityManager::update(sf::Time eTime)
{
    for (int i = 0; i < m_characters.firstInvalidIndex(); ++i) {
        TestCharacter_update(m_characters[i], eTime);
    }
}

int EntityManager::createEntity(EntityType type, const Vector2& pos)
{
    switch (type)
    {
        case EntityType::TEST_CHARACTER:
        {
            u32 uniqueId = _getNewUniqueId();
            int index = m_characters.addElement(uniqueId);

            TestCharacter& entity = m_characters[index];
            entity.uniqueId = uniqueId;
            entity.pos = pos;

            TestCharacter_init(entity);

            return index;
        }
    }

    return -1;
}

void EntityManager::takeSnapshot(EntityManager* snapshot) const
{
    m_characters.copyValidDataTo(snapshot->m_characters);
}

void EntityManager::packData(const EntityManager* snapshot, CRCPacket& outPacket) const
{
    outPacket << (u16) m_characters.firstInvalidIndex();

    for (int i = 0; i < m_characters.firstInvalidIndex(); ++i) {
        const TestCharacter& character = m_characters[i];
        const TestCharacter* prevEntity = nullptr;

        if (snapshot) {
            prevEntity = snapshot->m_characters.atUniqueId(character.uniqueId);
        }

        outPacket << character.uniqueId;
        TestCharacter_packData(character, prevEntity, outPacket);
    }
}

void EntityManager::allocateAll()
{
    m_characters.resize(100);
}

inline u32 EntityManager::_getNewUniqueId()
{
    return ++m_lastUniqueId;
}
