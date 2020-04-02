#include "buffs/reveal_buff.hpp"

#include "unit.hpp"

RevealBuff* RevealBuff::clone() const
{
    return new RevealBuff(*this);
}

void RevealBuff::revealForTeam(u8 teamId)
{
    m_revealForTeamId = teamId;
}

void RevealBuff::setDuration(float duration)
{
    m_duration = duration;
}

void RevealBuff::onPreUpdate(sf::Time eTime)
{
    m_unit->reveal(m_revealForTeamId);
}
