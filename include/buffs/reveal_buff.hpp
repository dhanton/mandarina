#pragma once

#include "buff.hpp"

class RevealBuff : public Buff
{
public:
    RevealBuff* clone() const;

    void revealForTeam(u8 teamId);
    void setDuration(float duration);

    void onPreUpdate(sf::Time eTime);

private:
    u8 m_revealForTeamId;
};
