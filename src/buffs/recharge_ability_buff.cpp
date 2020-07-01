#include "buffs/recharge_ability_buff.hpp"

#include "ability.hpp"
#include "game_mode.hpp"

RechargeAbilityBuff* RechargeAbilityBuff::clone() const
{
    return new RechargeAbilityBuff(*this);
}

void RechargeAbilityBuff::onDealDamage(u16 damage, Entity* target)
{
    if (m_ability) {
        //by default (multiplier=1) 200 damage = 10% charge
        m_ability->addToPercentage((static_cast<float>(damage)/2000.f) * getMultiplier());
    }
}

void RechargeAbilityBuff::onEntityKill(Entity* target)
{
    if (m_ability) {
        //kills are 30% charge
        m_ability->addToPercentage(0.3 * getMultiplier());
    }
}

void RechargeAbilityBuff::setCreator(void* creator, const ManagersContext& context)
{
    m_ability = static_cast<RechargeAbility*>(creator);
    m_gameMode = context.gameMode;
}

float RechargeAbilityBuff::getMultiplier() const
{
    if (m_ability) {
        float multiplier = 1.f;

        if (m_gameMode) {
            multiplier = m_gameMode->getRechargeMultiplier();
        }

        multiplier *= m_ability->getRechargeMultiplier();

    } else {
        return 0.f;
    }
}
