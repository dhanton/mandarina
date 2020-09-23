#pragma once

#include "ability.hpp"

class MeatShieldAbility : public RechargeAbility
{
public:
	MeatShieldAbility* clone() const;

	void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
    void C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating);

    void loadFromJson(const rapidjson::Document& doc);
};
