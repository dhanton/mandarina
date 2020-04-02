#pragma once

#include "buff.hpp"

class RootBuff : public Buff
{
public:
    RootBuff* clone() const;

    void onPreUpdate(sf::Time eTime);
};
