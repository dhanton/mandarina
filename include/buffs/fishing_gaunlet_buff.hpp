#pragma once

#include "buff.hpp"

class FishingGaunletBuff : public Buff
{
public:
    FishingGaunletBuff* clone() const;

    void onProjectileHit(Projectile& projectile, Entity* target);
};
