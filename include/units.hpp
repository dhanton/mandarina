#pragma once

#include <SFML/System/Time.hpp>
#include <unordered_map>

#include "defines.hpp"
#include "crcpacket.hpp"
#include "player_input.hpp"
#include "managers_context.hpp"
#include "json_parser.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// ABILITIES ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class RechargeType {
    None,

    //recharged by cooldown
    Cooldown,

    //never recharged nor casted
    Passive,

    //recharged by doing thing (dealing damage, gettings kills, objectives)
    Points
};

RechargeType rechargeTypeFromStr(const std::string& str);

enum AbilityType {
    ABILITY_NONE,

    #define LoadAbility(ability_name, json_id) \
        ABILITY_##ability_name,
    #include "abilities.inc"
    #undef LoadAbility

    ABILITY_MAX_TYPES
};

struct _AbilityCooldown
{
    u8 maxCharges;
    u8 currentCharges;
    float cooldown;
    float currentCooldown;

    //time that has to pass to use the next charge available
    float chargeRate;
};

struct _AbilityPoints
{
    float pointsMultiplier;
    float pointPercentage;
};

struct _AbilityPassive
{
    u16 passiveBuff;
};

struct Ability
{
    u8 type;
    RechargeType rechargeType;

    //we use one or the other depending on rechargeType
    union {
        _AbilityCooldown abCooldown;
        _AbilityPoints abPoints;
        _AbilityPassive abPassive;
    };  
};

extern Ability g_abilities[ABILITY_MAX_TYPES];

void loadAbilitiesFromJson(JsonParser* jsonParser);

void Ability_update(Ability& ability, sf::Time eTime);
void Ability_onCallback(Ability& ability);
bool Ability_canBeCasted(const Ability& ability);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// WEAPONS //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// typedef void (*WeaponCallback)(void);

struct WeaponData {
    u8 primaryFire;
    u8 secondaryFire;
    float scale;
    float angleOffset;
    u16 textureId;
};

enum WeaponType {
    WEAPON_NONE,

    #define LoadWeapon(weapon_name, callback_func, json_id, primary_fire_id, secondary_fire_id) \
        WEAPON_##weapon_name,
    #include "weapons.inc"
    #undef LoadWeapon

    WEAPON_MAX_TYPES
};

extern WeaponData g_weaponData[WEAPON_MAX_TYPES];

void loadWeaponsFromJson(JsonParser* jsonParser);

void WeaponCallback_devilsBow();

/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// UNITS ////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

enum UnitType {
    #define LoadUnit(unit_name, texture_id, json_id, weapon_id, alt_ability_id, ultimate_id) \
        UNIT_##unit_name,
    #include "units.inc"
    #undef LoadUnit

    UNIT_MAX_TYPES
};

#define MAX_UNITS 500

//times are packed as booleans (true if time > 0)
struct UnitStatus
{
    sf::Time stunTime;
    sf::Time rootTime;
    sf::Time disarmTime;
    sf::Time invisTime;

    bool solid = true;
    bool illusion = false;
    bool inBush = false;
};

struct C_UnitStatus
{
    bool stunned = false;
    bool rooted = false;
    bool disarmed = false;
    bool invisible = false;
    bool locallyHidden = false;
    bool forceSent = false;

    bool solid = true;
    bool illusion = false;
    bool inBush = false;
};

//neded for delta encoding
bool UnitStatus_equal(const UnitStatus& lhs, const UnitStatus& rhs);

void UnitStatus_packData(const UnitStatus& status, u8 teamId, CRCPacket& packet);
void C_UnitStatus_loadFromData(C_UnitStatus& status, CRCPacket& packet);

//@TODO
//Units and projectiles are too big to fit 2 in a single cache line anyway
//So maybe it's better to just stick with OOP and new/delete operations?

struct _BaseUnitData
{
    u32 uniqueId;
    Vector2 pos;
    u8 teamId;
    u8 type;
    u8 flyingHeight;
    u16 maxHealth;
    u16 health;
    u16 movementSpeed;
    float aimAngle;
    u8 weaponId;
    u8 collisionRadius;
    u8 trueSightRadius;

    Ability primaryFire;
    Ability secondaryFire;
    Ability altAbility;
    Ability ultimate;
};

struct Unit : _BaseUnitData
{
    UnitStatus status;
    Vector2 vel;
    bool dead;

    //stores if the unit is visible for each team (max 64 teams)
    u64 visionFlags;

    //all units close enough to a team will be forced to be sent
    //(even if they're hidden for that team)
    //this is to make reveals smooth in client
    u64 teamSentFlags;
};

struct C_Unit : _BaseUnitData
{
    C_UnitStatus status;
    u16 textureId;
    float scale;
};

//these are loaded at the start with json data
//initializing is just copying from here
extern Unit g_initialUnitData[UNIT_MAX_TYPES];
extern C_Unit g_initialCUnitData[UNIT_MAX_TYPES];

void loadUnitsFromJson(JsonParser* jsonParser);
void C_loadUnitsFromJson(JsonParser* jsonParser);

void Unit_init(Unit& unit, UnitType type);
void C_Unit_init(C_Unit& unit, UnitType type);

void Unit_packData(const Unit& unit, const Unit* prevUnit, u8 teamId, CRCPacket& outPacket);
void C_Unit_loadFromData(C_Unit& unit, CRCPacket& inPacket);

void Unit_moveColliding(Unit& unit, Vector2 newPos, const ManagersContext& context, bool force = false);

void Unit_update(Unit& unit, sf::Time eTime, const ManagersContext& context);
void Unit_preUpdate(Unit& unit, sf::Time eTime, const ManagersContext& context);

//revals the unit for team specified
void Unit_revealUnit(Unit& unit, u8 teamId);

//tells the server that the unit has to be sent
//to that specific team (even if hidden)
void Unit_markToSend(Unit& unit, u8 teamId);

bool Unit_isInvisible(const Unit& unit);
bool C_Unit_isInvisible(const C_Unit& unit);

bool Unit_isRevealedForTeam(const Unit& unit, u8 teamId);
bool Unit_isVisibleForTeam(const Unit& unit, u8 teamId);
bool Unit_isMarkedToSendForTeam(const Unit& unit, u8 teamId);

bool Unit_shouldBeHiddenFrom(const Unit& unit, const Unit& otherUnit);
bool C_Unit_shouldBeHiddenFrom(const C_Unit& unit, const C_Unit& otherUnit);

void C_Unit_interpolate(C_Unit& unit, const C_ManagersContext& context, const C_Unit* prevUnit, const C_Unit* nextUnit, double t, double d);

void Unit_applyInput(Unit& unit, const PlayerInput& input, const ManagersContext& context, u16 clientDelay);

//unit is not modified, only unitPos (since we want to interpolate)
void C_Unit_applyInput(const C_Unit& unit, Vector2& unitPos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt);
void C_Unit_applyAbilitiesInput(const C_Unit& unit, const PlayerInput& input, const C_ManagersContext& context);
