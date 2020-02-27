#include "server_entity_manager.hpp"

#include "collision_manager.hpp"
#include "tilemap.hpp"

EntityManager::EntityManager()
{
    m_lastUniqueId = 0;
}

void EntityManager::update(sf::Time eTime)
{
    ManagersContext context(this, m_collisionManager, m_tileMap);

    int i;

    for (i = 0; i < units.firstInvalidIndex(); ++i) {
        Unit_preUpdate(units[i], eTime, context);
    }

    for (i = 0; i < units.firstInvalidIndex(); ++i) {
        Unit_update(units[i], eTime, context);
    }

    for (i = 0; i < projectiles.firstInvalidIndex(); ++i) {
        Projectile_update(projectiles[i], eTime, context);
    }

    //remove dead projectiles
    i = 0;
    
    while (i < projectiles.firstInvalidIndex()) {
        if (projectiles[i].dead) {
            projectiles.removeElement(projectiles[i].uniqueId);
        } else {
            ++i;
        }
    }

    //don't remove units since they might respawn depending on the game mode
}

int EntityManager::createUnit(UnitType type, const Vector2& pos, u8 teamId)
{
    if (type < 0 || type >= UNIT_MAX_TYPES) {
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

int EntityManager::createProjectile(ProjectileType type, const Vector2& pos, float aimAngle, u8 teamId)
{
    if (type < 0 || type >= PROJECTILE_MAX_TYPES) {
        std::cout << "EntityManager::createProjectile error - Invalid type" << std::endl;
        return -1;
    }

    u32 uniqueId = _getNewUniqueId();
    int index = projectiles.addElement(uniqueId);

    Projectile& projectile = projectiles[index];

    Projectile_init(projectile, type, pos, aimAngle);

    projectile.uniqueId = uniqueId;
    projectile.teamId = teamId;

    return uniqueId;
}

void EntityManager::takeSnapshot(EntityManager* snapshot) const
{
    units.copyValidDataTo(snapshot->units);
    projectiles.copyValidDataTo(snapshot->projectiles);
}

void EntityManager::packData(const EntityManager* snapshot, u8 teamId, CRCPacket& outPacket) const
{
    u16 unitsToSend = 0;

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        if (Unit_isVisibleForTeam(units[i], teamId) || Unit_isMarkedToSendForTeam(units[i], teamId)) {
            unitsToSend++;
        }
    }

    outPacket << unitsToSend;

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        const Unit& unit = units[i];

        //don't send units that player cannot see
        if (!Unit_isVisibleForTeam(unit, teamId) && !Unit_isMarkedToSendForTeam(unit, teamId)) continue;

        const Unit* prevUnit = nullptr;

        if (snapshot) {
            prevUnit = snapshot->units.atUniqueId(unit.uniqueId);
        }

        outPacket << unit.uniqueId;

        //if the unit didn't exist or it wasn't visible or sent
        if (!prevUnit || (!Unit_isVisibleForTeam(*prevUnit, teamId) && !Unit_isMarkedToSendForTeam(*prevUnit, teamId))) {
            outPacket << unit.type;
            
            //we pack all the data again
            prevUnit = nullptr;
        }

        Unit_packData(unit, prevUnit, teamId, outPacket);
    }

    //We're assuming here all projectiles are visible (which is true?)
    outPacket << (u16) projectiles.firstInvalidIndex();

    for (int i = 0; i < projectiles.firstInvalidIndex(); ++i) {
        const Projectile& projectile = projectiles[i];
        const Projectile* prevProj = nullptr;

        if (snapshot) {
            prevProj = snapshot->projectiles.atUniqueId(projectile.uniqueId);
        }

        outPacket << projectile.uniqueId;

        if (!prevProj) {
            outPacket << projectile.type;
        }

        Projectile_packData(projectile, prevProj, teamId, outPacket);
    }
}

void EntityManager::allocateAll()
{
    units.resize(MAX_UNITS);
    projectiles.resize(MAX_PROJECTILES);
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
