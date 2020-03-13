#include "unit.hpp"

#include "texture_ids.hpp"
#include "bit_stream.hpp"
#include "tilemap.hpp"
#include "client_entity_manager.hpp"
#include "server_entity_manager.hpp"
#include "weapon.hpp"
#include "collision_manager.hpp"

namespace {

Vector2 _moveColliding_impl(const Vector2& oldPos, const Vector2& newPos, float collisionRadius, const BaseEntityComponent& collisionEntity)
{
    const Vector2 collisionPos = collisionEntity.getPosition();
    const float collisionEntityRadius = collisionEntity.getCollisionRadius();

    const Vector2 fixVector = newPos - collisionPos;
    const Vector2 fixDir = Helper_vec2unitary(fixVector);
    
    //should we force the unit to slide if it's moving exactly towards the other center?
    // if (Helper_pointsCollinear(oldPos, newPos, collisionPos)) {
    //     fixVector += Vector2(-fixDir.y, fixDir.x) * 1.f;
    //     fixDir = Helper_vec2unitary(fixVector);
    // }

    const float distance = Helper_vec2length(fixVector);
    const float targetDistance = std::abs(collisionEntityRadius + collisionRadius - distance);

    //push the unit in a random direction if it's too close
    if (distance == 0) {
        return Helper_vec2unitary(Vector2(1.f, 1.f)) * targetDistance;
    }

    return fixDir * targetDistance;
}

}

float _UnitBase::getAimAngle() const
{
    return m_aimAngle;
}

void _UnitBase::setAimAngle(float aimAngle)
{
    m_aimAngle = aimAngle;
}

u8 _UnitBase::getMovementSpeed() const
{
    return m_movementSpeed;
}

void _UnitBase::setMovementSpeed(u8 movementSpeed)
{
    m_movementSpeed = movementSpeed;
}

Vector2 _UnitBase::moveCollidingTilemap_impl(const Vector2& oldPos, Vector2 newPos, float collisionRadius, TileMap* map)
{
    const Vector2 posMovingX = {newPos.x, oldPos.y};
    const Vector2 posMovingY = {oldPos.x, newPos.y};

    sf::FloatRect collidingTile;

    if (map->getCollidingTileRect(TILE_BLOCK | TILE_WALL, Circlef(posMovingX, collisionRadius), collidingTile)) {
        //we assume if there's collision there was movement 

        if (oldPos.x > newPos.x) {
            newPos.x = collidingTile.left + collidingTile.width + collisionRadius;

        } else if (oldPos.x < newPos.x) {
            newPos.x = collidingTile.left - collisionRadius;
        }
    }

    if (map->getCollidingTileRect(TILE_BLOCK | TILE_WALL, Circlef(posMovingY, collisionRadius), collidingTile)) {
        if (oldPos.y > newPos.y) {
            newPos.y = collidingTile.top + collidingTile.height + collisionRadius;

        } else if (oldPos.y < newPos.y) {
            newPos.y = collidingTile.top - collisionRadius;
        }
    }

    return newPos;
}

void _UnitBase::loadFromJson(const rapidjson::Document& doc)
{
    m_aimAngle = 0.f;
    m_weaponId = Weapon_stringToType(doc["weapon"].GetString());
    m_movementSpeed = doc["movement_speed"].GetInt();
}

Unit* Unit::clone() const
{
    return new Unit(*this);
}

void Unit::loadFromJson(const rapidjson::Document& doc)
{
    Entity::loadFromJson(doc);
    _UnitBase::loadFromJson(doc);
    HealthComponent::loadFromJson(doc);
    TrueSightComponent::loadFromJson(doc);
    CasterComponent::loadFromJson(doc);

    m_solid = true;
}

