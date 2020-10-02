#pragma once

#include "buff.hpp"

class MeatShieldBuff : public Buff
{
public:
    MeatShieldBuff* clone() const;

    void onPreUpdate(sf::Time eTime);
    void onEntityKill(Entity* target);
};
