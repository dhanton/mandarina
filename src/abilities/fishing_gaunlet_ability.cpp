#include "abilities/fishing_gaunlet_ability.hpp"

#include "unit.hpp"
#include "projectiles.hpp"
#include "buff.hpp"
#include "entity.hpp"
#include "server_entity_manager.hpp"
#include "game_mode.hpp"

FishingGaunletAbility* FishingGaunletAbility::clone() const
{
    return new FishingGaunletAbility(*this);
}

void FishingGaunletAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    CooldownAbility::onCastUpdate();

    Projectile* projectile = context.entityManager->createProjectile(m_projectileType, caster->getPosition(), caster->getAimAngle(), caster->getTeamId());
    if (!projectile) return;

    projectile->shooterUniqueId = caster->getUniqueId();

    const u16 healthRemoved = m_healthRemoved * caster->getHealth();
    u16 selfDamage = healthRemoved;

    //because of MeatShield, damage done might be negated (which sets selfDamage to 0)
    //even if this is the case we still want the projectile to do damage, and that's why we use 2 different variables
    caster->takeDamage(selfDamage, caster, caster->getUniqueId(), caster->getTeamId());
    projectile->damage = (healthRemoved * m_healthToDamage) * context.gameMode->getDamageMultiplier(); 

    Projectile_backtrackCollisions(*projectile, context, clientDelay);
}

void FishingGaunletAbility::addBuffsToCaster(Unit* unit, const ManagersContext& context)
{
    SingleShotAbility::addBuffsToCaster(unit, context);

    Buff* buff = unit->addBuff(BUFF_FISHING_GAUNLET);
}

void FishingGaunletAbility::loadFromJson(const rapidjson::Document& doc)
{
    CooldownAbility::loadFromJson(doc);

    m_projectileType = PROJECTILE_FISHING_GAUNLET;

    m_healthRemoved = doc["health_removed"].GetFloat();
    m_healthToDamage = doc["health_to_damage"].GetFloat();
}
