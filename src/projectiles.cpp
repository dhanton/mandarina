#include "projectiles.hpp"

#include <SFML/System/Time.hpp>
#include "json_parser.hpp"
#include "collision_manager.hpp"
#include "quadtree.hpp"
#include "server_entity_manager.hpp"
#include "client_entity_manager.hpp"
#include "tilemap.hpp"
#include "bit_stream.hpp"
#include "unit.hpp"
#include "texture_ids.hpp"

Projectile g_initialProjectileData[PROJECTILE_MAX_TYPES];
C_Projectile g_initialCProjectileData[PROJECTILE_MAX_TYPES];

bool _BaseProjectile_loadFromJson(JsonParser* jsonParser, ProjectileType type, const char* filename, _BaseProjectileData& projectile)
{
    auto* doc = jsonParser->getDocument(filename);

    if (doc == nullptr) {
        std::cout << "_BaseProjectile_loadFromJson error - Invalid json filename " << filename << std::endl;
        return false;
    }

    projectile.type = type;

    projectile.dead = false;
    projectile.collisionRadius = (*doc)["collision_radius"].GetInt();
    projectile.movementSpeed = (*doc)["movement_speed"].GetInt();
    projectile.range = (*doc)["range"].GetInt();

    if (doc->HasMember("destroys_tiles")) {
        projectile.destroysTiles = (*doc)["destroys_tiles"].GetBool();
    } else {
        projectile.destroysTiles = false;
    }

    std::string hitFlags_str = (*doc)["hit_flags"].GetString();

    if (hitFlags_str == "BOTH") {
        projectile.hitFlags = HitFlags::Both;
    } else if (hitFlags_str == "ALLIES") {
        projectile.hitFlags = HitFlags::Allies;
    } else if (hitFlags_str == "ENEMIES") {
        projectile.hitFlags = HitFlags::Enemies;
    } else {
        projectile.hitFlags = 0;
    }

    return true;
}

void _BaseProjectile_angleInit(_BaseProjectileData& projectile, const Vector2& pos, float aimAngle)
{
    aimAngle *= PI/180.f;
    projectile.vel = Vector2(std::sin(aimAngle), std::cos(aimAngle)) * (float) projectile.movementSpeed;
}

bool _BaseProjectile_canHitTeam(const _BaseProjectileData& projectile, u8 teamId)
{
    return (teamId == projectile.teamId && (projectile.hitFlags & HitFlags::Allies) != 0) ||
           (teamId != projectile.teamId && (projectile.hitFlags & HitFlags::Enemies) != 0);
}

void _Projectile_loadFromJson(JsonParser* jsonParser, ProjectileType type, const char* filename, Projectile& projectile)
{
    bool result = _BaseProjectile_loadFromJson(jsonParser, type, filename, projectile);

    if (result) {
        projectile.distanceTraveled = 0.f;
    }
}

void _C_Projectile_loadFromJson(JsonParser* jsonParser, ProjectileType type, const char* filename, C_Projectile& projectile, u16 textureId)
{
    bool result = _BaseProjectile_loadFromJson(jsonParser, type, filename, projectile);

    if (result) {
        projectile.textureId = textureId;

        auto* doc = jsonParser->getDocument(filename);

        if (doc->HasMember("scale")) {
            projectile.scale = (*doc)["scale"].GetFloat();
        } else {
            projectile.scale = 1.f;
        }
    }
}

