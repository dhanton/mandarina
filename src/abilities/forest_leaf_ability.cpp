#include "abilities/forest_leaf_ability.hpp"

#include "helper.hpp"
#include "unit.hpp"
#include "server_entity_manager.hpp"
#include "game_mode.hpp"

ForestLeafAbility* ForestLeafAbility::clone() const
{
    return new ForestLeafAbility(*this);
}

void ForestLeafAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    CooldownAbility::onCastUpdate();

    Projectile* projectile = context.entityManager->createProjectile(m_projectileType, caster->getPosition(), caster->getAimAngle(), caster->getTeamId());
    if (!projectile) return;

    const float multiplier = context.gameMode->getDamageMultiplier() * caster->getDamageMultiplier();

    projectile->shooterUniqueId = caster->getUniqueId();
    projectile->damage *= multiplier;
    projectile->buffAppliedType = (Helper_Random::coinFlip() ? BUFF_FOREST_SLOW : BUFF_FOREST_SILENCE);

    //backtracking has to be done after damage multiplier is set
    Projectile_backtrackCollisions(*projectile, context, clientDelay);
}
