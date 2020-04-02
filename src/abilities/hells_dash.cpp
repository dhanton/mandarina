#include "abilities/hells_dash.hpp"

#include <cmath>
#include "unit.hpp"
#include "tilemap.hpp"

HellsDashAbility* HellsDashAbility::clone() const
{
    return new HellsDashAbility(*this);
}

bool HellsDashAbility::canBeCasted(const Status& status) const
{
    return !status.rooted && CooldownAbility::canBeCasted(status);
}

void HellsDashAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    CooldownAbility::onCastUpdate();

    caster->setPosition(dashColliding_impl(caster->getAimAngle(), caster->getPosition(), caster->getCollisionRadius(), context.tileMap));
}

void HellsDashAbility::C_onCast(C_Unit* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
    casterPos = dashColliding_impl(caster->getAimAngle(), casterPos, caster->getCollisionRadius(), context.tileMap);

    if (!repeating) {
        CooldownAbility::onCastUpdate();
    }
}

void HellsDashAbility::loadFromJson(const rapidjson::Document& doc)
{
    CooldownAbility::loadFromJson(doc);

    m_dashRange = doc["dash_range"].GetInt();
}

Vector2 HellsDashAbility::dashColliding_impl(float aimAngle, const Vector2& casterPos, float collisionRadius, TileMap* map)
{
    aimAngle *= PI/180.f;
    Vector2 targetPos = casterPos + m_dashRange * Vector2(std::sin(aimAngle), std::cos(aimAngle));
    Circlef collisionCircle(targetPos, collisionRadius);
    
    //if the caster would be colliding if the dash was completed we find the closest point to target where this doesn't happen
    if (map->isColliding(TILE_BLOCK | TILE_WALL, collisionCircle) || map->isOutsideMap(collisionCircle)) {
        const Vector2 direction = Helper_vec2unitary(casterPos - targetPos);
        const int steps = m_dashRange / (collisionRadius * 2.f);
        Vector2 oldPos = collisionCircle.center;

        int i = 0;
        while (i < steps) {
            collisionCircle.center += (collisionRadius * 2.f) * direction;

            //find the closest tile that doesn't cause a collision
            if (!map->isColliding(TILE_BLOCK | TILE_WALL, collisionCircle) && !map->isOutsideMap(collisionCircle)) {
                targetPos = _UnitBase::moveCollidingTilemap_impl(collisionCircle.center, oldPos, collisionRadius, map);
                break;
            }

            oldPos = collisionCircle.center;
            i++;
        }

        //don't move the caster if there was no break
        if (i == steps) {
            targetPos = casterPos;
        }
    }

    return targetPos;
}