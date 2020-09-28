#include "client_entity_manager.hpp"

#include <algorithm>
#include <SFML/Graphics.hpp>

#include "helper.hpp"
#include "tilemap.hpp"
#include "texture_ids.hpp"
#include "hero.hpp"

//all entities must be included
#include "entities/food.hpp"
#include "entities/crate.hpp"

RenderNode::RenderNode(u32 uniqueId)
{
    this->uniqueId = uniqueId;

    usingSprite = false;
    drawable = nullptr;

    height = 0.f;
    manualFilter = 0;
}

inline bool RenderNode::operator<(const RenderNode& other) 
{
    if (height == other.height) {
        if (uniqueId == other.uniqueId) {
            return manualFilter < other.manualFilter;
        }

        return uniqueId < other.uniqueId;
    }

    return height < other.height;
}

C_EntityManager::C_EntityManager(const Context& context, sf::Time worldTime):
    InContext(context)
{
    loadEntityData(context);

    localLastUniqueId = 0;
    m_controlledEntityUniqueId = 0;
    m_controlledEntityTeamId = 0;
    m_spectatingEntityUniqueId = 0;
    m_spectatingEntityTeamId = 0;
    m_heroDead = false;

#ifdef MANDARINA_DEBUG
    renderingDebug = false;
    renderingLocallyHidden = false;
    renderingEntityData = false;
#endif

    renderingEntitiesUI = false;
}

C_EntityManager::C_EntityManager():
    //dummy context used if this instance is a snapshot
    InContext(Context())
{

}

void C_EntityManager::update(sf::Time eTime)
{
    C_ManagersContext context(this, m_tileMap, nullptr);
    int i = 0;

    while (i < localProjectiles.firstInvalidIndex()) {
        // C_Projectile_localUpdate(localProjectiles[i], eTime, context);
        C_Projectile_checkCollisions(localProjectiles[i], context);

        if (localProjectiles[i].dead) {
            localProjectiles.removeElement(localProjectiles[i].uniqueId);
        } else {
            i++;
        }
    }
}

void C_EntityManager::renderUpdate(sf::Time eTime)
{
    C_ManagersContext managersContext(this, m_tileMap, nullptr);

    //we update local projectiles as much as possible (renderUpdate)
    //but we check their collision only in the normal update
    for (int i = 0; i < localProjectiles.firstInvalidIndex(); ++i) {
        C_Projectile_localUpdate(localProjectiles[i], eTime, managersContext);
    }

    //Add the appropiate render nodes
    m_renderNodes.clear();
    m_uiRenderNodes.clear();

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        it->insertRenderNode(managersContext, m_context);
    }

    for (int i = 0; i < projectiles.firstInvalidIndex(); ++i) {
        C_Projectile_insertRenderNode(projectiles[i], managersContext, m_context);
    }

    for (int i = 0; i < localProjectiles.firstInvalidIndex(); ++i) {
        C_Projectile_insertRenderNode(localProjectiles[i], managersContext, m_context);
    }

    //We have to sort all objects together by their height (accounting for flying objects)
    //(this has to be done every frame, otherwise the result might not look super good)
    //**vector sorting is faster because of the O(1) access operation**

    //sort them by height so they're displayed properly
    std::sort(m_renderNodes.begin(), m_renderNodes.end());
    std::sort(m_uiRenderNodes.begin(), m_uiRenderNodes.end());
}

void C_EntityManager::performInterpolation(const C_EntityManager* prevSnapshot, const C_EntityManager* nextSnapshot, double elapsedTime, double totalTime)
{
    if (!prevSnapshot) {
        std::cout << "performInterpolation error - Previous snapshot doesn't exist" << std::endl;
        return;
    }

    if (!nextSnapshot) {
        std::cout << "performInterpolation error - Next snapshot doesn't exist" << std::endl;
        return;
    }

    C_ManagersContext context(this, m_tileMap, nullptr);

    //interpolate entities
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        const C_Entity* prevEntity = prevSnapshot->entities.atUniqueId(it->getUniqueId());
        const C_Entity* nextEntity = nextSnapshot->entities.atUniqueId(it->getUniqueId());

        it->interpolate(prevEntity, nextEntity, elapsedTime, totalTime, it->getUniqueId() == m_controlledEntityUniqueId);
    }

    //interpolate projectiles
    for (int i = 0; i < projectiles.firstInvalidIndex(); ++i) {
        C_Projectile& projectile = projectiles[i];

        const C_Projectile* prevProj = prevSnapshot->projectiles.atUniqueId(projectile.uniqueId);
        const C_Projectile* nextProj = nextSnapshot->projectiles.atUniqueId(projectile.uniqueId);

        C_Projectile_interpolate(projectile, prevProj, nextProj, elapsedTime, totalTime);
    }
}

