#pragma once

#include "ability.hpp"

class NaturesRageAbility : public RechargeAbility
{
public:
    NaturesRageAbility* clone() const;

    void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
    void C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating);

    void loadFromJson(const rapidjson::Document& doc);

private:
    u16 m_rockNumber;
    u16 m_spreadDistance;

    float m_anglePerRock;
};