void Unit::update(sf::Time eTime, const ManagersContext& context)
{
    CasterComponent::update(eTime);

    Vector2 newPos = m_pos;

    //@DELETE
    //move all units of team 1 randomly (player is team 0)
    if (m_teamId == 1) {
        if (m_vel == Vector2()) {
            m_vel = Vector2(rand() % 200 - 100.f, rand() % 200 - 100.f);
        }
    }

    newPos += m_vel * eTime.asSeconds();
    //other required movement (like dragged movement, forces and friction, etc)

    if (newPos != m_pos) {
        moveColliding(newPos, context);
    }

    m_inBush = context.tileMap->isColliding(TILE_BUSH, Circlef(m_pos, m_collisionRadius));

    //reveal of all units inside true sight radius
    //we also send units slightly farther away so revealing them looks smooth on the client
    Circlef trueSightCircle(m_pos, (float) m_trueSightRadius + 100.f);
    auto query = context.collisionManager->getQuadtree()->QueryIntersectsRegion(BoundingBody<float>(trueSightCircle));

    while (!query.EndOfQuery()) {
        Entity* revealedEntity = context.entityManager->entities.atUniqueId(query.GetCurrent()->uniqueId);

        //we don't need to reveal units of the same team
        if (revealedEntity->getTeamId() == m_teamId) {
            query.Next();
            continue;
        }

        //@TODO: Maybe we should change this dynamic_cast using some sort of virtual function or component flags
        //Only if this could be bottlenecking performance
        InvisibleComponent* invisComp = dynamic_cast<InvisibleComponent*>(revealedEntity);

        if (invisComp) {
            if (!invisComp->shouldBeHiddenFrom(*this)) {
                //if the unit is inside true sight radius, reveal it
                invisComp->reveal(m_teamId);
            }

            //we mark it to send since it's inside the bigger circle
            invisComp->markToSend(m_teamId);
        }

        query.Next();
    }
}

void Unit::preUpdate(sf::Time eTime, const ManagersContext& context)
{
    resetInvisibleFlags();
}

void Unit::postUpdate(sf::Time eTime, const ManagersContext& context)
{

}

void Unit::packData(const Entity* prevEntity, u8 teamId, CRCPacket& outPacket) const
{
    // UnitStatus_packData(status, teamId, outPacket);

    const Unit* prevUnit = static_cast<const Unit*>(prevEntity);

    BitStream mainBits;
    
    mainBits.pushBit(isInvisible());
    mainBits.pushBit(isSolid());

    bool posXChanged = !prevUnit || m_pos.x != prevUnit->getPosition().x;
    mainBits.pushBit(posXChanged);

    bool posYChanged = !prevUnit || m_pos.y != prevUnit->getPosition().y;
    mainBits.pushBit(posYChanged);

    bool teamIdChanged = !prevUnit || teamId != prevUnit->getTeamId();
    mainBits.pushBit(teamIdChanged);

    bool flyingHeightChanged = !prevUnit || m_flyingHeight != prevUnit->getFlyingHeight();
    mainBits.pushBit(flyingHeightChanged);
    
    bool maxHealthChanged = !prevUnit || m_maxHealth != prevUnit->getMaxHealth();
    mainBits.pushBit(maxHealthChanged);

    bool healthChanged = !prevUnit || m_health != prevUnit->getHealth();
    mainBits.pushBit(healthChanged);

    bool aimAngleChanged = !prevUnit || m_aimAngle != prevUnit->getAimAngle();
    mainBits.pushBit(aimAngleChanged);

    bool collisionRadiusChanged = !prevUnit || m_collisionRadius != prevUnit->getCollisionRadius();
    mainBits.pushBit(collisionRadiusChanged);

    //units mark to send are never visible
    mainBits.pushBit(isMarkedToSendForTeam(teamId));

    //send the flags
    u8 byte1 = mainBits.popByte();
    u8 byte2 = mainBits.popByte();
    outPacket << byte1;
    outPacket << byte2;

    //this didn't work (probably the order was incorrect??)
    // outPacket << mainBits.popByte() << mainBits.popByte();

    //send the data if it has changed
    if (posXChanged) {
        outPacket << m_pos.x;
    }

    if (posYChanged) {
        outPacket << m_pos.y;
    }

    if (teamIdChanged) {
        outPacket << m_teamId;
    }

    if (flyingHeightChanged) {
        outPacket << m_flyingHeight;
    }

    if (maxHealthChanged) {
        outPacket << m_maxHealth;
    }

    if (healthChanged) {
        outPacket << m_health;
    }

    if (aimAngleChanged) {
        outPacket << Helper_angleTo16bit(m_aimAngle);
    }
    
    if (collisionRadiusChanged) {
        outPacket << m_collisionRadius;
    }
}

