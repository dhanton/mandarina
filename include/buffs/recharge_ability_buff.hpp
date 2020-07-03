#pragma once

#include "buff.hpp"
#include "managers_context.hpp"

class RechargeAbility;

class RechargeAbilityBuff : public Buff
{
public:
    RechargeAbilityBuff* clone() const;

    void onDealDamage(u16 damage, Entity* target);
    void onEntityKill(Entity* target);
    //onHeal should give ~ same percentage as damage
    //onPlaceBuff should give less than damage

    void setCreator(void* creator, const ManagersContext& context);

private:
    float getMultiplier() const;

    RechargeAbility* m_ability;
    GameMode* m_gameMode;
};
