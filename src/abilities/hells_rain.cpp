#include "abilities/hells_rain.hpp"

#include "unit.hpp"
#include "client_entity_manager.hpp"
#include "server_entity_manager.hpp"
#include "helper.hpp"
#include "game_mode.hpp"

HellsRainAbility* HellsRainAbility::clone() const
{
    return new HellsRainAbility(*this);
}

void HellsRainAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    RechargeAbility::onCastUpdate();

    const float multiplier = context.gameMode->getDamageMultiplier() * caster->getDamageMultiplier();

    std::uniform_int_distribution<u16> spreadDistr(0, m_spreadDistance);

    for (int i = 0; i < m_bubbleNumber; ++i) {
        const Vector2 randPos = calculateRndPos(spreadDistr);

        Projectile* projectile = context.entityManager->createProjectile(PROJECTILE_HELLS_BUBBLE, caster->getPosition() + randPos, caster->getAimAngle(), caster->getTeamId());
        if (!projectile) continue;

        projectile->shooterUniqueId = caster->getUniqueId();
        projectile->damage *= multiplier;

        //no need to backtrack each projectile since they're not created in client
    }
}

void HellsRainAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
    if (repeating) return;

    RechargeAbility::onCastUpdate();

    //We don't create anything in client since the spread is random
}

void HellsRainAbility::loadFromJson(const rapidjson::Document& doc)
{
    RechargeAbility::loadFromJson(doc);

    m_bubbleNumber = doc["bubble_number"].GetUint();
    
    if (doc.HasMember("spread_angle")) {
        m_spreadAngle = std::min(doc["spread_angle"].GetUint(), 360u);
    } else {
        m_spreadAngle = 360;
    }
    
    m_spreadDistance = doc["spread_distance"].GetUint();
}

Vector2 HellsRainAbility::calculateRndPos(std::uniform_int_distribution<u16>& distr)
{
    float angle = Helper_Random::rndAngleRadians();
    float dist = distr(Helper_Random::gen());

    return Vector2(std::sin(angle) * dist, std::cos(angle) * dist);
}
