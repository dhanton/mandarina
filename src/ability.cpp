#include "ability.hpp"

#include "helper.hpp"
#include "unit.hpp"

u8 Ability::stringToType(const std::string& typeStr)
{
    if (typeStr == "NONE") return ABILITY_NONE;

    #define DoAbility(class_name, type, json_id) \
        if (typeStr == #type) return ABILITY_##type;
    #include "abilities.inc"
    #undef DoAbility

    return ABILITY_NONE;
}

u8 Ability::getAbilityType() const
{
    return m_type;
}

void Ability::setAbilityType(u8 abilityType)
{
    m_type = abilityType;
}

void CooldownAbility::update(sf::Time eTime)
{
    if (m_currentCharges < m_maxCharges) {
        float delta = m_currentCooldown - eTime.asSeconds();

        if (delta <= 0.f) {
            m_currentCharges++;

            if (m_currentCharges < m_maxCharges) {
                m_currentCooldown = m_cooldown + delta;
            }
        } else {
            m_currentCooldown = delta;
        }
    }

    if (m_currentNextChargeDelay < m_nextChargeDelay) {
        m_currentNextChargeDelay += eTime.asSeconds();
    }
}

bool CooldownAbility::canBeCasted()
{
    return m_currentCharges > 0 && m_currentNextChargeDelay >= m_nextChargeDelay;
}

void CooldownAbility::loadFromJson(const rapidjson::Document& doc)
{
    m_maxCharges = doc["charges"].GetInt();
    m_cooldown = doc["cooldown"].GetFloat();

    if (doc.HasMember("next_charge_delay")) {
        m_nextChargeDelay = doc["next_charge_delay"].GetFloat();
    } else {
        m_nextChargeDelay = 0.f;
    }
    
    m_currentNextChargeDelay = 0.f;

    if (doc.HasMember("starting_charges")) {
        m_currentCharges = doc["starting_charges"].GetInt();
    } else {
        m_currentCharges = m_maxCharges;
    }

    if (doc.HasMember("starting_cooldown")) {
        m_currentCooldown = doc["starting_cooldown"].GetFloat();
    } else {
        m_currentCooldown = 0.f;
    }
}

void CooldownAbility::onCastUpdate()
{
    if (m_currentCharges != 0) {
        m_currentCharges--;
        m_currentCooldown = m_cooldown;
        m_currentNextChargeDelay = 0.f;
    }
}

void RechargeAbility::update(sf::Time eTime)
{

}

bool RechargeAbility::canBeCasted()
{
    return m_percentage >= 1.f;
}

void RechargeAbility::loadFromJson(const rapidjson::Document& doc)
{
    if (doc.HasMember("starting_percentage")) {
        m_percentage = Helper_clamp(doc["starting_percentage"].GetFloat(), 0.f, 1.f);
    } else {
        m_percentage = 0.f;
    }

    if (doc.HasMember("multiplier")) {
        m_multiplier = doc["multiplier"].GetFloat();
    } else {
        m_multiplier = 1.f;
    }
}

void RechargeAbility::onCastUpdate()
{
    m_percentage = 0.f;
}

void PassiveAbility::update(sf::Time eTime)
{

}

bool PassiveAbility::canBeCasted()
{

}

void PassiveAbility::loadFromJson(const rapidjson::Document& doc)
{

}
