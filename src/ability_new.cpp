#include "ability_new.hpp"

#include "unit.hpp"

CooldownAbility::CooldownAbility()
{
    //@BRANCH_WIP
    //load these from json
    m_maxCharges = 0;
    m_currentCharges = 0;
    m_cooldown = 0;
    m_currentCooldown = 0;

    m_nextChargeTime = 0;
}

void CooldownAbility::onCast(Unit* caster, const ManagersContext& context)
{

}

void CooldownAbility::C_onCast(C_Unit* caster, const C_ManagersContext& context)
{

}

void CooldownAbility::update(sf::Time eTime)
{

}

bool CooldownAbility::canBeCasted()
{

}

RechargeAbility::RechargeAbility()
{

}

void RechargeAbility::onCast(Unit* caster, const ManagersContext& context)
{

}

void RechargeAbility::C_onCast(C_Unit* caster, const C_ManagersContext& context)
{

}

void RechargeAbility::update(sf::Time eTime)
{

}

bool RechargeAbility::canBeCasted()
{
    return m_percentage >= 1.f;
}
