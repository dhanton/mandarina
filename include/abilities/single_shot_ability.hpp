#pragma once

#include "ability.hpp"

class SingleShotAbility : public CooldownAbility
{
public:
    virtual SingleShotAbility* clone() const;

    virtual void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
    virtual void C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating);

    virtual void loadFromJson(const rapidjson::Document& doc);
private:
    u8 m_projectileType;
};
