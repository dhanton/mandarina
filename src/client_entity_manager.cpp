#include "client_entity_manager.hpp"

#include <algorithm>
#include <SFML/Graphics.hpp>

#include "helper.hpp"
#include "tilemap.hpp"

RenderNode::RenderNode(float flyingHeight, u32 uniqueId, float collisionRadius)
{
    this->flyingHeight = flyingHeight;
    this->uniqueId = uniqueId;
    this->collisionRadius = collisionRadius;

    manualFilter = 0;
}

inline bool RenderNode::operator<(const RenderNode& other) 
{
    float height = sprite.getPosition().y + flyingHeight;
    float otherHeight = other.sprite.getPosition().y + other.flyingHeight;

    if (height == otherHeight) {
        if (uniqueId == other.uniqueId) {
            return manualFilter < other.manualFilter;
        }

        return uniqueId < other.uniqueId;
    }

    return height < otherHeight;
}

C_EntityManager::C_EntityManager(const Context& context, sf::Time worldTime):
    InContext(context)
{
    localLastUniqueId = 0;
    controlledEntityUniqueId = 0;
    controlledEntityTeamId = 0;
    renderingDebug = false;
    renderingLocallyHidden = false;
}

C_EntityManager::C_EntityManager():
    //dummy context used if this instance is a snapshot
    InContext(Context())
{
    controlledEntityUniqueId = 0;    
}

