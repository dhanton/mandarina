#include "server_entity_manager.hpp"

#include "collision_manager.hpp"
#include "tilemap.hpp"

EntityManager::EntityManager()
{
    m_lastUniqueId = 0;
}

void EntityManager::update(sf::Time eTime)
{
    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        Unit_preUpdate(units[i], eTime, ManagersContext(this, m_collisionManager, m_tileMap));
    }

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        Unit_update(units[i], eTime, ManagersContext(this, m_collisionManager, m_tileMap));
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

    //init has to go first since it copies values
    Unit_init(unit, type);

    unit.teamId = teamId;
    unit.uniqueId = uniqueId;
    unit.pos = pos;

    m_collisionManager->onInsertUnit(uniqueId, pos, unit.collisionRadius);

    //we move the unit, forcing it to update its position while checking for collisions
    Unit_moveColliding(unit, pos, ManagersContext(this, m_collisionManager, m_tileMap), true);

    return uniqueId;
}

void EntityManager::takeSnapshot(EntityManager* snapshot) const
{
    units.copyValidDataTo(snapshot->units);
}

void EntityManager::packData(const EntityManager* snapshot, u8 teamId, CRCPacket& outPacket) const
{
    u16 unitsVisible = 0;

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        if (Unit_isVisibleForTeam(units[i], teamId)) {
            unitsVisible++;
        }
    }

    outPacket << unitsVisible;

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        const Unit& unit = units[i];

        //don't send units that player cannot see
        if (!Unit_isVisibleForTeam(unit, teamId)) continue;

        const Unit* prevUnit = nullptr;

        if (snapshot) {
            prevUnit = snapshot->units.atUniqueId(unit.uniqueId);
        }

        outPacket << unit.uniqueId;

        //if the unit didn't exist or it wasn't visible
        if (!prevUnit || !Unit_isVisibleForTeam(*prevUnit, teamId)) {
            outPacket << unit.type;
            
            //we pack all the data again
            prevUnit = nullptr;
        }

        Unit_packData(unit, prevUnit, teamId, outPacket);
    }
}

void EntityManager::allocateAll()
{
    units.resize(MAX_UNITS);
}

void EntityManager::setCollisionManager(CollisionManager* collisionManager)
{
    m_collisionManager = collisionManager;
}

void EntityManager::setTileMap(TileMap* tileMap)
{
    m_tileMap = tileMap;
}

inline u32 EntityManager::_getNewUniqueId()
{
    return ++m_lastUniqueId;
}
