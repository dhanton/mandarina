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

    Projectile* projectile = nullptr;
    float multiplier = context.gameMode->getDamageMultiplier() * caster->getDamageMultiplier();

    for (int i = 0; i < m_bubbleNumber; ++i) {
        ABILITY_CREATE_PROJECTILE(PROJECTILE_HELLS_BUBBLE, caster->getPosition() + calculateRndPos(), caster->getAimAngle(), caster->getTeamId());
        ABILITY_SET_PROJECTILE_SHOOTER(caster)

        ABILITY_SET_PROJECTILE_DAMAGE_MULTIPLIER(multiplier)

        //no need to backtrack each projectile since they're not created in client
        // ABILITY_BACKTRACK_PROJECTILE(clientDelay)
    }
}

void HellsRainAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
    if (repeating) return;

    RechargeAbility::onCastUpdate();

    // C_Projectile* projectile = nullptr;

    //We don't create anything in client since the spread is random
    // for (int i = 0; i < m_bubbleNumber; ++i) {
    //     ABILITY_CREATE_PROJECTILE(PROJECTILE_HELLS_BUBBLE, caster->getPosition() + calculateRndPos(), caster->getAimAngle(), caster->getTeamId());
    //     ABILITY_SET_PROJECTILE_INPUT_ID(inputId)
    // }
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

Vector2 HellsRainAbility::calculateRndPos()
{
    float angle = (rand() % m_spreadAngle) * PI/180.f;
    float dist = rand() % m_spreadDistance;

    return Vector2(std::sin(angle) * dist, std::cos(angle) * dist);
}
