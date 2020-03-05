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

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        it->update(eTime, context);
    }

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        it->preUpdate(eTime, context);
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

    //@BRANCH_WIP: See how we can remove entities properly in a general way
    //Taking into account that some entities might respawn depending on game mode
}

//@BRANCH_WIP: See how we can create specific units using their type
//its important to have a type thats a u8 so we can send it between client/server
//and create entities in different places
Entity* EntityManager::createEntity(const Vector2& pos, u8 teamId)
{
    //@BRANCH_WIP: See the specified type is correct
    // if (type < 0 || type >= UNIT_MAX_TYPES) {
        // std::cout << "EntityManager::createUnit error - Invalid type" << std::endl;
        // return -1;
    // }

    u32 uniqueId = _getNewUniqueId();

    //@BRANCH_WIP: Use the specified type here
    Entity* entity = new Unit(uniqueId);
    entity->setTeamId(teamId);
    entity->setPosition(pos);

    entities.addEntity(entity);

    if (entity->inQuadtree()) {
        m_collisionManager->onInsertUnit(uniqueId, pos, entity->getCollisionRadius());
        entity->onQuadtreeInserted(ManagersContext(this, m_collisionManager, m_tileMap));
    }

    return entity;
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
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        snapshot->entities.addEntity(it->clone());
    }

    projectiles.copyValidDataTo(snapshot->projectiles);
}

void EntityManager::packData(const EntityManager* snapshot, u8 teamId, CRCPacket& outPacket) const
{
    u16 unitsToSend = 0;

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (it->shouldSendToTeam(teamId)) {
            unitsToSend++;
        }
    }

    outPacket << unitsToSend;

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (!it->shouldSendToTeam(teamId)) continue;

        const Entity* prevEntity = nullptr;

        if (snapshot) {
            prevEntity = snapshot->entities.atUniqueId(it->getUniqueId());
        }

        outPacket << it->getUniqueId();

        if (!prevEntity || !(prevEntity->shouldSendToTeam(teamId))) {
            //@BRANCH_WIP: Send the correct type
            outPacket << (u8) 0;

            //pack all data again
            prevEntity = nullptr;
        }

        it->packData(prevEntity, teamId, outPacket);
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

        Projectile_packData(projectile, prevProj, teamId, outPacket, this);
    }
}

void EntityManager::allocateAll()
{
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
