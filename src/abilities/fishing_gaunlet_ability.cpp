#include "abilities/fishing_gaunlet_ability.hpp"

#include "unit.hpp"

FishingGaunletAbility* FishingGaunletAbility::clone() const
{
	return new FishingGaunletAbility(*this);
}

void FishingGaunletAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
	CooldownAbility::onCastUpdate();
}

void FishingGaunletAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
	if (repeating) return;

	CooldownAbility::onCastUpdate();
}

void FishingGaunletAbility::loadFromJson(const rapidjson::Document& doc)
{
	CooldownAbility::loadFromJson(doc);
}