void C_EntityManager::update(sf::Time eTime)
{
    C_ManagersContext context(this, m_tileMap);
    int i = 0;

    while (i < localProjectiles.firstInvalidIndex()) {
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
    C_ManagersContext context(this, m_tileMap);

    for (int i = 0; i < localProjectiles.firstInvalidIndex(); ++i) {
        C_Projectile_localUpdate(localProjectiles[i], eTime, context);
    }
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

    C_ManagersContext context(this, m_tileMap);

    //interpolate units
    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        C_Unit& unit = units[i];

        const C_Unit* prevUnit = prevSnapshot->units.atUniqueId(unit.uniqueId);
        const C_Unit* nextUnit = nextSnapshot->units.atUniqueId(unit.uniqueId);

        C_Unit_interpolate(unit, context, prevUnit, nextUnit, elapsedTime, totalTime);
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
    controlledEntityUniqueId = snapshot->controlledEntityUniqueId;

    const C_Unit* controlledUnit = snapshot->units.atUniqueId(controlledEntityUniqueId);

    //copy only entities that don't exist locally
    for (int i = 0; i < snapshot->units.firstInvalidIndex(); ++i) {
        const C_Unit& snapshotUnit = snapshot->units[i];

        int index = units.getIndexByUniqueId(snapshotUnit.uniqueId);

        if (index == -1) {
            int thisIndex = units.addElement(snapshotUnit.uniqueId);
            units[thisIndex] = snapshotUnit;
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

    int i = 0;

    //remove units not in the snapshot
    while (i < units.firstInvalidIndex()) {
        const C_Unit* snapshotUnit = snapshot->units.atUniqueId(units[i].uniqueId);

        if (!snapshotUnit) {
            units.removeElement(units[i].uniqueId);
        } else {
            i++;
        }
    }

    i = 0;

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

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        units[i].status.inBush = m_tileMap->isColliding(TILE_BUSH, Circlef(units[i].pos, units[i].collisionRadius));
        units[i].status.locallyHidden = C_Unit_isInvisible(units[i]);
    }

    //see which units are locally hidden
    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        if (units[i].teamId == controlledEntityTeamId) {
            //check all units inside true sight range for everyone on the team
            for (int j = 0; j < units.firstInvalidIndex(); ++j) {
                if (!C_Unit_shouldBeHiddenFrom(units[i], units[j])) {
                    units[j].status.locallyHidden = false;
                } 
            }
        }
    }
}

int C_EntityManager::createProjectile(ProjectileType type, const Vector2& pos, float aimAngle, u8 teamId)
{
    //create locally predicted projectile
    //add it to the locally predicted array
    if (type < 0 || type >= PROJECTILE_MAX_TYPES) {
        std::cout << "EntityManager::createProjectile error - Invalid type" << std::endl;
        return -1;
    }

    u32 uniqueId = localLastUniqueId++;
    int index = localProjectiles.addElement(uniqueId);

    C_Projectile& projectile = localProjectiles[index];

    C_Projectile_init(projectile, type, pos, aimAngle);

    projectile.uniqueId = uniqueId;
    projectile.teamId = teamId;

    return uniqueId;
}

//@TODO: Implementation is very similar to method above
//Generalize in one single _copyFromSnapshot_impl method 
//that can take a packet or just the snapshot
void C_EntityManager::loadFromData(C_EntityManager* prevSnapshot, CRCPacket& inPacket)
{
    if (prevSnapshot) {
        controlledEntityUniqueId = prevSnapshot->controlledEntityUniqueId;
    }

    //number of units
    u16 unitNumber;
    inPacket >> unitNumber;

    for (int i = 0; i < unitNumber; ++i) {
        u32 uniqueId;
        inPacket >> uniqueId;

        C_Unit* prevUnit = nullptr;
        
        if (prevSnapshot) {
            prevUnit = prevSnapshot->units.atUniqueId(uniqueId);
        }

        //allocate the unit
        int index = units.addElement(uniqueId);

        if (prevUnit) {
            //if the unit existed in previous snapshot, copy it
            units[index] = *prevUnit;

        } else {
            u8 unitType;
            inPacket >> unitType;
            
            //otherwise initialize it
            C_Unit_init(units[index], (UnitType) unitType);
            units[index].uniqueId = uniqueId;

            //Entity creation callbacks might go here?
            //Or is it better to have them when the entity is rendered for the first time?
        }

        //in both cases it has to be loaded from packet
        C_Unit_loadFromData(units[index], inPacket);
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
    units.resize(MAX_UNITS);
    projectiles.resize(MAX_PROJECTILES);
    localProjectiles.resize(MAX_PROJECTILES);
}

void C_EntityManager::setTileMap(TileMap* tileMap)
{
    m_tileMap = tileMap;
}

void C_EntityManager::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    //We have to sort all objects together by their height (accounting for flying objects)
    //(this has to be done every frame, otherwise the result might not look super good)
    //**vector sorting is faster because of the O(1) access operation**

    std::vector<RenderNode> spriteNodes;

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
#ifdef MANDARINA_DEBUG
        if (!renderingLocallyHidden && units[i].status.locallyHidden && units[i].status.forceSent) continue;
#else
        if (units[i].status.locallyHidden && units[i].status.forceSent) continue;
#endif

        spriteNodes.emplace_back(RenderNode(units[i].flyingHeight, units[i].uniqueId, (float) units[i].collisionRadius));

        sf::Sprite& sprite = spriteNodes.back().sprite;

        int aimQuadrant = Helper_angleQuadrant(units[i].aimAngle);
        float mirrored = 1.f;

        //flip the sprite depending on aimAngle
        if (aimQuadrant == 3 || aimQuadrant == 4) {
            mirrored = -1.f;
        }

        sprite.setTexture(m_context.textures->getResource(units[i].textureId));
        sprite.setScale(mirrored * units[i].scale, units[i].scale);
        sprite.setOrigin(sprite.getLocalBounds().width/2.f, sprite.getLocalBounds().height/2.f);
        sprite.setPosition(units[i].pos);

        if (C_Unit_isInvisible(units[i])) {
            sf::Color color = sprite.getColor();
            color.a = 150.f;

            if (units[i].status.locallyHidden && renderingLocallyHidden) {
                color.r += 100.f;    
            }

            sprite.setColor(color);
        }

        //setup the weapon node if equipped
        if (units[i].weaponId != WEAPON_NONE) {
            spriteNodes.emplace_back(RenderNode(units[i].flyingHeight, units[i].uniqueId, (float) units[i].collisionRadius));

            const WeaponData& weaponData = g_weaponData[units[i].weaponId];
            sf::Sprite& weaponSprite = spriteNodes.back().sprite;

            weaponSprite.setTexture(m_context.textures->getResource(weaponData.textureId));
            weaponSprite.setScale(weaponData.scale, weaponData.scale);
            weaponSprite.setOrigin(weaponSprite.getLocalBounds().width/2.f, weaponSprite.getLocalBounds().height/2.f);
            weaponSprite.setPosition(units[i].pos);
            weaponSprite.setRotation(-units[i].aimAngle - weaponData.angleOffset);

            //put the weapon behind or in front of the unit depending on the quadrant
            if (aimQuadrant == 2 || aimQuadrant == 3) {
                spriteNodes.back().manualFilter = -1;
            } else {
                spriteNodes.back().manualFilter = 1;
            }
        }
    }

    for (int i = 0; i < projectiles.firstInvalidIndex(); ++i) {
        //@WIP: flyingHeight = shooter.height + shooter.flyingHeight
        spriteNodes.emplace_back(RenderNode(100, projectiles[i].uniqueId, (float) projectiles[i].collisionRadius));

        sf::Sprite& sprite = spriteNodes.back().sprite;

        sprite.setTexture(m_context.textures->getResource(projectiles[i].textureId));
        sprite.setScale(projectiles[i].scale, projectiles[i].scale);
        sprite.setOrigin(sprite.getLocalBounds().width/2.f, sprite.getLocalBounds().height/2.f);
        sprite.setPosition(projectiles[i].pos);
    }

    for (int i = 0; i < localProjectiles.firstInvalidIndex(); ++i) {
        //@WIP: flyingHeight = shooter.height + shooter.flyingHeight
        spriteNodes.emplace_back(RenderNode(100, localProjectiles[i].uniqueId, (float) localProjectiles[i].collisionRadius));

        sf::Sprite& sprite = spriteNodes.back().sprite;

        sprite.setTexture(m_context.textures->getResource(localProjectiles[i].textureId));
        sprite.setScale(localProjectiles[i].scale, localProjectiles[i].scale);
        sprite.setOrigin(sprite.getLocalBounds().width/2.f, sprite.getLocalBounds().height/2.f);
        sprite.setPosition(localProjectiles[i].pos);
    }

    std::sort(spriteNodes.begin(), spriteNodes.end());

    for (const auto& node : spriteNodes) {
        target.draw(node.sprite, states);

#ifdef MANDARINA_DEBUG
        if (renderingDebug) {
            sf::CircleShape shape;
            shape.setRadius(node.collisionRadius);
            shape.setOrigin(shape.getRadius(), shape.getRadius());
            shape.setFillColor(sf::Color(255, 0, 0, 80));
            shape.setOutlineColor(sf::Color::Red);
            shape.setOutlineThickness(1.5f);
            shape.setPosition(node.sprite.getPosition());
            target.draw(shape, states);
        }
#endif
    }
}
