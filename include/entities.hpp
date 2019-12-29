#pragma once

#include <SFML/System/Time.hpp>

#include "defines.hpp"
#include "crcpacket.hpp"
#include "texture_ids.hpp"

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

    //Same as a unit, but also has ability information
    HERO,

    //Example of types and sub-types (types have some data, sub-types have same data and functionality)
    // NONE,
    BULLET_CIRCLE,
    BULLET_PELLET,
    UNIT_DOG,
    UNIT_MELEE_MINION,
    HERO_MUSCLES,
    HERO_WATERTOAD,
    HERO_SPIRIT
};

//void createEntity(u16 type, u16 sub-type)
//pass sub-type to same type function

/*
    Each entity has 2 structures, one for client and one for server
    It needs functionality to pack the server data and load the client one
    It also needs funcionality to initialize both

    What differenciates entities is the type data that they have
    Each type of entity is stored in a separated bucket (so that every bucket is separated in
    same-sized chunks)
    
    When you need to create sub-types for a certain type you create more functions
    and do a switch inside the main one
    This is not super elegant but it's more memory efficient than having lambdas or virtual inheritance
*/


struct C_TestCharacter
{
    u32 uniqueId;
    Vector2 pos;
    u16 textureId;
    u16 flags;
    float scale;
    float rotation;
    u8 flyingHeight;
};

//Functions are not defined globally to make 
//structs as compact as possible in memory

struct TestCharacter
{
    u32 uniqueId;
    Vector2 pos;
    Vector2 vel;
    u16 flags;
    u8 flyingHeight;
};

void TestCharacter_init(TestCharacter& entity);
void C_TestCharacter_init(C_TestCharacter& entity);

void TestCharacter_packData(const TestCharacter& entity, const TestCharacter* prevEntity, CRCPacket& outPacket);
void C_TestCharacter_loadFromData(C_TestCharacter& entity, CRCPacket& inPacket);

void C_TestCharacter_interpolate(C_TestCharacter& entity, const C_TestCharacter* prev, const C_TestCharacter* next, double t, double d);

//copy all data that wasn't interpolated
void C_TestCharacter_copySnapshotData(C_TestCharacter& entity, const C_TestCharacter* other);

void TestCharacter_update(TestCharacter& entity, sf::Time eTime);
