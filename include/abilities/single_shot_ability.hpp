#pragma once

#include "ability.hpp"

class SingleShotAbility : public CooldownAbility
{
public:
    virtual SingleShotAbility* clone();

    virtual void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
    virtual void C_onCast(C_Unit* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId);

    virtual void loadFromJson(const rapidjson::Document& doc);
private:
    u8 m_projectileType;
};
