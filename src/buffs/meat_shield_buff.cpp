#include "buffs/meat_shield_buff.hpp"

#include "unit.hpp"
#include "status.hpp"

MeatShieldBuff* MeatShieldBuff::clone() const
{
    return new MeatShieldBuff(*this);
}

void MeatShieldBuff::onPreUpdate(sf::Time eTime)
{
    m_unit->getStatus()[STATUS_MEAT_SHIELD] = true;
}

void MeatShieldBuff::onEntityKill(Entity* target)
{
    m_unit->getSecondaryFire()->refresh();
}
