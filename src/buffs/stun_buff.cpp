#include "buffs/stun_buff.hpp"

#include "unit.hpp"

StunBuff* StunBuff::clone() const
{
    return new StunBuff(*this);
}

void StunBuff::onPreUpdate(sf::Time eTime)
{
    m_unit->getStatus().stunned = true;
}
