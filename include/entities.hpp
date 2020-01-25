#pragma once

#include <SFML/System/Time.hpp>
#include <unordered_map>

#include "defines.hpp"
#include "crcpacket.hpp"
#include "player_input.hpp"
#include "managers_context.hpp"

class JsonParser;

enum UnitType {
    UNIT_NONE,

    #define LoadUnit(unit_name, texture_id, json_filename, weapon_id) \
        UNIT_##unit_name,
    #include "units.inc"
    #undef LoadUnit

    UNIT_MAX_TYPES
};

typedef void (*WeaponCallback)(void);

struct WeaponData {
    WeaponCallback callback;
    float scale;
    float angleOffset;
    u16 textureId;
};

enum WeaponType {
    WEAPON_NONE,

    #define LoadWeapon(weapon_name, callback_func, json_filename) \
        WEAPON_##weapon_name,
    #include "weapons.inc"
    #undef LoadWeapon

    WEAPON_MAX_TYPES
};

extern WeaponData g_weaponData[WEAPON_MAX_TYPES];

void initializeWeaponData(JsonParser* jsonParser);

void WeaponCallback_devilsBow();

#define MAX_UNITS 500

enum ProjectileType {
    //@WIP: #include from projectiles.inc
    MAX_PROJECTILE_TYPES
};

//projectiles are collisionObject, movement pattern, speed, rendering
//damage is provided by their respective units
//other things can be provided by their units as well (overwriting the defaults)
#define MAX_PROJECTILES 2000

//counts are packed as booleans (true if time > 0)
struct UnitStatus
{
    sf::Time stunTime;
    sf::Time rootTime;
    sf::Time disarmTime;
    sf::Time invisTime;
    bool solid = true;
    bool illusion = false;
};

struct C_UnitStatus
{
    bool stunned = false;
    bool rooted = false;
    bool disarmed = false;
    bool invisible = false;
    bool locallyHidden = true;
    //@WIP
    bool forceRevealed = false;
    bool solid = true;
    bool illusion = false;
};

//neded for delta encoding
bool UnitStatus_equal(const UnitStatus& lhs, const UnitStatus& rhs);

void UnitStatus_packData(const UnitStatus& status, u8 teamId, CRCPacket& packet);
void C_UnitStatus_loadFromData(C_UnitStatus& status, CRCPacket& packet);

struct _BaseEntityData {
    u32 uniqueId;
    Vector2 pos;
    u8 teamId;
};

struct _BaseUnitData : _BaseEntityData
{
    u8 type;
    u8 flyingHeight;
    u16 maxHealth;
    u16 health;
    u16 movementSpeed;
    u8 maxAttacksAvailable;
    u8 attacksAvailable;
    float aimAngle;
    u8 weaponId;
    u8 collisionRadius;
    u8 trueSightRadius;
};

struct Unit : _BaseUnitData
{
    UnitStatus status;
    Vector2 vel;
    bool dead;

    //stores if the unit is visible for each team (max 64 teams)
    u64 visionFlags;

    //if the unit is forced to be revealed for a certain team
    u64 forcedVisionFlags;
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
void Unit_revealUnit(Unit& unit, u8 teamId, bool force);
bool Unit_isRevealedForTeam(const Unit& unit, u8 teamId);
bool Unit_isForcedRevealedForteam(const Unit& unit, u8 teamId);
bool Unit_isVisibleForTeam(const Unit& unit, u8 teamId);

void C_Unit_interpolate(C_Unit& unit, const C_Unit* prevUnit, const C_Unit* nextUnit, double t, double d, bool controlled);

void Unit_applyInput(Unit& unit, const PlayerInput& input, const ManagersContext& context);

//unit is not modified, only unitPos (since we want to interpolate)
void C_Unit_applyInput(const C_Unit& unit, Vector2& unitPos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt);
