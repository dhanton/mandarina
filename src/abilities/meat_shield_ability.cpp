#include "abilities/meat_shield_ability.hpp"

#include "unit.hpp"
#include "buff.hpp"

MeatShieldAbility* MeatShieldAbility::clone() const
{
	return new MeatShieldAbility(*this);
}

void MeatShieldAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
	RechargeAbility::onCastUpdate();

	caster->beHealed(caster->getMaxHealth(), caster);
	caster->addBuff(BUFF_MEAT_SHIELD);
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
