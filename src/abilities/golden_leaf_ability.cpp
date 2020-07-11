#include "abilities/golden_leaf_ability.hpp"

#include "unit.hpp"
#include "server_entity_manager.hpp"
#include "client_entity_manager.hpp"
#include "game_mode.hpp"

GoldenLeafAbility* GoldenLeafAbility::clone() const
{
    return new GoldenLeafAbility(*this);
}

void GoldenLeafAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
    CooldownAbility::onCastUpdate();

    Projectile* projectile = nullptr;

    ABILITY_CREATE_PROJECTILE(PROJECTILE_GOLDEN_LEAF, caster->getPosition(), caster->getAimAngle(), caster->getTeamId())
    ABILITY_SET_PROJECTILE_SHOOTER(caster)

    CooldownAbility* primaryFire = caster->getPrimaryFire();

    float chargesLeft = static_cast<float>(primaryFire->getCurrentCharges());

    if (chargesLeft > 0) {
        float baseMultiplier = context.gameMode->getDamageMultiplier() * caster->getDamageMultiplier() + m_damageMultiplier;
        projectile->damage = (projectile->damage + g_initialProjectileData[PROJECTILE_FOREST_LEAF].damage * chargesLeft) * baseMultiplier;

        primaryFire->toZero();
    }

    //backtracking has to be done after damage multiplier is set
    ABILITY_BACKTRACK_PROJECTILE(clientDelay)
}

void GoldenLeafAbility::C_onCast(C_Unit* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
    if (repeating) return;
    
    CooldownAbility::onCastUpdate();

    C_Projectile* projectile = nullptr;
    
    ABILITY_CREATE_PROJECTILE(PROJECTILE_GOLDEN_LEAF, caster->getPosition(), caster->getAimAngle(), caster->getTeamId())
    ABILITY_SET_PROJECTILE_INPUT_ID(inputId)

    //calling primaryFire->toZero() is complicated in client, but the input will be corrected so it's fine
}

void GoldenLeafAbility::loadFromJson(const rapidjson::Document& doc)
{
    CooldownAbility::loadFromJson(doc);

    m_damageMultiplier = doc["damage_multiplier"].GetFloat();
}
