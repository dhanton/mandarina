#include "buffs/invis_buff.hpp"

#include "unit.hpp"

InvisBuff* InvisBuff::clone() const
{
    return new InvisBuff(*this);
}

void InvisBuff::loadFromJson(const rapidjson::Document& doc)
{
    Buff::loadFromJson(doc);

    if (doc.HasMember("unit_solid")) {
        m_unitSolid = doc["unit_solid"].GetBool();
    } else {
        m_unitSolid = true;
    }

    if (doc.HasMember("damage_multiplier")) {
        m_damageMultiplier = doc["damage_multiplier"].GetFloat();
    } else {
        m_damageMultiplier = 0.f;
    }

    if (doc.HasMember("primary_fire_breaks_invis")) {
        m_primaryFireBreaksInvis = doc["primary_fire_breaks_invis"].GetBool();
    } else {
        m_primaryFireBreaksInvis = true;
    }

    if (doc.HasMember("secondary_fire_breaks_invis")) {
        m_secondaryFireBreaksInvis = doc["secondary_fire_breaks_invis"].GetBool();
    } else {
        m_secondaryFireBreaksInvis = true;
    }

    if (doc.HasMember("alt_ability_breaks_invis")) {
        m_altAbilityBreaksInvis = doc["alt_ability_breaks_invis"].GetBool();
    } else {
        m_altAbilityBreaksInvis = true;
    }

    if (doc.HasMember("ultimate_breaks_invis")) {
        m_ultimateBreaksInvis = doc["ultimate_breaks_invis"].GetBool();
    } else {
        m_ultimateBreaksInvis = true;
    }

    if (doc.HasMember("moving_breaks_invis")) {
        m_movingBreaksInvis = doc["moving_breaks_invis"].GetBool();
    } else {
        m_movingBreaksInvis = false;
    }

    if (doc.HasMember("bonus_movement_speed")) {
        m_bonusMovementSpeed = doc["bonus_movement_speed"].GetUint();
    } else {
        m_bonusMovementSpeed = 0;
    }
}

void InvisBuff::onPreUpdate(sf::Time eTime)
{
    m_unit->setInvisible(true);
    
    //it's important to not set solid if it's true
    //since other buffs might want to set it to false
    if (!m_unitSolid) {
        m_unit->setSolid(false);
    }

    m_unit->setMovementSpeed(m_unit->getMovementSpeed() + m_bonusMovementSpeed);
}

void InvisBuff::onGetDamageMultiplier(float& multiplier)
{
    multiplier += m_damageMultiplier;
}

void InvisBuff::onMovement()
{
    if (m_movingBreaksInvis) {
        m_dead = true;
    }
}

void InvisBuff::onPrimaryFireCasted()
{
    if (m_primaryFireBreaksInvis) {
        m_dead = true;
    }
}

void InvisBuff::onSecondaryFireCasted()
{
    if (m_secondaryFireBreaksInvis) {
        m_dead = true;
    }
}

void InvisBuff::onAltAbilityCasted()
{
    if (m_altAbilityBreaksInvis) {
        m_dead = true;
    }
}

void InvisBuff::onUltimateCasted()
{
    if (m_ultimateBreaksInvis) {
        m_dead = true;
    }
}
