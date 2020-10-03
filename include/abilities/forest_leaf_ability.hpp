#pragma once

#include "single_shot_ability.hpp"

class ForestLeafAbility : public SingleShotAbility
{
public:
    ForestLeafAbility* clone() const;

    void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
};
