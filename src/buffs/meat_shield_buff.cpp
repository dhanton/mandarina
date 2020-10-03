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

void MeatShieldBuff::onTakeDamage(u16& damage, Entity* source, u32 uniqueId, u8 teamId)
{
    m_unit->beHealed(damage, m_unit);
    damage = 0;
}
