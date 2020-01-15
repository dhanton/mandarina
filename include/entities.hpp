#pragma once

#include <SFML/System/Time.hpp>
#include <unordered_map>

#include "defines.hpp"
#include "crcpacket.hpp"
#include "player_input.hpp"

enum UnitType {
    UNIT_NONE,

    #define LoadUnit(unit_name, unit_texture_id, unit_json_file) \
        UNIT_##unit_name,
    #include "units.inc"
    #undef LoadUnit

    UNIT_MAX_TYPES
};

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
    bool solid = true;
    bool illusion = false;
};

//neded for delta encoding
bool UnitStatus_equal(const UnitStatus& lhs, const UnitStatus& rhs);

void UnitStatus_packData(const UnitStatus& status, CRCPacket& packet);
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
    //@TODO: Implement weapons
    u8 weaponType;
    u8 collisionRadius;
};

struct Unit : _BaseUnitData
{
    UnitStatus status;
    Vector2 vel;
    bool dead;
};

struct C_Unit : _BaseUnitData
{
    C_UnitStatus status;
    u16 textureId;
    float scale;
};

//these are loaded at the start with json data
//initializing is just copying from here
extern Unit INITIAL_UNIT_DATA[UNIT_MAX_TYPES];
extern C_Unit INITIAL_C_UNIT_DATA[UNIT_MAX_TYPES];

class JsonParser;

void loadUnitsFromJson(JsonParser* jsonParser);
void C_loadUnitsFromJson(JsonParser* jsonParser);

void Unit_init(Unit& unit, UnitType type);
void C_Unit_init(C_Unit& unit, UnitType type);

void Unit_packData(const Unit& unit, const Unit* prevUnit, CRCPacket& outPacket);
void C_Unit_loadFromData(C_Unit& unit, CRCPacket& inPacket);

//@WIP: Pass context where appropiate (entity manager and quadtree)
void Unit_update(Unit& unit, sf::Time eTime);

void C_Unit_interpolate(C_Unit& unit, const C_Unit* prevUnit, const C_Unit* nextUnit, double t, double d, bool controlled);

//@WIP: Pass context where appropiate (entity manager and quadtree)
void Unit_applyInput(Unit& unit, const PlayerInput& input);

//We may want to create this function to check for collisions in client
//Keep in mind that the client doesn't update the position when applying inputs
//(because it has to interpolate)
//Vector2 C_Unit_applyInput(const C_Unit& unit, PlayerInput& input, sf::Time dt);
