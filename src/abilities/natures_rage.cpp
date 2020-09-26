#include "abilities/natures_rage.hpp"

#include "unit.hpp"
#include "client_entity_manager.hpp"
#include "server_entity_manager.hpp"
#include "helper.hpp"

NaturesRageAbility* NaturesRageAbility::clone() const
{
    return new NaturesRageAbility(*this);
}

void NaturesRageAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    RechargeAbility::onCastUpdate();

    Projectile* projectile = nullptr;

    //we need it in radians
    const float aimAngle = caster->getAimAngle() * PI/180.f;

    for (int i = 0; i < m_rockNumber; ++i) {
        const float extraAngle = (i + 1) * m_anglePerRock;
        const Vector2 dir = Vector2(std::sin(aimAngle + extraAngle), std::cos(aimAngle + extraAngle));
        const Vector2 pos = caster->getPosition() + dir * static_cast<float>(m_spreadDistance);

        Projectile* projectile = context.entityManager->createProjectile(PROJECTILE_NATURES_ROCK, pos, (aimAngle + extraAngle) * 180.f/PI, caster->getTeamId());
        if (!projectile) continue;

        projectile->shooterUniqueId = caster->getUniqueId();
    }

    caster->getPrimaryFire()->refresh();
    caster->getSecondaryFire()->refresh();
    caster->getAltAbility()->refresh();
}

void NaturesRageAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
    if (repeating) return;

    RechargeAbility::onCastUpdate();
}

void NaturesRageAbility::loadFromJson(const rapidjson::Document& doc)
{
    RechargeAbility::loadFromJson(doc);

    m_rockNumber = doc["rock_number"].GetUint();
    m_spreadDistance = doc["spread_distance"].GetUint();

    //in radians
    m_anglePerRock = (2*PI)/m_rockNumber;
}