void loadProjectilesFromJson(JsonParser* jsonParser)
{
    #define LoadProjectile(projectile_name, texture_id, json_filename) \
        _Projectile_loadFromJson(jsonParser, PROJECTILE_##projectile_name, json_filename, g_initialProjectileData[PROJECTILE_##projectile_name]);
    
    #include "projectiles.inc"
    #undef LoadProjectile
}

void C_loadProjectilesFromJson(JsonParser* jsonParser)
{
    #define LoadProjectile(projectile_name, texture_id, json_filename) \
        _C_Projectile_loadFromJson(jsonParser, PROJECTILE_##projectile_name, json_filename, g_initialCProjectileData[PROJECTILE_##projectile_name], TextureId::texture_id);
    
    #include "projectiles.inc"
    #undef LoadProjectile
}

void Projectile_init(Projectile& projectile, u8 type, const Vector2& pos, float aimAngle)
{
    if (type < 0 || type >= PROJECTILE_MAX_TYPES) {
        return;
    }

    projectile = g_initialProjectileData[type];

    projectile.pos = pos;
    _BaseProjectile_angleInit(projectile, pos, aimAngle);
}

void C_Projectile_init(C_Projectile& projectile, u8 type)
{
    if (type < 0 || type >= PROJECTILE_MAX_TYPES) {
        return;
    }

    projectile = g_initialCProjectileData[type];
}

void C_Projectile_init(C_Projectile& projectile, u8 type, const Vector2& pos, float aimAngle)
{
    if (type < 0 || type >= PROJECTILE_MAX_TYPES) {
        return;
    }

    projectile = g_initialCProjectileData[type];

    projectile.pos = pos;
    _BaseProjectile_angleInit(projectile, pos, aimAngle);
}

void Projectile_packData(const Projectile& projectile, const Projectile* prevProj, u8 teamId, CRCPacket& outPacket, const EntityManager* entityManager)
{
    BitStream bits;

    bool posXChanged = !prevProj || projectile.pos.x != prevProj->pos.x;
    bits.pushBit(posXChanged);

    bool posYChanged = !prevProj || projectile.pos.y != prevProj->pos.y;
    bits.pushBit(posYChanged);

    bool collisionRadiusChanged = !prevProj || projectile.collisionRadius != prevProj->collisionRadius;
    bits.pushBit(collisionRadiusChanged);

    bool shooterUniqueIdChanged = !prevProj || projectile.shooterUniqueId != prevProj->shooterUniqueId;
    bits.pushBit(shooterUniqueIdChanged);

    //@BRANCH_WIP: If the shooter is not a unit this crashes the game
    //Should we use a dynamic_cast instead?
    const Unit* shooter = (Unit*) entityManager->entities.atUniqueId(projectile.shooterUniqueId);
    
    //Send position of shooter only for the first packet
    // bool sendShooterPos = !prevProj && shooter && !Unit_isVisibleForTeam(*shooter, teamId);
    //should we use isVisibleForTeam instead?
    bool sendShooterPos = !prevProj && shooter && !shooter->shouldSendToTeam(teamId);
    bits.pushBit(sendShooterPos);

    u8 byte = bits.popByte();
    outPacket << byte;

    if (posXChanged) {
        outPacket << projectile.pos.x;
    }

    if (posYChanged) {
        outPacket << projectile.pos.y;
    }

    if (collisionRadiusChanged) {
        outPacket << projectile.collisionRadius;
    }
    
    if (sendShooterPos) {
        outPacket << projectile.lastShooterPos.x << projectile.lastShooterPos.y;
    } 
    
    if (shooterUniqueIdChanged) {
        outPacket << projectile.shooterUniqueId;
    }
}

void C_Projectile_loadFromData(C_Projectile& projectile, CRCPacket& inPacket)
{
    BitStream bits;

    u8 byte;
    inPacket >> byte;
    bits.pushByte(byte);

    bool posXChanged = bits.popBit();
    bool posYChanged = bits.popBit();
    bool collisionRadiusChanged = bits.popBit();
    bool shooterUniqueIdChanged = bits.popBit();
    bool sendShooterPos = bits.popBit();

    if (posXChanged) {
        inPacket >> projectile.pos.x;
    }

    if (posYChanged) {
        inPacket >> projectile.pos.y;
    }

    if (collisionRadiusChanged) {
        inPacket >> projectile.collisionRadius;
    }
    
    if (sendShooterPos) {
        inPacket >> projectile.lastShooterPos.x >> projectile.lastShooterPos.y;
    } 
    
    if (shooterUniqueIdChanged) {
        inPacket >> projectile.shooterUniqueId;
    }
}

void Projectile_update(Projectile& projectile, sf::Time eTime, const ManagersContext& context)
{
    if (projectile.dead) return;

    //0 range means the projectile can travel infinitely
    if (projectile.range != 0 && projectile.distanceTraveled > projectile.range) {
        projectile.dead = true;
        return;
    }

    //@BRANCH_WIP: Should we use dynamic_cast instead?
    Unit* shooter = (Unit*) context.entityManager->entities.atUniqueId(projectile.shooterUniqueId);

    if (shooter) {
        projectile.lastShooterPos = shooter->getPosition();
    }

    Vector2 moveVec = projectile.vel * eTime.asSeconds();
    projectile.pos += moveVec;
    projectile.distanceTraveled += Helper_vec2length(moveVec);

    const Circlef circle(projectile.pos, projectile.collisionRadius);

    u16 collidingTile = context.tileMap->getCollidingTile(circle);

    if (projectile.destroysTiles && (collidingTile & (TILE_BLOCK | TILE_BUSH)) != 0) {
        //@WIP: Destroy those tiles
        //(send it to the client or predict it??)
    }

    if ((collidingTile & (TILE_BLOCK | TILE_WALL)) != 0) {
        //we don't return here since we want to check collision with units as well
        projectile.dead = true;

        //@TODO: Some projectiles might not get destroyed when they hit a tile
        //They might bounce, or pass through
        //create function onTileHit(collidingTile) and add it to the _funcTable[PROJ_TYPE]
    }

    auto query = context.collisionManager->getQuadtree()->QueryIntersectsRegion(BoundingBody<float>(circle));

    //Because of the implementation, queries can't be stopped unless EndOfQuery is true
    bool queryDone = false;

    while (!query.EndOfQuery()) {
        if (queryDone) {
            query.Next();
            continue;
        }

        u32 collisionUniqueId = query.GetCurrent()->uniqueId;
        Unit* unit = (Unit*) context.entityManager->entities.atUniqueId(collisionUniqueId);

        if (unit) {
            if (_BaseProjectile_canHitTeam(projectile, unit->getTeamId())) {

                Projectile_onHit(projectile, unit);
                projectile.dead = true;
                queryDone = true;
            }
        }

        query.Next();
    }
}

void C_Projectile_localUpdate(C_Projectile& projectile, sf::Time eTime, const C_ManagersContext& context)
{
    projectile.pos += projectile.vel * eTime.asSeconds();
}

void C_Projectile_checkCollisions(C_Projectile& projectile, const C_ManagersContext& context)
{
    const Circlef circle(projectile.pos, projectile.collisionRadius);
    Circlef unitCircle;

    u16 collidingTile = context.tileMap->getCollidingTile(circle);
    if ((collidingTile & (TILE_BLOCK | TILE_WALL)) != 0) {
        projectile.dead = true;
    }

    for (auto it = context.entityManager->entities.begin(); it != context.entityManager->entities.end(); ++it) {
        unitCircle.center = it->getPosition();
        unitCircle.radius = (float) it->getCollisionRadius();

        if (_BaseProjectile_canHitTeam(projectile, it->getTeamId()) && circle.intersects(unitCircle)) {
            projectile.dead = true;

            //We could also reveal the entity if it's locally hidden

            break;
        }
    }

    // for (int i = 0; i < context.entityManager->units.firstInvalidIndex(); ++i) {
    //     const C_Unit& unit = context.entityManager->units[i];
    //     unitCircle.center = unit.pos;
    //     unitCircle.radius = (float) unit.collisionRadius;

    //     if (_BaseProjectile_canHitTeam(projectile, unit.teamId) && circle.intersects(unitCircle)) {
    //         projectile.dead = true;

    //         //we can also reveal the unit locally if its locally hidden

    //         break;
    //     }
    // }
}

void Projectile_onHit(Projectile& projectile, Unit* unitHit)
{
    //if the unit shooter unit is dead
    //then we need the damage stored somewhere else

    //@WIP: Most projectiles apply a certain force on hit
    //(implement friction as well)

    //the hook projectile does something as well

    //@WIP: Apply reveal on hit for ~2 seconds
    std::cout << "Hit some unit!" << std::endl;
}

void C_Projectile_interpolate(C_Projectile& projectile, const C_Projectile* prevProj, const C_Projectile* nextProj, double t, double d)
{
    if (prevProj && nextProj) {
        projectile.pos = Helper_lerpVec2(prevProj->pos, nextProj->pos, t, d);
    }
}