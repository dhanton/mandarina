#include "buffs/silence_buff.hpp"

#include "unit.hpp"

SilenceBuff* SilenceBuff::clone() const
{
	return new SilenceBuff(*this);
}

void SilenceBuff::onPreUpdate(sf::Time eTime)
{
	m_unit->getStatus()[STATUS_SILENCED] = true;
}