void Unit::applyInput(const PlayerInput& input, const ManagersContext& context, u16 clientDelay)
{
    //@TODO: Check flags to see if we can actually move (stunned, rooted, etc)

    m_aimAngle = input.aimAngle;
    
    //We cast abilities before moving so it works like in the client
    //(because in the client inputs don't move the unit instantaneously)
    CasterComponent::applyInput(this, input, context, clientDelay);
    
    //we copy the position after applying abilities input 
    //in case abilities move the unit
    Vector2 newPos = m_pos;
    
    bool moved = PlayerInput_repeatAppliedInput(input, newPos, m_movementSpeed);

    if (moved) {
        moveColliding(newPos, context);
    }
}

bool Unit::shouldSendToTeam(u8 teamId) const
{
    return isVisibleForTeam(teamId) || isMarkedToSendForTeam(teamId);
}

void Unit::onQuadtreeInserted(const ManagersContext& context)
{

}

void Unit::moveColliding(Vector2 newPos, const ManagersContext& context, bool force)
{
    if (!force && newPos == m_pos) return;
    
    //since the collision we need is very simple, 
    //we'll only perform it against the closest unit
    const Entity* closestEntity = nullptr;
    float closestDistance = 0.f;

    if (m_solid) {      
        const Circlef circle(newPos, m_collisionRadius);

        auto query = context.collisionManager->getQuadtree()->QueryIntersectsRegion(BoundingBody<float>(circle));

        while (!query.EndOfQuery()) {
            u32 collisionUniqueId = query.GetCurrent()->uniqueId;
            // const Unit* collisionUnit = context.entityManager->units.atUniqueId(collisionUniqueId);

            const Entity* collisionEntity = context.entityManager->entities.atUniqueId(collisionUniqueId);

            if (collisionEntity) {
                if (canCollide(*collisionEntity)) {
                    float distance = Helper_vec2length(collisionEntity->getPosition() - newPos);

                    if (!closestEntity || distance < closestDistance) {
                        closestEntity = collisionEntity;
                        closestDistance = distance;
                    }
                }
            }

            query.Next();
        }

        if (closestEntity) {
            newPos += _moveColliding_impl(m_pos, newPos, m_collisionRadius, *closestEntity);
        }
    }

    m_pos = moveCollidingTilemap_impl(m_pos, newPos, m_collisionRadius, context.tileMap);

    context.collisionManager->onUpdateUnit(m_uniqueId, m_pos, m_collisionRadius);
}

C_Unit* C_Unit::clone() const
{
    return new C_Unit(*this);
}

void C_Unit::loadFromJson(const rapidjson::Document& doc, u16 textureId)
{
    C_Entity::loadFromJson(doc, textureId);
    _UnitBase::loadFromJson(doc);
    HealthComponent::loadFromJson(doc);
    TrueSightComponent::loadFromJson(doc);

    m_solid = true;
}

void C_Unit::update(sf::Time eTime, const C_ManagersContext& context)
{

}

