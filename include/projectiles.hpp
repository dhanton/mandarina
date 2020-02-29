#pragma once

#include <SFML/System/Time.hpp>
#include "defines.hpp"
#include "crcpacket.hpp"
#include "managers_context.hpp"
#include "json_parser.hpp"

//???
//@TODO: Projectiles should be encapsulated in a more general class that includes
//all entities that have no health/abilities/buffs

enum ProjectileType {
    #define LoadProjectile(projectile_name, texture_id, json_id) \
        PROJECTILE_##projectile_name,
    #include "projectiles.inc"
    #undef LoadProjectile
    
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

    u32 shooterUniqueId;
    
    //sent if the hit enemy doesn't have vision of the shooter or if it died
    Vector2 lastShooterPos;
};

struct Projectile : _BaseProjectileData 
{
    float distanceTraveled;

    //for projectiles that don't die when they hit something
    //maybe it's better to have it as a pointer
    // std::unordered_set<u32> hitUnits; (something like this)

    //onHit callback
};

struct C_Projectile : _BaseProjectileData 
{
    float scale;
    u16 textureId;

    //if the projectile was locally created
    //this is the input id that was used
    u32 createdInputId;
};

extern Projectile g_initialProjectileData[PROJECTILE_MAX_TYPES];
extern C_Projectile g_initialCProjectileData[PROJECTILE_MAX_TYPES];

void loadProjectilesFromJson(JsonParser* jsonParser);
void C_loadProjectilesFromJson(JsonParser* jsonParser);

struct Unit;

void Projectile_init(Projectile& projectile, u8 type, const Vector2& pos, float aimAngle);
void C_Projectile_init(C_Projectile& projectile, u8 type);

//Used to locally predict projectiles when player fires
void C_Projectile_init(C_Projectile& projectile, u8 type, const Vector2& pos, float aimAngle);

//@WIP: Using delta encoding is probably worse for projectiles
//since they're always moving and the don't carry any other info
//(maybe we have to send the position of the one who shot it??, or his uniqueId??)
void Projectile_packData(const Projectile& projectile, const Projectile* prevProj, u8 teamId, CRCPacket& outPacket, const EntityManager* entityManager);
void C_Projectile_loadFromData(C_Projectile& projectile, CRCPacket& inPacket);

//check if it's hitting someone (using hitsAllies and hitsEnemies)
//call onHit and set dead = true
void Projectile_update(Projectile& projectile, sf::Time eTime, const ManagersContext& context);

//simply update the position of the projectile using its velocity
void C_Projectile_localUpdate(C_Projectile& projectile, sf::Time eTime, const C_ManagersContext& context);

//check collisions with units locally and remove the projectile if collision happens
void C_Projectile_checkCollisions(C_Projectile& projectile, const C_ManagersContext& context);

//depending on the projectile type this function might do different things
//default (and most projectiles) just deal damage
//some heal, pull units to them, etc
//damage depends on the unit that shot it
void Projectile_onHit(Projectile& projectile, Unit* unitHit);
//Unit* shooter = context.entityManager.units.atUniqueId(projectile.shooterUniqueId);
//if (shooter)
//unitHit->takeDamage(shooter->damage)
//shooter->onDealDamage(*shooter, unitHit)

//hitting a wall/unit or 
void Projectile_onDeath(Projectile& projectile);

//basic position interpolation
void C_Projectile_interpolate(C_Projectile& projectile, const C_Projectile* prevProj, const C_Projectile* nextProj, double t, double d);

//@WIP: Projectiles can render custom effects (like the fishing hook that renders a chain to the shooter)
void Projectile_render(const Projectile& projectile);