void C_EntityManager::copySnapshotData(const C_EntityManager* snapshot, u32 latestAppliedInputId)
{
    //controlledEntity might change
    m_controlledEntityUniqueId = snapshot->getControlledEntityUniqueId();

    //copy entities that don't exist locally
    for (auto it = snapshot->entities.begin(); it != snapshot->entities.end(); ++it) {
        C_Entity* currentEntity = entities.atUniqueId(it->getUniqueId());

        if (!currentEntity) {
            entities.addEntity(it->clone());
        } else {
            currentEntity->copySnapshotData(&it, it->getUniqueId() == m_controlledEntityUniqueId);
        }
    }

    for (int i = 0; i < snapshot->projectiles.firstInvalidIndex(); ++i) {
        const C_Projectile& snapshotProj = snapshot->projectiles[i];

        int index = projectiles.getIndexByUniqueId(snapshotProj.uniqueId);

        if (index == -1) {
            int thisIndex = projectiles.addElement(snapshotProj.uniqueId);
            projectiles[thisIndex] = snapshotProj;
        }
    }

    auto it = entities.begin();

    //remove entities not in the snapshot
    while (it != entities.end()) {
        const C_Entity* snapshotEntity = snapshot->entities.atUniqueId(it->getUniqueId());

        if (!snapshotEntity) {
            it = entities.removeEntity(it);
        } else {
            ++it;
        }
    }

    int i = 0;

    while (i < projectiles.firstInvalidIndex()) {
        const C_Projectile* snapshotProj = snapshot->projectiles.atUniqueId(projectiles[i].uniqueId);

        if (!snapshotProj) {
            projectiles.removeElement(projectiles[i].uniqueId);
        } else {
            i++;
        }
    }

    i = 0;

    //@TODO: Smoothly interpolate to the received position instead of 
    //instantly deleting them (?)
    while (i < localProjectiles.firstInvalidIndex()) {
        //remove local projectiles that have already been simulated in server
        if (localProjectiles[i].createdInputId <= latestAppliedInputId) {
            localProjectiles.removeElement(localProjectiles[i].uniqueId);
        } else {
            i++;
        }
    }
}

void C_EntityManager::updateRevealedUnits()
{
    //TODO: Maybe a quadtree is needed for this?
    //this operation is O(n*m) with n and m being units and alive players in your team
    //since teams are usually small (<4 players), this method should be fine

    //O(n + m*n) where n is units and m is units on the same team

    C_ManagersContext context(this, m_tileMap, nullptr);

    //locally update if each entity is visible or not
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        it->updateLocallyVisible(context);
    }

    //reveal entities locally if they meet the conditions
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        //only entities of this team can reveal other entities
        if (it->getTeamId() != getLocalTeamId()) continue;

        for (auto it2 = entities.begin(); it2 != entities.end(); ++it2) {
            it->localReveal(&it2);
        }
    }
}

C_Entity* C_EntityManager::createEntity(u8 entityType, u32 uniqueId)
{
    if (entityType < 0 || entityType >= ENTITY_MAX_TYPES) {
        std::cout << "C_EntityManager::createEntity error - Invalid entity type" << std::endl;
        return nullptr;
    }

    C_Entity* entity = nullptr;

    entity = m_entityData[entityType]->clone();
    entity->setUniqueId(uniqueId);

    entities.addEntity(entity);

    return entity;
}

C_Projectile* C_EntityManager::createProjectile(u8 type, const Vector2& pos, float aimAngle, u8 teamId)
{
    //create locally predicted projectile
    //add it to the locally predicted array
    if (type < 0 || type >= PROJECTILE_MAX_TYPES) {
        std::cout << "EntityManager::createProjectile error - Invalid type" << std::endl;
        return nullptr;
    }

    u32 uniqueId = localLastUniqueId++;
    int index = localProjectiles.addElement(uniqueId);

    C_Projectile& projectile = localProjectiles[index];

    C_Projectile_init(projectile, type, pos, aimAngle);

    projectile.uniqueId = uniqueId;
    projectile.teamId = teamId;

    return &projectile;
}

void C_EntityManager::loadFromData(C_EntityManager* prevSnapshot, CRCPacket& inPacket, CasterSnapshot& casterSnapshot)
{
    //number of units
    u16 entityNumber;
    inPacket >> entityNumber;

    for (int i = 0; i < entityNumber; ++i) {
        u32 uniqueId;
        inPacket >> uniqueId;

        const C_Entity* prevEntity = nullptr;
        
        if (prevSnapshot) {
            prevEntity = prevSnapshot->entities.atUniqueId(uniqueId);
        }

        //allocate the unit

        C_Entity* entity = nullptr;

        if (prevEntity) {
            //if the unit existed in previous snapshot, copy it
            entity = entities.addEntity(prevEntity->clone());

        } else {
            u8 entityType;
            inPacket >> entityType;
            
            //otherwise initialize it
            entity = createEntity(entityType, uniqueId);

            //Entity creation callbacks might go here?
            //Or is it better to have them when the entity is rendered for the first time?
        }

        //in both cases it has to be loaded from packet
        entity->loadFromData(getControlledEntityUniqueId(), inPacket, casterSnapshot);
    }

    u16 projectileNumber;
    inPacket >> projectileNumber;

    for (int i = 0; i < projectileNumber; ++i) {
        u32 uniqueId;
        inPacket >> uniqueId;

        C_Projectile* prevProj = nullptr;

        if (prevSnapshot) {
            prevProj = prevSnapshot->projectiles.atUniqueId(uniqueId);
        }

        int index = projectiles.addElement(uniqueId);

        if (prevProj) {
            projectiles[index] = *prevProj;

        } else {
            u8 type;
            inPacket >> type;

            C_Projectile_init(projectiles[index], (ProjectileType) type);
            projectiles[index].uniqueId = uniqueId;
        }

        C_Projectile_loadFromData(projectiles[index], inPacket);
    }
}

