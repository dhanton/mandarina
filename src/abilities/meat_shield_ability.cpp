#include "abilities/meat_shield_ability.hpp"

MeatShieldAbility* MeatShieldAbility::clone() const
{
	return new MeatShieldAbility(*this);
}

void MeatShieldAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
	RechargeAbility::onCastUpdate();
}

void MeatShieldAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
	if (repeating) return;

	RechargeAbility::onCastUpdate();
}

void MeatShieldAbility::loadFromJson(const rapidjson::Document& doc)
{
	RechargeAbility::loadFromJson(doc);
}
