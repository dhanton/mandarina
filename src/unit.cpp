#include "unit.hpp"

#include "texture_ids.hpp"
#include "bit_stream.hpp"
#include "tilemap.hpp"
#include "client_entity_manager.hpp"
#include "server_entity_manager.hpp"
#include "weapon.hpp"
#include "collision_manager.hpp"
#include "game_mode.hpp"

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

u16 _UnitBase::getMovementSpeed() const
{
    return m_movementSpeed;
}

void _UnitBase::setMovementSpeed(u16 movementSpeed)
{
    m_movementSpeed = movementSpeed;
}

Status& _UnitBase::getStatus()
{
    return m_status;
}

const Status& _UnitBase::getStatus() const
{
    return m_status;
}

Vector2 _UnitBase::moveCollidingTilemap_impl(const Vector2& oldPos, Vector2 newPos, float collisionRadius, TileMap* map)
{
    sf::FloatRect collidingTile;

    int i = 0;

    while (map->getCollidingTileRect(TILE_BLOCK | TILE_WALL, Circlef(newPos, collisionRadius), collidingTile)) {
        const Vector2 dir = Helper_vec2unitary(newPos - Vector2(collidingTile.left + collidingTile.width/2.f, collidingTile.top + collidingTile.height/2.f));

        //calculate where the unit is coming from to know which side of
        //the tile it's gonna block its movement
        //(might fail if distance is too high)

        if (dir.y < SQRT2_INV && dir.y > -SQRT2_INV) {
            if (dir.x >= 0.f) {
                newPos.x = collidingTile.left + collidingTile.width + collisionRadius;
            } else {
                newPos.x = collidingTile.left - collisionRadius;
            }

        } else if (dir.x < SQRT2_INV && dir.x > -SQRT2_INV) {
            if (dir.y >= 0.f) {
                newPos.y = collidingTile.top + collidingTile.height + collisionRadius;
            } else {
                newPos.y = collidingTile.top - collisionRadius;
            }
        }

        //just in case, this shouldn't happen normally
        //(units perform only 1 iteration for walls, and 2 for corners)
        if (++i > 4) break;
    }

    return newPos;
}

void _UnitBase::loadFromJson(const rapidjson::Document& doc)
{
    m_aimAngle = 0.f;
    m_weaponId = Weapon_stringToType(doc["weapon"].GetString());
    m_movementSpeed = doc["movement_speed"].GetUint();
    m_baseMovementSpeed = m_movementSpeed;
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

    if (doc.HasMember("time_to_passive_heal")) {
        m_timeToPassiveHeal = sf::seconds(doc["time_to_passive_heal"].GetFloat());
    } else {
        m_timeToPassiveHeal = sf::seconds(3.5f);
    }

    //in max health percentage
    if (doc.HasMember("passive_healing_per_second")) {
        m_passiveHealingPerSecond = doc["passive_healing_per_second"].GetFloat();
    } else {
        m_passiveHealingPerSecond = 0.13;
    }

    m_solid = true;
}

void Unit::update(sf::Time eTime, const ManagersContext& context)
{
    checkDead(context);

    if (m_dead) return;

    //some abilities add buffs to the unit when created
    //areBuffsAdded is checked individually just in case abilities change or something like that
    if (!m_primaryFire->areBuffsAdded()) m_primaryFire->addBuffsToCaster(this, context);
    if (!m_secondaryFire->areBuffsAdded()) m_secondaryFire->addBuffsToCaster(this, context);
    if (!m_altAbility->areBuffsAdded()) m_altAbility->addBuffsToCaster(this, context);
    if (!m_ultimate->areBuffsAdded()) m_ultimate->addBuffsToCaster(this, context);

    CasterComponent::update(eTime, context.gameMode);

    //it's important to check every update in case the unit has moved using abilities or something else instead of input
    if (m_pos != m_prevPos || m_collisionRadius != m_prevCollisionRadius) {
        context.collisionManager->onUpdateEntity(m_uniqueId, m_pos, m_collisionRadius);

        m_prevPos = m_pos;
        m_prevCollisionRadius = m_collisionRadius;
    }

    context.gameMode->onUnitUpdate(this);

    m_inBush = context.tileMap->isColliding(TILE_BUSH, Circlef(m_pos, m_collisionRadius));

    //reveal of all units inside true sight radius
    //we also send units slightly farther away so revealing them looks smooth on the client
    Circlef trueSightCircle(m_pos, (float) m_trueSightRadius + 100.f);
    auto query = context.collisionManager->getQuadtree()->QueryIntersectsRegion(BoundingBody<float>(trueSightCircle));

    while (!query.EndOfQuery()) {
        Entity* revealedEntity = context.entityManager->entities.atUniqueId(query.GetCurrent()->uniqueId);

        //we don't need to reveal units of the same team
        if (!revealedEntity || revealedEntity->getTeamId() == m_teamId) {
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

                //we mark it inside the smaller circle
                invisComp->markToSendCloser(m_teamId);
            }

            //we mark it to send since it's inside the bigger circle
            invisComp->markToSend(m_teamId);
        }

        query.Next();
    }

    BuffHolderComponent::onUpdate(eTime);
}

