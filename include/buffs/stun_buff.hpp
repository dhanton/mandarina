#pragma once

#include "buff.hpp"

class StunBuff : public Buff
{
public:
    StunBuff* clone() const;

    void onPreUpdate(sf::Time eTime);
};
