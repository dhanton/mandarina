#pragma once

#include "ability.hpp"

class HellsDashAbility : public CooldownAbility
{
public:
    HellsDashAbility* clone() const;

    bool canBeCasted(const Status& status) const;

    void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
    void C_onCast(C_Unit* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating);

    void loadFromJson(const rapidjson::Document& doc);

private:
    Vector2 dashColliding_impl(float aimAngle, const Vector2& casterPos, float collisionRadius, TileMap* map);

private:
    float m_dashRange;
};
