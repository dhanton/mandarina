#pragma once

#include "ability.hpp"
#include "abilities/single_shot_ability.hpp"

class FishingGaunletAbility : public SingleShotAbility 
{
public:
    virtual FishingGaunletAbility* clone() const;

    virtual void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);

    virtual void addBuffsToCaster(Unit* unit, const ManagersContext& context);
    virtual void loadFromJson(const rapidjson::Document& doc);

private:
    float m_healthRemoved;
    float m_healthToDamage;
};
