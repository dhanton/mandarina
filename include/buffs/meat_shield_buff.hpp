#pragma once

#include "buff.hpp"

class MeatShieldBuff : public Buff
{
public:
    MeatShieldBuff* clone() const;

    void onPreUpdate(sf::Time eTime);
    void onTakeDamage(u16& damage, Entity* source, u32 uniqueId, u8 teamId);
};
