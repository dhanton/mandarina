#include "buffs/root_buff.hpp"

#include "unit.hpp"

RootBuff* RootBuff::clone() const
{
    return new RootBuff(*this);
}

void RootBuff::onPreUpdate(sf::Time eTime)
{
    m_unit->getStatus()[STATUS_ROOTED] = true;
}
