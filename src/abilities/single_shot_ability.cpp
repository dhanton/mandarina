#include "abilities/single_shot_ability.hpp"

#include "unit.hpp"
#include "server_entity_manager.hpp"
#include "client_entity_manager.hpp"
#include "game_mode.hpp"

SingleShotAbility* SingleShotAbility::clone() const
{
    return new SingleShotAbility(*this);
}

void SingleShotAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    CooldownAbility::onCastUpdate();

    Projectile* projectile = context.entityManager->createProjectile(m_projectileType, caster->getPosition(), caster->getAimAngle(), caster->getTeamId());
    if (!projectile) return;

    const float multiplier = context.gameMode->getDamageMultiplier() * caster->getDamageMultiplier();

    projectile->shooterUniqueId = caster->getUniqueId();
    projectile->damage *= multiplier;

    //backtracking has to be done after damage multiplier is set
    Projectile_backtrackCollisions(*projectile, context, clientDelay);
}

void SingleShotAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
    if (repeating) return;
    
    CooldownAbility::onCastUpdate();

    //Using casterPos here doesn't look as good as using caster->getPosition() for some reason
    C_Projectile* projectile = context.entityManager->createProjectile(m_projectileType, unit->getPosition(), unit->getAimAngle(), unit->getTeamId());
    projectile->createdInputId = inputId;
}

void SingleShotAbility::loadFromJson(const rapidjson::Document& doc)
{
    CooldownAbility::loadFromJson(doc);

    m_projectileType = Projectile_stringToType(doc["projectile"].GetString());
}
