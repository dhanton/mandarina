#include "buffs/phased_buff.hpp"

#include "unit.hpp"

PhasedBuff* PhasedBuff::clone() const
{
    return new PhasedBuff(*this);
}

void PhasedBuff::onPreUpdate(sf::Time eTime)
{
    m_unit->setSolid(false);
}
