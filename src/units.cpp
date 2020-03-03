#include "units.hpp"

#include "texture_ids.hpp"
#include "helper.hpp"
#include "bit_stream.hpp"

#include "texture_ids.hpp"

#include "collision_manager.hpp"
#include "server_entity_manager.hpp"
#include "client_entity_manager.hpp"
#include "quadtree.hpp"

#include "tilemap.hpp"

RechargeType rechargeTypeFromStr(const std::string& str)
{
    if (str == "Cooldown") {
        return RechargeType::Cooldown;
    } else if (str == "Passive") {
        return RechargeType::Passive;
    } else if (str == "Points") {
        return RechargeType::Points;
    }

    return RechargeType::None;
}

Ability g_abilities[ABILITY_MAX_TYPES];

void _loadAbility(JsonParser* jsonParser, AbilityType type, Ability& ability, const char* json_id)
{
    auto* doc = jsonParser->getDocument(json_id);

    ability.type = type;
    ability.rechargeType = rechargeTypeFromStr((*doc)["recharge_type"].GetString());

    if (ability.rechargeType == RechargeType::Cooldown) {
        ability.abCooldown.maxCharges = (*doc)["charges"].GetInt();
        ability.abCooldown.cooldown = (*doc)["cooldown"].GetFloat();

        if (doc->HasMember("charge_rate")) {
            ability.abCooldown.chargeRate = (*doc)["charge_rate"].GetFloat();
        } else {
            ability.abCooldown.chargeRate = 0.f;
        }

        if (doc->HasMember("starting_charges")) {
            ability.abCooldown.currentCharges = (*doc)["starting_charges"].GetInt();
        } else {
            ability.abCooldown.currentCharges = ability.abCooldown.maxCharges;
        }

        if (doc->HasMember("starting_cooldown")) {
            ability.abCooldown.currentCooldown = (*doc)["starting_cooldown"].GetFloat();
        } else {
            ability.abCooldown.currentCooldown = 0.f;
        }

    } else if (ability.rechargeType == RechargeType::Points) {
        if (doc->HasMember("starting_percentage")) {
            ability.abPoints.pointPercentage = Helper_clamp((*doc)["starting_percentage"].GetFloat(), 0.f, 1.f);
        } else {
            ability.abPoints.pointPercentage = 0.f;
        }

        if (doc->HasMember("points_multiplier")) {
            ability.abPoints.pointPercentage = (*doc)["points_multiplier"].GetFloat();
        } else {
            ability.abPoints.pointPercentage = 1.f;
        }

        ability.abPoints.pointsMultiplier = 1.f;
    }
}