void C_EntityManager::allocateAll()
{
    projectiles.resize(MAX_PROJECTILES);
    localProjectiles.resize(MAX_PROJECTILES);
}

void C_EntityManager::setTileMap(TileMap* tileMap)
{
    m_tileMap = tileMap;
}

u8 C_EntityManager::getLocalTeamId() const
{
    return isHeroDead() ? m_spectatingEntityTeamId : m_controlledEntityTeamId;
}

u32 C_EntityManager::getLocalUniqueId() const
{
    return isHeroDead() ? m_spectatingEntityUniqueId : m_controlledEntityUniqueId;
}

u8 C_EntityManager::getControlledEntityTeamId() const
{
    return m_controlledEntityTeamId;
}

void C_EntityManager::setControlledEntityTeamId(u8 teamId)
{
    m_controlledEntityTeamId = teamId;
}

u32 C_EntityManager::getControlledEntityUniqueId() const
{
    return m_controlledEntityUniqueId;
}

void C_EntityManager::setControlledEntityUniqueId(u32 uniqueId)
{
    m_controlledEntityUniqueId = uniqueId;
}

void C_EntityManager::setSpectatingEntityTeamId(u8 teamId)
{
    m_spectatingEntityTeamId = teamId;
}

void C_EntityManager::setSpectatingEntityUniqueId(u32 uniqueId)
{
    m_spectatingEntityUniqueId = uniqueId;
}

bool C_EntityManager::isHeroDead() const
{
    return (entities.atUniqueId(m_controlledEntityUniqueId) == nullptr);
}

void C_EntityManager::setHeroDead(bool heroDead)
{
    m_heroDead = heroDead;
}

std::vector<RenderNode>& C_EntityManager::getRenderNodes()
{
    return m_renderNodes;
}

std::vector<RenderNode>& C_EntityManager::getUIRenderNodes()
{
    return m_uiRenderNodes;
}

void C_EntityManager::loadEntityData(const Context& context)
{
    if (m_entitiesJsonLoaded) return;

    #define DoEntity(class_name, type, json_id) \
        m_entityData[ENTITY_##type] = std::unique_ptr<C_Entity>(new C_##class_name()); \
        m_entityData[ENTITY_##type]->setEntityType(ENTITY_##type); \
        m_entityData[ENTITY_##type]->loadFromJson(*(context.jsonParser->getDocument(json_id)), TextureId::type, context);
    #include "entities.inc"
    #undef DoEntity

    m_entitiesJsonLoaded = true;
}

C_Entity* C_EntityManager::getEntityData(u8 entityType)
{
	if (entityType >= ENTITY_MAX_TYPES) return nullptr;

	return m_entityData[entityType].get();
}

void C_EntityManager::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (renderingEntitiesUI) {
        for (const auto& node : m_uiRenderNodes) {
            if (node.usingSprite) {
                target.draw(node.sprite, states);
            } else {
                target.draw(*node.drawable, states);
            }

#ifdef MANDARINA_DEBUG
            if (renderingEntityData) {
                sf::Text text;
                text.setPosition(node.position + Vector2(40.f, -40.f));
                text.setFillColor(sf::Color::Red);
                text.setFont(m_context.fonts->getResource("keep_calm_font"));
                text.setCharacterSize(11);
                text.setString(node.debugDisplayData);
                target.draw(text, states);
            }
#endif
        }

    } else {
        for (const auto& node : m_renderNodes) {
            if (node.usingSprite) {
                target.draw(node.sprite, states);
            } else {
                target.draw(*node.drawable, states);
            }

#ifdef MANDARINA_DEBUG
            if (renderingDebug) {
                sf::CircleShape shape;
                shape.setRadius(node.collisionRadius);
                shape.setOrigin(shape.getRadius(), shape.getRadius());
                shape.setFillColor(sf::Color(255, 0, 0, 80));
                shape.setOutlineColor(sf::Color::Red);
                shape.setOutlineThickness(1.5f);
                shape.setPosition(node.position);
                target.draw(shape, states);

                Vector2 heightPos = Vector2(node.position.x, node.height);
                sf::Vertex line[2];
                line[0].position = heightPos - Vector2(node.collisionRadius, 0.f);
                line[0].color  = sf::Color::Red;
                line[1].position = heightPos + Vector2(node.collisionRadius, 0.f);
                line[1].color = sf::Color::Red;
                target.draw(line, 2, sf::Lines);
            }
#endif
        }
    }
}

bool C_EntityManager::m_entitiesJsonLoaded = false;
std::unique_ptr<C_Entity> C_EntityManager::m_entityData[ENTITY_MAX_TYPES];


