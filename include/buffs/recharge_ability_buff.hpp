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

    //@WIP: Add recharge values for other actions
    //kills should be approx 30% charge
    //heals should be like damage
    //placing buffs on enemies should be less than the rest
    
    void setCreator(void* creator, const ManagersContext& context);

private:
    float getMultiplier() const;

    RechargeAbility* m_ability;
    GameMode* m_gameMode;
};
