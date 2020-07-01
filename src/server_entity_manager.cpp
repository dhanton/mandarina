#include "server_entity_manager.hpp"

#include "collision_manager.hpp"
#include "tilemap.hpp"

EntityManager::EntityManager(const JsonParser* jsonParser)
{
    loadEntityData(jsonParser);

    m_lastUniqueId = 0;

    m_managers.entityManager = this;
}

//constructor used if the instance is a snapshot
EntityManager::EntityManager()
{

}

void EntityManager::update(sf::Time eTime)
{
    int i;

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        it->preUpdate(eTime, m_managers);
    }

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        it->update(eTime, m_managers);
    }

    //Is postupdate really needed?
    for (auto it = entities.begin(); it != entities.end();) {
        it->postUpdate(eTime, m_managers);

        //remove dead entities
        if (it->isDead()) {
            it = entities.removeEntity(it);
        } else {
            ++it;
        }
    }

    for (i = 0; i < projectiles.firstInvalidIndex(); ++i) {
        Projectile_update(projectiles[i], eTime, m_managers);
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

    //@WIP: See how we can remove entities properly in a general way (without abusing delete)
    //Taking into account that some entities might respawn depending on game mode
    //for entities that respawn (heroes) it's probably best to just move them to some other list
    //so the pointer is still kept alive
}

Projectile* EntityManager::createProjectile(u8 type, const Vector2& pos, float aimAngle, u8 teamId)
{
    if (type < 0 || type >= PROJECTILE_MAX_TYPES) {
        std::cout << "EntityManager::createProjectile error - Invalid type" << std::endl;
        return nullptr;
    }

    u32 uniqueId = _getNewUniqueId();
    int index = projectiles.addElement(uniqueId);

    Projectile& projectile = projectiles[index];

    Projectile_init(projectile, type, pos, aimAngle);

    projectile.uniqueId = uniqueId;
    projectile.teamId = teamId;

    return &projectile;
}

Entity* EntityManager::createEntity(u8 entityType, const Vector2& pos, u8 teamId, u32 forcedUniqueId)
{
    if (entityType < 0 || entityType >= ENTITY_MAX_TYPES) {
        std::cout << "EntityManager::createEntity error - Invalid entity type" << std::endl;
        return nullptr;
    }
    
    Entity* entity = nullptr;
    u32 uniqueId;

    //sometimes we might want to force the new entity to have a specific uniqueId
    if (forcedUniqueId != 0) {
        if (entities.atUniqueId(forcedUniqueId)) {
            std::cout << "EntityManager::createEntity error - Forced uniqueId already exists" << std::endl;
            return nullptr;
        }

        uniqueId = forcedUniqueId;

    } else {
        uniqueId = _getNewUniqueId();
    }

    entity = m_entityData[entityType]->clone();
    entity->setUniqueId(uniqueId);

    entity->setTeamId(teamId);
    entity->setPosition(pos);

    entities.addEntity(entity);

    //if the entity is initially solid, add it to the quadtree
    //@WIP: Maybe some quadtree entities start not being solid?
    if (entity->isSolid()) {
        m_managers.collisionManager->onInsertEntity(uniqueId, pos, entity->getCollisionRadius());
        entity->onQuadtreeInserted(m_managers);
    }

    return entity;
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
            outPacket << it->getEntityType();

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

void EntityManager::setManagersContext(const ManagersContext& managers)
{
    m_managers.collisionManager = managers.collisionManager;
    m_managers.tileMap = managers.tileMap;
    m_managers.gameMode = managers.gameMode;
}

bool EntityManager::m_entitiesJsonLoaded = false;
std::unique_ptr<Entity> EntityManager::m_entityData[ENTITY_MAX_TYPES];

const Entity* EntityManager::getEntityData(u8 type)
{
    if (type >= ENTITY_MAX_TYPES) return nullptr;

    return m_entityData[type].get();
}

void EntityManager::loadEntityData(const JsonParser* jsonParser)
{
    if (m_entitiesJsonLoaded) return;

    #define DoEntity(class_name, type, json_id) \
        m_entityData[ENTITY_##type] = std::unique_ptr<Entity>(new class_name()); \
        m_entityData[ENTITY_##type]->setEntityType(ENTITY_##type); \
        m_entityData[ENTITY_##type]->loadFromJson(*jsonParser->getDocument(json_id));
    #include "entities.inc"
    #undef DoEntity

    m_entitiesJsonLoaded = true;
}

inline u32 EntityManager::_getNewUniqueId()
{
    return ++m_lastUniqueId;
}