void C_Unit::loadFromData(CRCPacket& inPacket)
{
    // C_UnitStatus_loadFromData(unit.status, inPacket);

    BitStream mainBits;

    u8 byte;
    inPacket >> byte;
    mainBits.pushByte(byte);
    inPacket >> byte;
    mainBits.pushByte(byte);

    setInvisible(mainBits.popBit());
    setSolid(mainBits.popBit());

    bool posXChanged = mainBits.popBit();
    bool posYChanged = mainBits.popBit();
    bool teamIdChanged = mainBits.popBit();
    bool flyingHeightChanged = mainBits.popBit();
    bool maxHealthChanged = mainBits.popBit();
    bool healthChanged = mainBits.popBit();
    bool aimAngleChanged = mainBits.popBit();
    bool collisionRadiusChanged = mainBits.popBit();
    setForceSent(mainBits.popBit());

    if (posXChanged) {
        inPacket >> m_pos.x;
    }

    if (posYChanged) {
        inPacket >> m_pos.y;
    }

    if (teamIdChanged) {
        inPacket >> m_teamId;
    }

    if (flyingHeightChanged) {
        inPacket >> m_flyingHeight;
    }

    if (maxHealthChanged) {
        inPacket >> m_maxHealth;
    }

    if (healthChanged) {
        inPacket >> m_health;
    }

    if (aimAngleChanged) {
        u16 angle16bit;
        inPacket >> angle16bit;
        m_aimAngle = Helper_angleFrom16bit(angle16bit);
    }

    if (collisionRadiusChanged) {
        inPacket >> m_collisionRadius;
    }
}

void C_Unit::interpolate(const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d, bool isControlled)
{
    const C_Unit* prevUnit = static_cast<const C_Unit*>(prevEntity);
    const C_Unit* nextUnit = static_cast<const C_Unit*>(nextEntity);

    if (prevUnit && nextUnit) {
        //only interpolate position and aimAngle for units we're not controlling
        if (!isControlled) {
            m_pos = Helper_lerpVec2(prevUnit->getPosition(), nextUnit->getPosition(), t, d);
            m_aimAngle = Helper_lerpAngle(prevUnit->getAimAngle(), nextUnit->getAimAngle(), t, d);
        }
    }

    if (!prevUnit && nextUnit) {
        //This is the first time the entity is being rendered
        //creation callbacks (?)
    }

    if (prevUnit && !nextUnit) {
        //This is the last time entity is being rendered
        //destruction callbacks (?)
    }
}

void C_Unit::copySnapshotData(const C_Entity* snapshotEntity, bool isControlled)
{
    Vector2 pos = getPosition();
    float aimAngle = getAimAngle();

    *this = *(static_cast<const C_Unit*>(snapshotEntity));

    if (isControlled) {
        //this is not really needed since the controlled entity position is
        //constatly being interpolated from the latest two inputs
        //but we do it just in case
        setPosition(pos);
        setAimAngle(aimAngle);
    }
}

void C_Unit::updateControlledAngle(float aimAngle)
{
    m_aimAngle = aimAngle;
}

void C_Unit::applyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt)
{
     //@TODO: Check flags to see if we can actually move (stunned, rooted, etc)

    Vector2 oldPos = pos;
    bool moved = PlayerInput_applyInput(input, pos, (float) m_movementSpeed, dt);

    if (moved) {
        predictMovementLocally(oldPos, pos, context);
    }
}

void C_Unit::reapplyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context)
{
    Vector2 oldPos = pos;
    bool moved = PlayerInput_repeatAppliedInput(input, pos, (float) m_movementSpeed);

    if (moved) {
        predictMovementLocally(oldPos, pos, context);
    }
}

u16 C_Unit::getControlledMovementSpeed() const
{
    return m_movementSpeed;
}

void C_Unit::updateLocallyVisible(const C_ManagersContext& context)
{
    m_inBush = context.tileMap->isColliding(TILE_BUSH, Circlef(m_pos, (float) m_collisionRadius));
    m_locallyHidden = isInvisibleOrBush();
}

