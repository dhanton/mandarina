#include "unit.hpp"

#include "bit_stream.hpp"
#include "tilemap.hpp"
#include "client_entity_manager.hpp"

Unit::Unit(u32 uniqueId):
    Entity(uniqueId),
    InvisibleComponent(teamId, pos, inBush),
    TrueSightComponent(pos, inBush)
{

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

void Unit::packData(const Unit* prevUnit, u8 teamId, CRCPacket& outPacket)
{
    // UnitStatus_packData(status, teamId, outPacket);

    BitStream mainBits;
    
    bool posXChanged = !prevUnit || pos.x != prevUnit->pos.x;
    mainBits.pushBit(posXChanged);

    bool posYChanged = !prevUnit || pos.y != prevUnit->pos.y;
    mainBits.pushBit(posYChanged);

    bool teamIdChanged = !prevUnit || teamId != prevUnit->teamId;
    mainBits.pushBit(teamIdChanged);

    bool flyingHeightChanged = !prevUnit || flyingHeight != prevUnit->flyingHeight;
    mainBits.pushBit(flyingHeightChanged);
    
    bool maxHealthChanged = !prevUnit || maxHealth != prevUnit->maxHealth;
    mainBits.pushBit(maxHealthChanged);

    bool healthChanged = !prevUnit || health != prevUnit->health;
    mainBits.pushBit(healthChanged);

    bool aimAngleChanged = !prevUnit || aimAngle != prevUnit->aimAngle;
    mainBits.pushBit(aimAngleChanged);

    bool collisionRadiusChanged = !prevUnit || collisionRadius != prevUnit->collisionRadius;
    mainBits.pushBit(collisionRadiusChanged);

    //units mark to send are never visible
    mainBits.pushBit(Unit_isMarkedToSendForTeam(unit, teamId));

    //send the flags
    u8 byte1 = mainBits.popByte();
    u8 byte2 = mainBits.popByte();
    outPacket << byte1;
    outPacket << byte2;

    //this didn't work (probably the order was incorrect??)
    // outPacket << mainBits.popByte() << mainBits.popByte();

    //send the data if it has changed
    if (posXChanged) {
        outPacket << pos.x;
    }

    if (posYChanged) {
        outPacket << pos.y;
    }

    if (teamIdChanged) {
        outPacket << teamId;
    }

    if (flyingHeightChanged) {
        outPacket << flyingHeight;
    }

    if (maxHealthChanged) {
        outPacket << m_maxHealth;
    }

    if (healthChanged) {
        outPacket << m_health;
    }

    if (aimAngleChanged) {
        outPacket << Helper_angleTo16bit(aimAngle);
    }
    
    if (collisionRadiusChanged) {
        outPacket << collisionRadius;
    }
}

C_Unit::C_Unit(u32 uniqueId):
    C_Entity(uniqueId),
    InvisibleComponent(teamId, pos, inBush),
    TrueSightComponent(pos, inBush)
{

}

C_Unit* C_Unit::clone() const
{
    return new Unit(*this);
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

void C_Unit::updateLocallyVisible(const C_ManagersContext& context)
{
    inBush = context.tileMap->isColliding(TILE_BUSH, Circlef(pos, (float) collisionRadius));
    locallyHidden = isInvisible();
}

void C_Unit::localReveal(const C_Unit* unit)
{
    if (unit->locallyHidden && !unit->shouldBeHiddenFrom(this)) {
        unit->locallyHidden = false;
    }
}