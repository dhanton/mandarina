#pragma once

#include "buff.hpp"

class SilenceBuff : public Buff
{
public:
    SilenceBuff* clone() const;

    void onPreUpdate(sf::Time eTime);
};