void C_Unit::localReveal(C_Entity* entity)
{
    InvisibleComponent* invisComp = dynamic_cast<InvisibleComponent*>(entity);

    if (invisComp) {
        if (invisComp->isLocallyHidden() && !invisComp->shouldBeHiddenFrom(*this)) {
            invisComp->setLocallyHidden(false);
        }
    }
}

void C_Unit::insertRenderNode(const C_ManagersContext& managersContext, const Context& context) const
{
    bool renderingLocallyHidden = managersContext.entityManager->renderingLocallyHidden;

#ifdef MANDARINA_DEBUG
        if (!renderingLocallyHidden && m_locallyHidden && m_forceSent) return;
#else
        if (m_locallyHidden && m_forceSent) return;
#endif

    std::vector<RenderNode>& renderNodes = managersContext.entityManager->getRenderNodes();

    C_Entity::insertRenderNode(managersContext, context);

    sf::Sprite& sprite = renderNodes.back().sprite;

    int aimQuadrant = Helper_angleQuadrant(m_aimAngle);
    float mirrored = 1.f;

    if (aimQuadrant == 3 || aimQuadrant == 4) {
        mirrored = -1.f;
    }

    //flip the sprite depending on aimAngle
    sprite.setScale(mirrored * m_scale, m_scale);

    if (isInvisibleOrBush()) {
        sf::Color color = sprite.getColor();
        color.a = 150.f;

        if (m_locallyHidden && renderingLocallyHidden) {
            color.r += 100.f;    
        }

        sprite.setColor(color);
    }

    //setup the weapon node if equipped
    if (m_weaponId != WEAPON_NONE) {
        renderNodes.emplace_back(RenderNode(m_flyingHeight, m_uniqueId, (float) m_collisionRadius));

        const Weapon& weapon = g_weaponData[m_weaponId];
        sf::Sprite& weaponSprite = renderNodes.back().sprite;

        weaponSprite.setTexture(context.textures->getResource(weapon.textureId));
        weaponSprite.setScale(weapon.scale, weapon.scale);
        weaponSprite.setOrigin(weaponSprite.getLocalBounds().width/2.f, weaponSprite.getLocalBounds().height/2.f);
        weaponSprite.setPosition(getPosition());
        weaponSprite.setRotation(-getAimAngle() - weapon.angleOffset);

        //put the weapon behind or in front of the unit depending on the quadrant
        if (aimQuadrant == 2 || aimQuadrant == 3) {
            renderNodes.back().manualFilter = -1;
        } else {
            renderNodes.back().manualFilter = 1;
        }
    }

#ifdef MANDARINA_DEBUG
    std::string& dataString = renderNodes.back().debugDisplayData;
    dataString += std::to_string(m_uniqueId) + "\n";
    dataString += "Team:" + std::to_string(m_teamId) + "\n";
    dataString += "ForceSent:" + std::to_string(isForceSent()) + "\n";
#endif
}

void C_Unit::predictMovementLocally(const Vector2& oldPos, Vector2& newPos, const C_ManagersContext& context) const
{
    //we don't update the circle to mimic how it works in the server
    //where we only use one Query
    const Circlef circle(newPos, m_collisionRadius);
 
    const C_Entity* closestEntity = nullptr;
    float closestDistance = 0.f;

    auto it = context.entityManager->entities.begin();
    for (; it != context.entityManager->entities.end(); ++it) {
        if (!it->isSolid()) continue;

        if (canCollide(*it)) {
            const Circlef otherCircle(it->getPosition(), it->getCollisionRadius());

            if (circle.intersects(otherCircle)) {
                float distance = Helper_vec2length(it->getPosition() - newPos);

                if (!closestEntity || distance < closestDistance) {
                    closestEntity = &it;
                    closestDistance = distance;
                }
            }
        }
    }

    if (closestEntity) {
        newPos += _moveColliding_impl(oldPos, newPos, m_collisionRadius, *closestEntity);
    }

    newPos = moveCollidingTilemap_impl(oldPos, newPos, m_collisionRadius, context.tileMap);
}
