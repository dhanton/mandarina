#pragma once

#include <SFML/System/Time.hpp>
#include <unordered_map>

#include "defines.hpp"
#include "crcpacket.hpp"
#include "texture_ids.hpp"
#include "player_input.hpp"

enum class EntityType {
    NONE,
    TEST_CHARACTER,

    //Things that move constantly
    //they deal damage and do something on hit
    //no need to check if bullet position has changed - we assume it always does
    //that means traversing all bullets to send them is very cache-friendly (very local operation)
    BULLET,

    //Has a collision body, moves around at a certain speed
    //can fly, become stunned/silenced/rooted
    //become invisible
    //is affected by buffs and abilities
    UNIT,

    //Same as a unit, but also has abilities (and ultimate) and xp/upgrade system
    HERO,

    //Example of types and sub-types (types have some data, sub-types have same data and functionality)
    // NONE,
    BULLET_CIRCLE,
    BULLET_PELLET,
    UNIT_DOG,
    UNIT_MELEE_MINION,
    HERO_MUSCLES,
    HERO_WATERTOAD,
    HERO_SPIRIT,

    //number of different types of units
    MAX_UNIT_TYPES
};

enum UnitType {
    // #include from units.inc
    MAX_UNIT_TYPES
};

enum ProjectileType {
    //#include from projectiles.inc
    MAX_PROJECTILE_TYPES
};

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


//@WIP: Use only one type of UnitStatus for both client and server??
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

//data needed to pack/load (HOT DATA)
struct UnitNetData {
    u32 uniqueId;
    u8 type;
    u8 teamId;

    Vector2 pos;

    //doesn't need to be sent in areas
    u8 flyingHeight;

    u16 health;
    u16 maxHealth;

    u8 attacksAvailable;

    float aimAngle;

    u8 collisionRadius;

    UnitStatus status;
};

//similar array for all the possible abilities of each ability type
// extern (*void)(u32 uniqueId, ...)

//similar array for initializing projectiles types

//data not needed to pack/load (COLD DATA)
struct UnitOtherData {
    u16 movementSpeed;
    Vector2 vel;
    u8 maxAttacksAvailable;

    //abilites go here as function pointers (start nullptr)
    //the reference to each ability is a global table (for each ability type)
    //units point to that table when initialized
};

struct C_UnitOtherData {
    u16 textureId;
    float scale;
    u8 maxAttacksAvailable;
};

//load this with default data for each unit type
//initializing is just copying from that array
extern UnitNetData UnitInitialNetData[MAX_UNIT_TYPES];
extern UnitOtherData UnitInitialData[MAX_UNIT_TYPES];
extern C_UnitOtherData C_UnitInitialData[MAX_UNIT_TYPES];

#define MAX_UNITS 500

struct UnitSet {
    UnitNetData netData[MAX_UNITS];
    UnitOtherData otherData[MAX_UNITS];

    std::unordered_map<u32, int> uniqueIds;

    int firstInvalidIndex;

    //need functionality to obtain index based on uniqueId
    //also to keep both arrays concurrent
};

struct C_UnitSet {
    UnitNetData netData[MAX_UNITS];
    C_UnitOtherData otherData[MAX_UNITS];

    std::unordered_map<u32, int> uniqueIds;
};

//projectiles are collisionObject, movement pattern, speed, rendering
//damage is provided by their respective units
//other things can be provided by their units as well (overwriting the defaults)

#define MAX_PROJECTILES 2000

struct ProjectileNetData {
    Vector2 pos;

    //shooter may or may not be send
    //depending on the projectile type
    u32 shooterUniqueId;

    u8 type;
};

struct ProjectileOtherData {
    Vector2 vel;
};

struct C_ProjectileOtherData {
    u16 textureId;
};


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct _BaseEntityData {
    u32 uniqueId;
    Vector2 pos;
    u8 teamId;
};

struct _BaseUnitData : _BaseEntityData
{
    u8 flyingHeight;
    u16 health;
    u16 maxHealth;
    u16 movementSpeed;
    u8 attacksAvailable;
    u8 maxAttacksAvailable;
    float aimAngle;
    //@TODO: Implement weapons
    u8 weaponType;
    u8 collisionRadius;
};

struct Unit : _BaseUnitData
{
    UnitStatus status;
    Vector2 vel;
    bool dead = false;
};

struct C_Unit : _BaseUnitData
{
    C_UnitStatus status;
    u16 textureId;
    float scale;
};

//@WIP: Use json files to init values
//using the chosen sub-type
void Unit_init(Unit& unit);
void C_Unit_init(C_Unit& unit);

//@WIP: Use delta encoding (for all fields) using bitstream
void Unit_packData(const Unit& unit, CRCPacket& packet);
void C_Unit_loadFromData(C_Unit& unit, CRCPacket& packet);

//@WIP: Pass context where appropiate (entity manager and quadtree)

void Unit_update(Unit& unit, sf::Time eTime);

//@WIP: Copy the rest of the fields as well (status, flyingHeight, health, etc)
//All those fields might require interpolation as well (when animations/particles are implemented)
void C_Unit_interpolate(C_Unit& unit, const C_Unit* prevUnit, const C_Unit* nextUnit, double t, double d);

//@WIP: Check we can actually move here
void Unit_applyInput(Unit& unit, const PlayerInput& input);

//unit is not modified when applying input in client, because we have to interpolate
//@WIP: Check we can actually move
Vector2 C_Unit_applyInput(const C_Unit& unit, PlayerInput& input, sf::Time eTime);

struct C_TestCharacter
{
    u32 uniqueId;
    Vector2 pos;
    u16 textureId;
    u16 flags;
    float scale;
    float rotation;
    u8 flyingHeight;
    u8 teamId;
    u16 movementSpeed;
};

struct TestCharacter
{
    u32 uniqueId;
    Vector2 pos;
    Vector2 vel;
    u16 flags;
    u8 flyingHeight;
    u8 teamId;
    u16 movementSpeed;
};

void TestCharacter_init(TestCharacter& entity);
void C_TestCharacter_init(C_TestCharacter& entity);

void TestCharacter_packData(const TestCharacter& entity, const TestCharacter* prevEntity, CRCPacket& outPacket);
void C_TestCharacter_loadFromData(C_TestCharacter& entity, CRCPacket& inPacket);

void C_TestCharacter_interpolate(C_TestCharacter& entity, const C_TestCharacter* prev, const C_TestCharacter* next, double t, double d);

//copy all data that wasn't interpolated
//@WIP: I believe this is not used anywhere at the moment (_interpolate is used instead)
void C_TestCharacter_copySnapshotData(C_TestCharacter& entity, const C_TestCharacter* other);

void TestCharacter_update(TestCharacter& entity, sf::Time eTime);

//@TODO: Check against collision
void TestCharacter_applyInput(TestCharacter& entity, PlayerInput& input);
void C_TestCharacter_applyInput(C_TestCharacter& entity, PlayerInput& input, sf::Time dt);
