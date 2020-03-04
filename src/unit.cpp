#include "unit.hpp"

#include "bit_stream.hpp"
#include "tilemap.hpp"
#include "client_entity_manager.hpp"
#include "weapon.hpp"

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

Unit::Unit(u32 uniqueId):
    Entity(uniqueId),
    InvisibleComponent(m_teamId, m_pos, m_inBush),
    TrueSightComponent(m_pos, m_inBush)
{
    //@BRANCH_WIP
    //We need a static boolean that tells us if the units jsons are loaded
    //If its false, we load them here
    //If its true we just use it normally
    //Units are loaded into a global array (like they are now)
}

Unit* Unit::clone() const
{
    return new Unit(*this);
}

void Unit::update(sf::Time eTime, const ManagersContext& context)
{

}

void Unit::preUpdate(sf::Time eTime, const ManagersContext& context)
{

}

void Unit::postUpdate(sf::Time eTime, const ManagersContext& context)
{

}

void Unit::packData(const Unit* prevUnit, u8 teamId, CRCPacket& outPacket) const
{
    // UnitStatus_packData(status, teamId, outPacket);

    BitStream mainBits;
    
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

bool Unit::shouldSendForTeam(u8 teamId) const
{
    return isVisibleForTeam(teamId) || isMarkedToSendForTeam(teamId);
}

void Unit::onQuadtreeInserted(const ManagersContext& context)
{

}

bool Unit::inQuadtree() const
{
    return true;
}

C_Unit::C_Unit(u32 uniqueId):
    C_Entity(uniqueId),
    InvisibleComponent(m_teamId, m_pos, m_inBush),
    TrueSightComponent(m_pos, m_inBush)
{

}

C_Unit* C_Unit::clone() const
{
    return new C_Unit(*this);
}

void C_Unit::update(sf::Time eTime, const C_ManagersContext& context)
{

}

void C_Unit::loadFromData(CRCPacket& inPacket)
{


}

void C_Unit::interpolate(const C_ManagersContext& context, const C_Unit* prevUnit, const C_Unit* nextUnit, double t, double d)
{

}

void C_Unit::updateControlledAngle(float aimAngle)
{
    m_aimAngle = aimAngle;
}

void C_Unit::applyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt)
{

}

void C_Unit::applyAbilitiesInput(const PlayerInput& input, const C_ManagersContext& context)
{

}

u16 C_Unit::getControlledMovementSpeed() const
{
    return m_movementSpeed;
}

void C_Unit::updateLocallyVisible(const C_ManagersContext& context)
{
    m_inBush = context.tileMap->isColliding(TILE_BUSH, Circlef(m_pos, (float) m_collisionRadius));
    m_locallyHidden = isInvisible();
}

void C_Unit::localReveal(C_Unit* unit)
{
    if (unit->m_locallyHidden && !unit->shouldBeHiddenFrom(*this)) {
        unit->m_locallyHidden = false;
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

    if (isInvisible()) {
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
}