void Unit::preUpdate(sf::Time eTime, const ManagersContext& context)
{
    //reset parameters
    resetInvisibleFlags();
    m_status.preUpdate();

    m_movementSpeed = m_baseMovementSpeed;

    //update parameters using buffs
    BuffHolderComponent::onPreUpdate(eTime);

    m_timeSinceDamaged += eTime;

    if (m_timeSinceDamaged >= m_timeToPassiveHeal) {
        m_passiveHealingTimer += eTime;

        if (m_passiveHealingTimer >= sf::seconds(1.f)) {
            m_passiveHealingTimer -= sf::seconds(1.f);

            beHealed(m_passiveHealingPerSecond * m_maxHealth, nullptr);
        }
    }
}

void Unit::postUpdate(sf::Time eTime, const ManagersContext& context)
{
    checkDead(context);
}

void Unit::packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const
{
    const Unit* prevUnit = static_cast<const Unit*>(prevEntity);

    BitStream mainBits;
    
    mainBits.pushBit(isInvisible());
    mainBits.pushBit(isSolid());
    mainBits.pushBit(m_status.stunned);
    mainBits.pushBit(m_status.silenced);
    mainBits.pushBit(m_status.disarmed);
    mainBits.pushBit(m_status.rooted);
    mainBits.pushBit(m_status.slowed);

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

    //tell the client if the unit is being revealed by some other method that's not close proximity
    mainBits.pushBit(isRevealedForTeam(teamId) && !isMarkedToSendCloserForTeam(teamId));

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

    if (m_uniqueId == controlledEntityUniqueId) {
        m_primaryFire->packData(outPacket);
        m_secondaryFire->packData(outPacket);
        m_altAbility->packData(outPacket);
        m_ultimate->packData(outPacket);
    }
}

float Unit::getDamageMultiplier() const
{
    float damageMultiplier = 1.f;

    BuffHolderComponent::onGetDamageMultiplier(damageMultiplier);
    return damageMultiplier;
}

void Unit::onTakeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId)
{
    BuffHolderComponent::onTakeDamage(damage, source, uniqueId, teamId);
    HealthComponent::onTakeDamage(damage, source, uniqueId, teamId);

    if (damage > 0) {
        m_timeSinceDamaged = sf::Time::Zero;
    }
}

void Unit::onBeHealed(u16 amount, Entity* source)
{
    BuffHolderComponent::onBeHealed(amount, source);
    HealthComponent::onBeHealed(amount, source);
}

void Unit::onDeath(bool& dead, const ManagersContext& context)
{
    BuffHolderComponent::onDeath(dead);
}

void Unit::applyInput(const PlayerInput& input, const ManagersContext& context, u16 clientDelay)
{
    m_aimAngle = input.aimAngle;

    CanCast_ExtraFlags extraFlags;
    extraFlags.primaryFire = m_status.canAttack();
    extraFlags.secondaryFire = m_status.canCast();
    extraFlags.altAbility = m_status.canCast();
    extraFlags.ultimate = m_status.canCast();

    //We cast abilities before moving so it works like in the client
    //(because in the client inputs don't move the unit instantaneously)
    CasterComponent::applyInput(this, input, context, clientDelay, extraFlags);

    if (m_status.canMove()) {
        //we copy the position after applying abilities input 
        //in case abilities move the unit
        Vector2 newPos = m_pos;
        
        bool moved = PlayerInput_repeatAppliedInput(input, newPos, m_movementSpeed);

        if (moved) {
            onMovement();
            moveColliding(newPos, context);
        }
    }
}

bool Unit::shouldSendToTeam(u8 teamId) const
{
    return isVisibleForTeam(teamId) || isMarkedToSendForTeam(teamId);
}

void Unit::onQuadtreeInserted(const ManagersContext& context)
{

}

void Unit::onPrimaryFireCasted()
{
    BuffHolderComponent::onPrimaryFireCasted();
}

void Unit::onSecondaryFireCasted()
{
    BuffHolderComponent::onSecondaryFireCasted();
}

void Unit::onAltAbilityCasted()
{
    BuffHolderComponent::onAltAbilityCasted();
}

void Unit::onUltimateCasted()
{
    BuffHolderComponent::onUltimateCasted();
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

    context.collisionManager->onUpdateEntity(m_uniqueId, m_pos, m_collisionRadius);
}

