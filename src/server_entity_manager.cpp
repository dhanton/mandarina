#include "server_entity_manager.hpp"

EntityManager::EntityManager()
{
    m_lastUniqueId = 0;
}

void EntityManager::update(sf::Time eTime)
{
    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        Unit_update(units[i], eTime);
    }
}

int EntityManager::createUnit(UnitType type, const Vector2& pos, u8 teamId)
{
    if (type <= UNIT_NONE || type >= UNIT_MAX_TYPES) {
        std::cout << "EntityManager::createUnit error - Invalid type" << std::endl;
        return -1;
    }

    u32 uniqueId = _getNewUniqueId();
    int index = units.addElement(uniqueId);

    Unit& unit = units[index];

    //init has to go first since we're copying values
    Unit_init(unit, type);

    unit.pos = pos;
    unit.teamId = teamId;
    unit.uniqueId = uniqueId;

    return index;
}

void EntityManager::takeSnapshot(EntityManager* snapshot) const
{
    units.copyValidDataTo(snapshot->units);
}

void EntityManager::packData(const EntityManager* snapshot, CRCPacket& outPacket) const
{
    outPacket << (u16) units.firstInvalidIndex();

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        const Unit& unit = units[i];
        const Unit* prevUnit = nullptr;

        if (snapshot) {
            prevUnit = snapshot->units.atUniqueId(unit.uniqueId);
        }

        outPacket << unit.uniqueId;

        if (!prevUnit) {
            outPacket << unit.type;
        }

        Unit_packData(unit, prevUnit, outPacket);
    }
}

void EntityManager::allocateAll()
{
    units.resize(MAX_UNITS);
}

inline u32 EntityManager::_getNewUniqueId()
{
    return ++m_lastUniqueId;
}
