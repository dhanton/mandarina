#include "abilities/self_buff_ability.hpp"

#include "unit.hpp"

SelfBuffAbility* SelfBuffAbility::clone() const
{
    return new SelfBuffAbility(*this);
}

void SelfBuffAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    CooldownAbility::onCastUpdate();

    caster->addBuff(m_buffType);
}

void SelfBuffAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
    if (!repeating) {
        CooldownAbility::onCastUpdate();
    }
}

void SelfBuffAbility::loadFromJson(const rapidjson::Document& doc)
{
    CooldownAbility::loadFromJson(doc);

    m_buffType = Buff::stringToType(doc["buff"].GetString());
}
