#include "abilities/single_shot_ability.hpp"

#include "unit.hpp"
#include "server_entity_manager.hpp"
#include "client_entity_manager.hpp"

SingleShotAbility* SingleShotAbility::clone()
{
    return new SingleShotAbility(*this);
}

void SingleShotAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    CooldownAbility::onCastUpdate();

    Projectile* projectile = nullptr;

    ABILITY_CREATE_PROJECTILE(m_projectileType, caster->getPosition(), caster->getAimAngle(), caster->getTeamId())
    ABILITY_SET_PROJECTILE_SHOOTER(caster)
    ABILITY_BACKTRACK_PROJECTILE(clientDelay)
}

void SingleShotAbility::C_onCast(C_Unit* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
    if (repeating) return;
    
    CooldownAbility::onCastUpdate();

    C_Projectile* projectile = nullptr;

    //Using casterPos here doesn't look as good as using caster->getPosition() for some reason
    ABILITY_CREATE_PROJECTILE(m_projectileType, caster->getPosition(), caster->getAimAngle(), caster->getTeamId())
    ABILITY_SET_PROJECTILE_INPUT_ID(inputId)
}

void SingleShotAbility::loadFromJson(const rapidjson::Document& doc)
{
    CooldownAbility::loadFromJson(doc);

    m_projectileType = Projectile_stringToType(doc["projectile"].GetString());
}
