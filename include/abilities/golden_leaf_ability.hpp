#pragma once

#include "abilities/single_shot_ability.hpp"

class GoldenLeafAbility : public CooldownAbility
{
public:
    virtual GoldenLeafAbility* clone() const;

    virtual void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
    virtual void C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating);

    virtual void loadFromJson(const rapidjson::Document& doc);

private:
    float m_damageMultiplier;
};
