#pragma once

#include <SFML/System/Time.hpp>
#include "defines.hpp"
#include "crcpacket.hpp"
#include "managers_context.hpp"
#include "context.hpp"
#include "json_parser.hpp"

//???
//@TODO: Projectiles should be encapsulated in a more general class
//TouchEntity are lightweight objects that are created and destroyed constantly
//like projectiles or pickup items
//This type of elements all use the same data and as such can be put into the same Bucket

class Entity;
class Unit;

enum ProjectileType {
    #define DoProjectile(projectile_name, json_id) \
        PROJECTILE_##projectile_name,
    #include "projectiles.inc"
    #undef DoProjectile
    
    PROJECTILE_MAX_TYPES
};

//projectiles are collisionObject, movement pattern, speed, rendering
//damage is provided by their respective units
//other things can be provided by their units as well (overwriting the defaults)
#define MAX_PROJECTILES 2000

namespace HitFlags 
{
    enum _HitFlags{
        None    = 0b00,
        Allies  = 0b01,
        Enemies = 0b10,
        Both    = 0b11
    };
}

struct _BaseProjectileData 
{
    u32 uniqueId;
    Vector2 pos;
    Vector2 vel;

    u8 teamId;
    u8 type;
    bool dead;

    u8 collisionRadius;
    u8 hitFlags;
    u16 movementSpeed;

    //some projectiles can destroy blocks and bushes
    bool destroysTiles;

    //distance before the projectile dies automatically
    u16 range;

    float rotation;
};

struct Projectile : _BaseProjectileData 
{
    float distanceTraveled;
    u32 shooterUniqueId;

    u8 buffAppliedType;
    u16 damage;

    //in milliseconds
    u16 revealTime;
    bool shouldReveal;

    //for projectiles that don't die when they hit something
    //maybe it's better to have it as a pointer
    // std::unordered_set<u32> hitUnits; (something like this)
    //(they should be entities)
};

struct C_Projectile : _BaseProjectileData 
{
    float scale;
    u16 textureId;

    //if the projectile was locally created
    //this is the input id that was used
    u32 createdInputId;

    //to render each projectile properly
    float rotationOffset;
    bool renderRotation;
};

extern Projectile g_initialProjectileData[PROJECTILE_MAX_TYPES];
extern C_Projectile g_initialCProjectileData[PROJECTILE_MAX_TYPES];

void loadProjectilesFromJson(JsonParser* jsonParser);
void C_loadProjectilesFromJson(JsonParser* jsonParser);

u8 Projectile_stringToType(const std::string& typeStr);

void Projectile_init(Projectile& projectile, u8 type, const Vector2& pos, float aimAngle);
void C_Projectile_init(C_Projectile& projectile, u8 type);

//Used to locally predict projectiles when player fires
void C_Projectile_init(C_Projectile& projectile, u8 type, const Vector2& pos, float aimAngle);

void Projectile_packData(const Projectile& projectile, const Projectile* prevProj, u8 teamId, CRCPacket& outPacket, const EntityManager* entityManager);
void C_Projectile_loadFromData(C_Projectile& projectile, CRCPacket& inPacket);

void Projectile_update(Projectile& projectile, sf::Time eTime, const ManagersContext& context);

//clientDelay in ms
void Projectile_backtrackCollisions(Projectile& projectile, const ManagersContext& context, u16 clientDelay);

//simply update the position of the projectile using its velocity
void C_Projectile_localUpdate(C_Projectile& projectile, sf::Time eTime, const C_ManagersContext& context);

//check collisions with units locally and remove the projectile if collision happens
void C_Projectile_checkCollisions(C_Projectile& projectile, const C_ManagersContext& context);

void Projectile_onHit(Projectile& projectile, Entity* unitHit, const ManagersContext& context);

//hitting a wall/unit or 
void Projectile_onDeath(Projectile& projectile);

//basic position interpolation
void C_Projectile_interpolate(C_Projectile& projectile, const C_Projectile* prevProj, const C_Projectile* nextProj, double t, double d);

void C_Projectile_insertRenderNode(const C_Projectile& projectile, const C_ManagersContext& managersContext, const Context& context);
