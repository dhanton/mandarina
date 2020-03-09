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
    //we want rotation to be in degrees
    projectile.rotation = aimAngle;

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

        if (doc->HasMember("rotation_offset")) {
            projectile.rotationOffset = (*doc)["rotation_offset"].GetFloat();
        } else {
            projectile.rotationOffset = 0.f;
        }

        if (doc->HasMember("render_rotation")) {
            projectile.renderRotation = (*doc)["render_rotation"].GetBool();
        } else {
            projectile.renderRotation = true;
        }
    }
}

void loadProjectilesFromJson(JsonParser* jsonParser)
{
    #define DoProjectile(projectile_name, json_filename) \
        _Projectile_loadFromJson(jsonParser, PROJECTILE_##projectile_name, json_filename, g_initialProjectileData[PROJECTILE_##projectile_name]);
    
    #include "projectiles.inc"
    #undef DoProjectile
}

void C_loadProjectilesFromJson(JsonParser* jsonParser)
{
    #define DoProjectile(projectile_name, json_filename) \
        _C_Projectile_loadFromJson(jsonParser, PROJECTILE_##projectile_name, json_filename, g_initialCProjectileData[PROJECTILE_##projectile_name], TextureId::projectile_name);
    
    #include "projectiles.inc"
    #undef DoProjectile
}

u8 Projectile_stringToType(const std::string& typeStr)
{
    //There is no PROJECTILE_NONE so we return invalid projectile
    if (typeStr == "NONE") return PROJECTILE_MAX_TYPES;

    #define DoProjectile(projectile_name, json_filename) \
        if (typeStr == #projectile_name) return PROJECTILE_##projectile_name;
    #include "projectiles.inc"
    #undef DoProjectile

    return PROJECTILE_MAX_TYPES;
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

    bool rotationChanged = !prevProj || projectile.rotation != prevProj->rotation;
    bits.pushBit(rotationChanged);

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
    
    if (rotationChanged) {
        outPacket << Helper_angleTo16bit(projectile.rotation);
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
    bool rotationChanged = bits.popBit();

    if (posXChanged) {
        inPacket >> projectile.pos.x;
    }

    if (posYChanged) {
        inPacket >> projectile.pos.y;
    }

    if (collisionRadiusChanged) {
        inPacket >> projectile.collisionRadius;
    }
    
    if (rotationChanged) {
        u16 angle16bit;
        inPacket >> angle16bit;
        projectile.rotation = Helper_angleFrom16bit(angle16bit);
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

void Projectile_backtrackCollisions(Projectile& projectile, const ManagersContext& context, u16 clientDelay)
{
    float totalDistance = ((float) clientDelay / 1000.f) * projectile.movementSpeed;

    //do no more than 5 iteration steps
    int steps = std::min((int) (totalDistance / projectile.collisionRadius), 5);
    sf::Time stepTime = sf::milliseconds(clientDelay/steps);
    int i = 0;

    while (i < steps && !projectile.dead) {
        Projectile_update(projectile, stepTime, context);
        i++;
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
        return;
    }

    for (auto it = context.entityManager->entities.begin(); it != context.entityManager->entities.end(); ++it) {
        unitCircle.center = it->getPosition();
        unitCircle.radius = (float) it->getCollisionRadius();

        if (_BaseProjectile_canHitTeam(projectile, it->getTeamId()) && circle.intersects(unitCircle)) {
            projectile.dead = true;

            //We could also reveal the entity if it's locally hidden
            //(probably not needed)

            break;
        }
    }
}

void Projectile_onHit(Projectile& projectile, Unit* unitHit)
{
    //if the unit shooter unit is dead
    //then we need the damage stored somewhere else

    //@WIP
    //Projectiles apply a certain force on hit => projectile.hitForce
    //They also reveal the unit for ~2 seconds => projectile.revealTime
    //They deal damage                         => projectile.damage
    //They apply a buff                        => projectile.buffAppliedType
    //(implement some sort of friction for units as well)

    std::cout << "Hit some unit!" << std::endl;
}

void C_Projectile_interpolate(C_Projectile& projectile, const C_Projectile* prevProj, const C_Projectile* nextProj, double t, double d)
{
    if (prevProj && nextProj) {
        projectile.pos = Helper_lerpVec2(prevProj->pos, nextProj->pos, t, d);
    }
}

void C_Projectile_insertRenderNode(const C_Projectile& projectile, const C_ManagersContext& managersContext, const Context& context)
{
    std::vector<RenderNode>& renderNodes = managersContext.entityManager->getRenderNodes();

    //@WIP: flyingHeight = shooter.height + shooter.flyingHeight
    renderNodes.emplace_back(RenderNode(100, projectile.uniqueId, (float) projectile.collisionRadius));

    sf::Sprite& sprite = renderNodes.back().sprite;

    sprite.setTexture(context.textures->getResource(projectile.textureId));
    sprite.setScale(projectile.scale, projectile.scale);
    sprite.setOrigin(sprite.getLocalBounds().width/2.f, sprite.getLocalBounds().height/2.f);

    if (projectile.renderRotation) {
        sprite.setRotation(-projectile.rotation-projectile.rotationOffset);
    }

    sprite.setPosition(projectile.pos);
}
