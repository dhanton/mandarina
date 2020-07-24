#pragma once

#include "buff.hpp"

class PhasedBuff : public Buff
{
public:
    PhasedBuff* clone() const;

    void onPreUpdate(sf::Time eTime);
};