void loadAbilitiesFromJson(JsonParser* jsonParser)
{
    #define LoadAbility(ability_name, json_id) \
        _loadAbility(jsonParser, ABILITY_##ability_name, g_abilities[ABILITY_##ability_name], json_id);
    #include "abilities.inc"
    #undef LoadAbility
}
void Ability_update(Ability& ability, sf::Time eTime)
{
    float dt = eTime.asSeconds();

    switch (ability.rechargeType) 
    {
        case RechargeType::Cooldown:
        {
            if (ability.abCooldown.currentCharges < ability.abCooldown.maxCharges) {
                if (ability.abCooldown.currentCooldown != 0.f) {
                    ability.abCooldown.currentCooldown = std::max(ability.abCooldown.currentCooldown - dt, 0.f);

                    if (std::fmod(ability.abCooldown.currentCooldown, ability.abCooldown.cooldown) == 0) {
                        ability.abCooldown.currentCharges++;
                    }
                }
            }
            break;
        }
    }
}

void Ability_onCallback(Ability& ability)
{
    switch (ability.rechargeType) 
    {
        case RechargeType::Cooldown:
        {
            if (ability.abCooldown.currentCharges != 0) {
                ability.abCooldown.currentCharges--;
                ability.abCooldown.currentCooldown += ability.abCooldown.cooldown;
            }
            break;
        }

        case RechargeType::Points:
        {
            ability.abPoints.pointPercentage = 0;
            break;
        }
    }
}

bool Ability_canBeCasted(const Ability& ability)
{
    switch (ability.rechargeType) 
    {
        case RechargeType::Cooldown:
        {
            return ability.abCooldown.currentCharges > 0;
        }

        case RechargeType::Points:
        {
            return ability.abPoints.pointPercentage >= 1;
        }
    }

    return false;
}

WeaponData g_weaponData[WEAPON_MAX_TYPES];

void _loadWeapon(JsonParser* jsonParser, WeaponData& weaponData, const char* json_id, u16 textureId, u8 primaryFire, u8 secondaryFire)
{
    auto* doc = jsonParser->getDocument(json_id);

    weaponData.textureId = textureId;
    weaponData.primaryFire = primaryFire;
    weaponData.secondaryFire = secondaryFire;

    if (doc->HasMember("scale")) {
        weaponData.scale = (*doc)["scale"].GetFloat();
    } else {
        weaponData.scale = 1.f;
    }

    if (doc->HasMember("angle_offset")) {
        weaponData.angleOffset = (*doc)["angle_offset"].GetFloat();
    } else {
        weaponData.angleOffset = 0.f;
    }
}

void loadWeaponsFromJson(JsonParser* jsonParser)
{
    #define LoadWeapon(weapon_name, callback_func, json_id, primary_fire_id, secondary_fire_id) \
        _loadWeapon(jsonParser, g_weaponData[WEAPON_##weapon_name], json_id, TextureId::weapon_name, primary_fire_id, secondary_fire_id); \
        // g_weaponData[WEAPON_##weapon_name].callback = &WeaponCallback_##callback_func;
    #include "weapons.inc"
    #undef LoadWeapon
}

//Placeholder callback
void WeaponCallback_devilsBow()
{
    printf("Devils bow fired!\n");
}

bool UnitStatus_equals(const UnitStatus& lhs, const UnitStatus& rhs)
{
    return lhs.stunTime == rhs.stunTime &&
           lhs.rootTime == rhs.rootTime &&
           lhs.disarmTime == rhs.disarmTime &&
           lhs.invisTime == rhs.invisTime &&
           lhs.solid == rhs.solid &&
           lhs.illusion == rhs.illusion;
}

void UnitStatus_packData(const UnitStatus& status, u8 teamId, CRCPacket& packet)
{
    BitStream stream;

    stream.pushBit(status.stunTime > sf::Time::Zero);
    stream.pushBit(status.rootTime > sf::Time::Zero);
    stream.pushBit(status.disarmTime > sf::Time::Zero);
    stream.pushBit(status.invisTime > sf::Time::Zero);
    stream.pushBit(status.solid);
    stream.pushBit(status.illusion);

    packet << stream.popByte();
}

void C_UnitStatus_loadFromData(C_UnitStatus& status, CRCPacket& packet)
{
    BitStream stream;

    u8 byte;
    packet >> byte;
    stream.pushByte(byte);

    status.stunned = stream.popBit();
    status.rooted = stream.popBit();
    status.disarmed = stream.popBit();
    status.invisible = stream.popBit();
    status.solid = stream.popBit();
    status.illusion = stream.popBit();
}

Unit g_initialUnitData[UNIT_MAX_TYPES];
C_Unit g_initialCUnitData[UNIT_MAX_TYPES];

bool _BaseUnit_loadFromJson(JsonParser* jsonParser, UnitType type, const char* json_id, _BaseUnitData& unit, u8 weaponId, u8 altAbilityId, u8 ultimateId)
{
    auto* doc = jsonParser->getDocument(json_id);

    if (doc == nullptr) {
        std::cout << "_Unit_loadFromJson error - Invalid json id " << json_id << std::endl;
        return false;
    }

    unit.type = type;

    unit.primaryFire = g_abilities[g_weaponData[weaponId].primaryFire];
    unit.secondaryFire = g_abilities[g_weaponData[weaponId].secondaryFire];
    unit.altAbility = g_abilities[altAbilityId];
    unit.ultimate = g_abilities[ultimateId];

    if (doc->HasMember("flying_height")) {
        unit.flyingHeight = (*doc)["flying_height"].GetInt();
    } else {
        unit.flyingHeight = 0;
    }
    
    unit.maxHealth = (*doc)["max_health"].GetInt();

    if (doc->HasMember("health")) {
        unit.health = (*doc)["health"].GetInt();
    } else {
        unit.health = unit.maxHealth;
    }

    unit.movementSpeed = (*doc)["movement_speed"].GetInt();

    unit.aimAngle = 0.f;
    unit.weaponId = weaponId;
    unit.collisionRadius = (*doc)["collision_radius"].GetInt();

    if (doc->HasMember("true_sight_radius")) {
        unit.trueSightRadius = (*doc)["true_sight_radius"].GetInt();
    } else {
        unit.trueSightRadius = 170;
    }

    return true;
}

void _Unit_loadFromJson(JsonParser* jsonParser, UnitType type, const char* json_id, Unit& unit, u8 weaponId, u8 altAbilityId, u8 ultimateId)
{
    bool result = _BaseUnit_loadFromJson(jsonParser, type, json_id, unit, weaponId, altAbilityId, ultimateId);

    if (result) {
        unit.dead = false;
    }
}

void _C_Unit_loadFromJson(JsonParser* jsonParser, UnitType type, const char* json_id, C_Unit& unit, u16 textureId, u8 weaponId, u8 altAbilityId, u8 ultimateId)
{
    bool result = _BaseUnit_loadFromJson(jsonParser, type, json_id, unit, weaponId, altAbilityId, ultimateId);

    if (result) {
        unit.textureId = textureId;
        
        auto* doc = jsonParser->getDocument(json_id);

        if (doc->HasMember("scale")) {
            unit.scale = (*doc)["scale"].GetFloat();
        } else {
            unit.scale = 1.f;
        }
    }
}

void loadUnitsFromJson(JsonParser* jsonParser)
{
    #define LoadUnit(unit_name, texture_id, json_id, weapon_id, alt_ability_id, ultimate_id) \
        _Unit_loadFromJson(jsonParser, UNIT_##unit_name, json_id, g_initialUnitData[UNIT_##unit_name], weapon_id, alt_ability_id, ultimate_id);

    #include "units.inc"
    #undef LoadUnit
}

void C_loadUnitsFromJson(JsonParser* jsonParser)
{
    #define LoadUnit(unit_name, texture_id, json_id, weapon_id, alt_ability_id, ultimate_id) \
        _C_Unit_loadFromJson(jsonParser, UNIT_##unit_name, json_id, g_initialCUnitData[UNIT_##unit_name], TextureId::texture_id, weapon_id, alt_ability_id, ultimate_id);

    #include "units.inc"
    #undef LoadUnit
}

void Unit_init(Unit& unit, UnitType type)
{
    if (type < 0 || type >= UNIT_MAX_TYPES) {
        return;
    }

    unit = g_initialUnitData[type];
}

void C_Unit_init(C_Unit& unit, UnitType type)
{
    if (type < 0 || type >= UNIT_MAX_TYPES) {
        return;
    }

    unit = g_initialCUnitData[type];
}

void Unit_packData(const Unit& unit, const Unit* prevUnit, u8 teamId, CRCPacket& outPacket)
{
    UnitStatus_packData(unit.status, teamId, outPacket);

    BitStream mainBits;
    
    bool posXChanged = !prevUnit || unit.pos.x != prevUnit->pos.x;
    mainBits.pushBit(posXChanged);

    bool posYChanged = !prevUnit || unit.pos.y != prevUnit->pos.y;
    mainBits.pushBit(posYChanged);

    bool teamIdChanged = !prevUnit || unit.teamId != prevUnit->teamId;
    mainBits.pushBit(teamIdChanged);

    bool flyingHeightChanged = !prevUnit || unit.flyingHeight != prevUnit->flyingHeight;
    mainBits.pushBit(flyingHeightChanged);
    
    bool maxHealthChanged = !prevUnit || unit.maxHealth != prevUnit->maxHealth;
    mainBits.pushBit(maxHealthChanged);

    bool healthChanged = !prevUnit || unit.health != prevUnit->health;
    mainBits.pushBit(healthChanged);

    bool aimAngleChanged = !prevUnit || unit.aimAngle != prevUnit->aimAngle;
    mainBits.pushBit(aimAngleChanged);

    bool collisionRadiusChanged = !prevUnit || unit.collisionRadius != prevUnit->collisionRadius;
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
        outPacket << unit.pos.x;
    }

    if (posYChanged) {
        outPacket << unit.pos.y;
    }

    if (teamIdChanged) {
        outPacket << unit.teamId;
    }

    if (flyingHeightChanged) {
        outPacket << unit.flyingHeight;
    }

    if (maxHealthChanged) {
        outPacket << unit.maxHealth;
    }

    if (healthChanged) {
        outPacket << unit.health;
    }

    if (aimAngleChanged) {
        outPacket << Helper_angleTo16bit(unit.aimAngle);
    }
    
    if (collisionRadiusChanged) {
        outPacket << unit.collisionRadius;
    }
}

void C_Unit_loadFromData(C_Unit& unit, CRCPacket& inPacket)
{
    C_UnitStatus_loadFromData(unit.status, inPacket);

    BitStream mainBits;

    u8 byte;
    inPacket >> byte;
    mainBits.pushByte(byte);
    inPacket >> byte;
    mainBits.pushByte(byte);

    bool posXChanged = mainBits.popBit();
    bool posYChanged = mainBits.popBit();
    bool teamIdChanged = mainBits.popBit();
    bool flyingHeightChanged = mainBits.popBit();
    bool maxHealthChanged = mainBits.popBit();
    bool healthChanged = mainBits.popBit();
    bool aimAngleChanged = mainBits.popBit();
    bool collisionRadiusChanged = mainBits.popBit();
    unit.status.forceSent = mainBits.popBit();

    if (posXChanged) {
        inPacket >> unit.pos.x;
    }

    if (posYChanged) {
        inPacket >> unit.pos.y;
    }

    if (teamIdChanged) {
        inPacket >> unit.teamId;
    }

    if (flyingHeightChanged) {
        inPacket >> unit.flyingHeight;
    }

    if (maxHealthChanged) {
        inPacket >> unit.maxHealth;
    }

    if (healthChanged) {
        inPacket >> unit.health;
    }

    if (aimAngleChanged) {
        u16 angle16bit;
        inPacket >> angle16bit;
        unit.aimAngle = Helper_angleFrom16bit(angle16bit);
    }

    if (collisionRadiusChanged) {
        inPacket >> unit.collisionRadius;
    }
}

//used by client and server
Vector2 _moveColliding_impl(const Vector2& oldPos, const Vector2& newPos, float collisionRadius, const _BaseUnitData* collisionUnit)
{
    //these values are copied to avoid constatly traversing the pointer
    const Vector2 collisionPos = collisionUnit->pos;
    const float collisionUnitRadius = collisionUnit->collisionRadius;

    const Vector2 fixVector = newPos - collisionPos;
    const Vector2 fixDir = Helper_vec2unitary(fixVector);
    
    //should we force the unit to slide if it's moving exactly towards the other center?
    // if (Helper_pointsCollinear(oldPos, newPos, collisionPos)) {
    //     fixVector += Vector2(-fixDir.y, fixDir.x) * 1.f;
    //     fixDir = Helper_vec2unitary(fixVector);
    // }

    const float distance = Helper_vec2length(fixVector);
    const float targetDistance = std::abs(collisionUnitRadius + collisionRadius - distance);

    //push the unit in a random direction if it's too close
    if (distance == 0) {
        return Helper_vec2unitary(Vector2(1.f, 1.f)) * targetDistance;
    }

    return fixDir * targetDistance;
}

Vector2 _moveCollidingMap_impl(const Vector2& oldPos, Vector2 newPos, float collisionRadius, TileMap* map)
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

//has to work for both Unit and C_Unit (kind of ugly, but we need to access status.solid for both unit types)
template<typename UnitType>
//checks two units can collide (not the same unit, both solid, same flying height, etc)
bool _Unit_canCollide(const UnitType& unit1, const UnitType& unit2)
{
    //@TODO: Check flyingHeight is correct
    return unit1.uniqueId != unit2.uniqueId && unit1.status.solid && unit2.status.solid;
}

//move while checking collision
void Unit_moveColliding(Unit& unit, Vector2 newPos, const ManagersContext& context, bool force)
{
    if (!force && newPos == unit.pos) return;
    
    //since the collision we need is very simple, 
    //we'll only perform it against the closest unit
    const Unit* closestUnit = nullptr;
    float closestDistance = 0.f;

    if (unit.status.solid) {      
        const Circlef circle(newPos, unit.collisionRadius);

        auto query = context.collisionManager->getQuadtree()->QueryIntersectsRegion(BoundingBody<float>(circle));

        while (!query.EndOfQuery()) {
            u32 collisionUniqueId = query.GetCurrent()->uniqueId;
            const Unit* collisionUnit = context.entityManager->units.atUniqueId(collisionUniqueId);

            if (collisionUnit) {
                if (_Unit_canCollide(unit, *collisionUnit)) {
                    float distance = Helper_vec2length(collisionUnit->pos - newPos);

                    if (!closestUnit || distance < closestDistance) {
                        closestUnit = collisionUnit;
                        closestDistance = distance;
                    }
                }
            }

            //@TODO: Check in case the uniqueId is another type of entity 
            //that could block the unit (like a scene object that's not part of the tilemap)

            query.Next();
        }

        if (closestUnit) {
            newPos += _moveColliding_impl(unit.pos, newPos, unit.collisionRadius, closestUnit);
        }
    }

    unit.pos = _moveCollidingMap_impl(unit.pos, newPos, unit.collisionRadius, context.tileMap);

    context.collisionManager->onUpdateUnit(unit.uniqueId, unit.pos, unit.collisionRadius);
}

void Unit_update(Unit& unit, sf::Time eTime, const ManagersContext& context)
{
    Ability_update(unit.primaryFire, eTime);
    Ability_update(unit.secondaryFire, eTime);
    Ability_update(unit.altAbility, eTime);
    Ability_update(unit.ultimate, eTime);

    Vector2 newPos = unit.pos;

    //@DELETE
    //we're adding 20 units before we add the player (TESTING)
    //so we move all units but the player randomly
    // if (unit.uniqueId < 210) {
    //     if (unit.vel == Vector2()) {
    //         unit.vel = Vector2(rand() % 200 - 100.f, rand() % 200 - 100.f);
    //     }
    // }

    newPos += unit.vel * eTime.asSeconds();
    //other required movement (like dragged movement, forces and friction, etc)

    if (newPos != unit.pos) {
        Unit_moveColliding(unit, newPos, context);
    }

    unit.status.inBush = context.tileMap->isColliding(TILE_BUSH, Circlef(unit.pos, unit.collisionRadius));

    //reveal of all units inside true sight radius
    //we also send units slightly farther away so revealing them looks smooth on the client
    Circlef trueSightCircle(unit.pos, (float) unit.trueSightRadius + 100.f);
    auto query = context.collisionManager->getQuadtree()->QueryIntersectsRegion(BoundingBody<float>(trueSightCircle));

    while (!query.EndOfQuery()) {
        Unit* revealedUnit = context.entityManager->units.atUniqueId(query.GetCurrent()->uniqueId);

        if (revealedUnit) {
            if (!Unit_shouldBeHiddenFrom(unit, *revealedUnit)) {
                //if the unit is inside true sight radius, reveal it
                Unit_revealUnit(*revealedUnit, unit.teamId);
            }

            //we mark it to send since it's inside the bigger circle
            Unit_markToSend(*revealedUnit, unit.teamId);
        }

        query.Next();
    }

    //@DELETE
    unit.aimAngle += 100 * eTime.asSeconds();
}

void Unit_preUpdate(Unit& unit, sf::Time eTime, const ManagersContext& context)
{
    unit.visionFlags = 0;
    unit.teamSentFlags = 0;
}

void Unit_revealUnit(Unit& unit, u8 teamId)
{
    unit.visionFlags |= (1 << teamId);
}

void Unit_markToSend(Unit& unit, u8 teamId)
{
    unit.teamSentFlags |= (1 << teamId);
}

bool Unit_isInvisible(const Unit& unit)
{
    return unit.status.invisTime > sf::Time::Zero || unit.status.inBush;
}

bool C_Unit_isInvisible(const C_Unit& unit)
{
    return unit.status.invisible || unit.status.inBush;
}

bool Unit_isRevealedForTeam(const Unit& unit, u8 teamId)
{
    return (unit.visionFlags & (1 << teamId));
}

bool Unit_isVisibleForTeam(const Unit& unit, u8 teamId)
{
    if (unit.teamId == teamId || !Unit_isInvisible(unit)) {
        return true;
    } else {
        return Unit_isRevealedForTeam(unit, teamId);
    }
}

bool Unit_isMarkedToSendForTeam(const Unit& unit, u8 teamId)
{
    return (unit.teamSentFlags & (1 << teamId));
}

bool _shouldBeHiddenFrom_impl(const Vector2& unitPos, const Vector2& otherPos, float trueSightRadius, bool unitInBush, bool otherInBush)
{
    if (Helper_vec2length(otherPos - unitPos) >= trueSightRadius) {
        return true;
    } else {
        if (otherInBush) {
            return !unitInBush;
        } else {
            return false;
        }
    }
}

bool Unit_shouldBeHiddenFrom(const Unit& unit, const Unit& otherUnit)
{
    return _shouldBeHiddenFrom_impl(unit.pos, otherUnit.pos, unit.trueSightRadius,
                                    unit.status.inBush, otherUnit.status.inBush);
}

bool C_Unit_shouldBeHiddenFrom(const C_Unit& unit, const C_Unit& otherUnit)
{
    return _shouldBeHiddenFrom_impl(unit.pos, otherUnit.pos, unit.trueSightRadius,
                                    unit.status.inBush, otherUnit.status.inBush);
}

void C_Unit_interpolate(C_Unit& unit, const C_ManagersContext& context, const C_Unit* prevUnit, const C_Unit* nextUnit, double t, double d)
{
    if (prevUnit && nextUnit) {
        //only interpolate position and aimAngle for units we're not controlling
        if (unit.uniqueId != context.entityManager->controlledEntityTeamId) {
            unit.pos = Helper_lerpVec2(prevUnit->pos, nextUnit->pos, t, d);
            unit.aimAngle = Helper_lerpAngle(prevUnit->aimAngle, nextUnit->aimAngle, t, d);
        }
    }

    if (!prevUnit && nextUnit) {
        //This is the first time the entity is being rendered
        //creation callbacks
    }

    if (prevUnit && !nextUnit) {
        //This is the last time entity is being rendered
        //destruction callbacks
    }
}

void Unit_applyInput(Unit& unit, const PlayerInput& input, const ManagersContext& context, u16 clientDelay)
{
    //@TODO: Check flags to see if we can actually move (stunned, rooted, etc)

    Vector2 newPos = unit.pos;

    unit.aimAngle = input.aimAngle;

    //@WIP: Check cooldowns and the type of ability to call proper callback
    //We cast abilities before moving so it works like in the client
    //(because in the client inputs don't move the unit instantaneously)
    if (input.primaryFire && Ability_canBeCasted(unit.primaryFire)) {
        int uniqueId = context.entityManager->createProjectile(PROJECTILE_HellsBubble, unit.pos, unit.aimAngle, unit.teamId);

        if (uniqueId != -1) {
            Projectile* projectile = context.entityManager->projectiles.atUniqueId(uniqueId);

            //@WIP: Do all this automatically for every projectile generated
            //It has to work even for callbacks that generate multiple projectiles
            if (projectile) {
                Ability_onCallback(unit.primaryFire);

                float totalDistance = ((float) clientDelay / 1000.f) * projectile->movementSpeed;

                //do no more than 5 iteration steps
                int steps = std::min((int) (totalDistance / projectile->collisionRadius), 5);
                sf::Time stepTime = sf::milliseconds(clientDelay/steps);
                int i = 0;

                while (i < steps && !projectile->dead) {
                    Projectile_update(*projectile, stepTime, context);
                    i++;
                }
            }
        }
    }
    
    bool moved = PlayerInput_repeatAppliedInput(input, newPos, unit.movementSpeed);

    if (moved) {
        Unit_moveColliding(unit, newPos, context);
    }
}

//we don't need to use this function outside this file for now
void _C_Unit_predictMovement(const C_Unit& unit, const Vector2& oldPos, Vector2& newPos, const C_ManagersContext& context)
{
    //we don't update the circle to mimic how it works in the server
    //where we only use one Query
    const Circlef circle(newPos, unit.collisionRadius);
 
    const C_Unit* closestUnit = nullptr;
    float closestDistance = 0.f;

    for (int i = 0; i < context.entityManager->units.firstInvalidIndex(); ++i) {
        const C_Unit& otherUnit = context.entityManager->units[i];

        if (_Unit_canCollide(unit, otherUnit)) {
            const Circlef otherCircle(otherUnit.pos, otherUnit.collisionRadius);

            if (circle.intersects(otherCircle)) {
                float distance = Helper_vec2length(otherUnit.pos - newPos);

                if (!closestUnit || distance < closestDistance) {
                    closestUnit = &otherUnit;
                    closestDistance = distance;
                }
            }
        }
    }

    if (closestUnit) {
        newPos += _moveColliding_impl(oldPos, newPos, unit.collisionRadius, closestUnit);
    }

    newPos = _moveCollidingMap_impl(oldPos, newPos, unit.collisionRadius, context.tileMap);
}

void C_Unit_applyInput(const C_Unit& unit, Vector2& unitPos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt)
{
    //@TODO: Check flags to see if we can actually move (stunned, rooted, etc)

    Vector2 oldPos = unitPos;
    bool moved = PlayerInput_applyInput(input, unitPos, unit.movementSpeed, dt);

    if (moved) {
        _C_Unit_predictMovement(unit, oldPos, unitPos, context);
    }
}

void C_Unit_applyAbilitiesInput(const C_Unit& unit, const PlayerInput& input, const C_ManagersContext& context)
{
    //@WIP
    //Check if it can be casted
    //Start reducing the cooldown
    //If casting fails on server, correct the cooldown in client

    if (input.primaryFire) {
        int uniqueId = context.entityManager->createProjectile(PROJECTILE_HellsBubble, unit.pos, unit.aimAngle, unit.teamId);

        if (uniqueId != -1) {
            context.entityManager->localProjectiles.atUniqueId(uniqueId)->createdInputId = input.id;
        }
    }
}