void Unit::checkDead(const ManagersContext& context)
{
    if (!m_dead && m_health == 0) {
        m_dead = true;

        //m_dead is passed as a reference so that buffs can prevent a unit from dying
        onDeath(m_dead, context);

        if (m_dead) {
            u32 killerUniqueId = getLatestDamageDealer();
            Entity* entity = context.entityManager->entities.atUniqueId(killerUniqueId);

            if (entity) {
                Unit* unit = dynamic_cast<Unit*>(entity);

                if (unit) {
                    unit->onEntityKill(this);
                }
            }
        }
    }
}

C_Unit* C_Unit::clone() const
{
    return new C_Unit(*this);
}

void C_Unit::loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context)
{
    C_Entity::loadFromJson(doc, textureId, context);
    _UnitBase::loadFromJson(doc);
    HealthComponent::loadFromJson(doc);
    TrueSightComponent::loadFromJson(doc);

    m_solid = true;
    m_locallyHidden = false;
    m_serverRevealed = true;
    m_invisible = false;
}

void C_Unit::update(sf::Time eTime, const C_ManagersContext& context)
{

}

void C_Unit::loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot)
{
    BitStream mainBits;

    u8 byte;
    inPacket >> byte;
    mainBits.pushByte(byte);
    inPacket >> byte;
    mainBits.pushByte(byte);

    m_invisible = mainBits.popBit();
    setSolid(mainBits.popBit());
    m_status.stunned = mainBits.popBit();
    m_status.silenced = mainBits.popBit();
    m_status.disarmed = mainBits.popBit();
    m_status.rooted = mainBits.popBit();
    m_status.slowed = mainBits.popBit();

    bool posXChanged = mainBits.popBit();
    bool posYChanged = mainBits.popBit();
    bool teamIdChanged = mainBits.popBit();
    bool flyingHeightChanged = mainBits.popBit();
    bool maxHealthChanged = mainBits.popBit();
    bool healthChanged = mainBits.popBit();
    bool aimAngleChanged = mainBits.popBit();
    bool collisionRadiusChanged = mainBits.popBit();
    setServerRevealed(mainBits.popBit());

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

    if (m_uniqueId == controlledEntityUniqueId) {
        casterSnapshot.loadFromData(inPacket);
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
    UnitUI unitUI = m_unitUI;
    HealthUI healthUI = m_healthUI;

    _doSnapshotCopy(snapshotEntity);

    m_unitUI = unitUI;
    m_healthUI = healthUI;

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
    if (!m_status.canMove()) return;

    Vector2 oldPos = pos;
    bool moved = PlayerInput_applyInput(input, pos, (float) m_movementSpeed, dt);

    if (moved) {
        predictMovementLocally(oldPos, pos, context);
    }
}

void C_Unit::reapplyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context)
{
    if (!m_status.canMove()) return;
    
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
    m_locallyHidden = m_inBush || m_invisible;
}

void C_Unit::localReveal(C_Entity* entity)
{
    //@WIP: We should actually reveal C_InvisibleComponent entities
    C_Unit* unit = dynamic_cast<C_Unit*>(entity);

    if (unit) {
        if (unit->isLocallyHidden() && !unit->shouldBeHiddenFrom(*this)) {
            unit->setLocallyHidden(false);
        }
    }
}

void C_Unit::insertRenderNode(const C_ManagersContext& managersContext, const Context& context)
{
#ifdef MANDARINA_DEBUG
    bool renderingLocallyHidden = managersContext.entityManager->renderingLocallyHidden;
    if (!renderingLocallyHidden && m_locallyHidden && !m_serverRevealed) return;
#else
    if (m_locallyHidden && !m_serverRevealed) return;
#endif

    std::vector<RenderNode>& renderNodes = managersContext.entityManager->getRenderNodes();
    C_Entity::insertRenderNode(managersContext, context);

    const float unitHeight = renderNodes.back().height;

    sf::Sprite& sprite = renderNodes.back().sprite;

    int aimQuadrant = Helper_angleQuadrant(m_aimAngle);
    float mirrored = 1.f;

    if (aimQuadrant == 3 || aimQuadrant == 4) {
        mirrored = -1.f;
    }

    //flip the sprite depending on aimAngle
    sprite.setScale(mirrored * m_scale, m_scale);

    if (m_inBush || m_invisible) {
        sf::Color color = sprite.getColor();
        color.a = 150.f;

#ifdef MANDARINA_DEBUG
        if (m_locallyHidden && renderingLocallyHidden) {
            color.r += 100.f;    
        }
#endif

        sprite.setColor(color);
    }

    std::vector<RenderNode>& uiRenderNodes = managersContext.entityManager->getUIRenderNodes();

    //add unit UI
    if (!m_unitUI.getUnit()) {
        m_unitUI.setUnit(this);
        m_unitUI.setFonts(context.fonts);
        m_unitUI.setTextureLoader(context.textures);
    }

    //add health UI
    if (!m_healthUI.getEntity()) {
        m_healthUI.setEntity(this);
        m_healthUI.setHealthComponent(this);
        m_healthUI.setFonts(context.fonts);
    }

    //isAlly can change if the client changes the team its spectating
    bool isAlly = (m_teamId == managersContext.entityManager->getLocalTeamId());

    if (!m_healthUI.getEntity() || isAlly != m_healthUI.getIsAlly()) {
        m_healthUI.setIsAlly(m_teamId == managersContext.entityManager->getLocalTeamId());
    }

    m_healthUI.setIsControlledEntity(m_uniqueId == managersContext.entityManager->getControlledEntityUniqueId());

    uiRenderNodes.emplace_back(m_uniqueId);
    uiRenderNodes.back().usingSprite = false;
    uiRenderNodes.back().drawable = &m_unitUI;
    uiRenderNodes.back().height = getPosition().y;

    uiRenderNodes.emplace_back(m_uniqueId);
    uiRenderNodes.back().usingSprite = false;
    uiRenderNodes.back().drawable = &m_healthUI;
    uiRenderNodes.back().height = getPosition().y;

#ifdef MANDARINA_DEBUG
    uiRenderNodes.back().position = getPosition();
    std::string& dataString = uiRenderNodes.back().debugDisplayData;
    dataString += std::to_string(m_uniqueId) + "\n";
    // dataString += "Team: " + std::to_string(m_teamId) + "\n";
    // dataString += "ServerRevealed: " + std::to_string(isServerRevealed()) + "\n";
    // dataString += "LocallyHidden: " + std::to_string(m_locallyHidden) + "\n";
    // dataString += "Is Solid: " + std::to_string(m_solid) + "\n";
    dataString += std::to_string(getPosition().x) + "\n";
    dataString += std::to_string(getPosition().y) + "\n";
#endif

    //setup the weapon node if equipped
    if (m_weaponId != WEAPON_NONE) {
        renderNodes.emplace_back(m_uniqueId);
        renderNodes.back().usingSprite = true;

        const Weapon& weapon = g_weaponData[m_weaponId];
        sf::Sprite& weaponSprite = renderNodes.back().sprite;

        weaponSprite.setTexture(context.textures->getResource(weapon.textureId));
        weaponSprite.setScale(weapon.scale, weapon.scale);
        weaponSprite.setOrigin(weaponSprite.getLocalBounds().width/2.f, weaponSprite.getLocalBounds().height/2.f);
        weaponSprite.setPosition(getPosition());
        weaponSprite.setRotation(-getAimAngle() - weapon.angleOffset);

        renderNodes.back().height = unitHeight;

        //put the weapon behind or in front of the unit depending on the quadrant
        if (aimQuadrant == 2 || aimQuadrant == 3) {
            renderNodes.back().manualFilter = -1;
        } else {
            renderNodes.back().manualFilter = 1;
        }

#ifdef MANDARINA_DEBUG
        //weapons don't need to draw collision debug circle
        //but we do it so it's rendered above the unit
        renderNodes.back().collisionRadius = (float) m_collisionRadius;
        renderNodes.back().position = getPosition();
#endif
    }
}

UnitUI* C_Unit::getUnitUI()
{
    return &m_unitUI;
}

const UnitUI* C_Unit::getUnitUI() const
{
    return &m_unitUI;
}

HealthUI* C_Unit::getHealthUI()
{
    return &m_healthUI;
}

const HealthUI* C_Unit::getHealthUI() const
{
    return &m_healthUI;
}

bool C_Unit::isLocallyHidden() const
{
    return m_locallyHidden;
}

void C_Unit::setLocallyHidden(bool locallyHidden)
{
    m_locallyHidden = locallyHidden;
}

bool C_Unit::isServerRevealed() const
{
    return m_serverRevealed;
}

void C_Unit::setServerRevealed(bool serverRevealed)
{
    m_serverRevealed = serverRevealed;
}

bool C_Unit::shouldBeHiddenFrom(const C_Unit& unit) const
{
    if (m_teamId == unit.m_teamId) {
        return false;
    } else if (Helper_vec2length(m_pos - unit.m_pos) >= unit.getTrueSightRadius()) {
        return true;
    } else {
        return m_inBush ? !unit.m_inBush : false;
    }
}

void C_Unit::_doSnapshotCopy(const C_Entity* snapshotEntity)
{
    *this = *(static_cast<const C_Unit*>(snapshotEntity));
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
