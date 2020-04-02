#include "caster_component.hpp"

#include "weapon.hpp"
#include "unit.hpp"
#include "json_parser.hpp"
#include "texture_ids.hpp"

//all abilities have to be included for loadAbilityData to work
#include "abilities/single_shot_ability.hpp"
#include "abilities/hells_dash.hpp"
#include "abilities/hells_rain.hpp"

bool CasterComponent::m_abilitiesLoaded = false;
std::unique_ptr<Ability> CasterComponent::m_abilityData[ABILITY_MAX_TYPES];

void CasterComponent::loadAbilityData(const JsonParser* jsonParser)
{
    if (m_abilitiesLoaded) return;

    #define DoAbility(class_name, type, json_id) \
        m_abilityData[ABILITY_##type] = std::unique_ptr<Ability>(new class_name()); \
        m_abilityData[ABILITY_##type]->setAbilityType(ABILITY_##type); \
        m_abilityData[ABILITY_##type]->setIconTextureId(TextureId::ICON_##type); \
        m_abilityData[ABILITY_##type]->loadFromJson(*jsonParser->getDocument(json_id));
    #include "abilities.inc"
    #undef DoAbility

    m_abilitiesLoaded = true;
}

CasterComponent::CasterComponent(CasterComponent const& other):
    m_primaryFire(other.m_primaryFire ? other.m_primaryFire->clone() : nullptr),
    m_secondaryFire(other.m_secondaryFire ? other.m_secondaryFire->clone() : nullptr),
    m_altAbility(other.m_altAbility ? other.m_altAbility->clone() : nullptr),
    m_ultimate(other.m_ultimate ? other.m_ultimate->clone() : nullptr)
{

}

CasterComponent& CasterComponent::operator=(CasterComponent const& other)
{
    m_primaryFire = std::unique_ptr<CooldownAbility>(other.m_primaryFire ? other.m_primaryFire->clone() : nullptr);
    m_secondaryFire = std::unique_ptr<Ability>(other.m_secondaryFire ? other.m_secondaryFire->clone() : nullptr);
    m_altAbility = std::unique_ptr<Ability>(other.m_altAbility ? other.m_altAbility->clone() : nullptr);
    m_ultimate = std::unique_ptr<Ability>(other.m_ultimate ? other.m_ultimate->clone() : nullptr);
    return *this;
}

void CasterComponent::update(sf::Time eTime)
{
    if (m_primaryFire) {
        m_primaryFire->update(eTime);
    }

    if (m_secondaryFire) {
        m_secondaryFire->update(eTime);
    }

    if (m_altAbility) {
        m_altAbility->update(eTime);
    }

    if (m_ultimate) {
        m_ultimate->update(eTime);
    }
}

void CasterComponent::applyInput(Unit* caster, const PlayerInput& input, const ManagersContext& context, u16 clientDelay, const CanCast_ExtraFlags& extraFlags)
{
    if (input.primaryFire && extraFlags.primaryFire && m_primaryFire && m_primaryFire->canBeCasted(caster->getStatus())) {
        m_primaryFire->onCast(caster, context, clientDelay);
    }

    if (input.secondaryFire && extraFlags.secondaryFire && m_secondaryFire && m_secondaryFire->canBeCasted(caster->getStatus())) {
        m_secondaryFire->onCast(caster, context, clientDelay);
    }

    if (input.altAbility && extraFlags.altAbility && m_altAbility && m_altAbility->canBeCasted(caster->getStatus())) {
        m_altAbility->onCast(caster, context, clientDelay);
    }

    if (input.ultimate && extraFlags.ultimate && m_ultimate && m_ultimate->canBeCasted(caster->getStatus())) {
        m_ultimate->onCast(caster, context, clientDelay);
    }
}

void CasterComponent::C_applyInput(C_Unit* caster, Vector2& casterPos, const PlayerInput& input, const C_ManagersContext& context, bool repeating, const CanCast_ExtraFlags& extraFlags)
{
    if (input.primaryFire && extraFlags.primaryFire && m_primaryFire && m_primaryFire->canBeCasted(caster->getStatus())) {
        m_primaryFire->C_onCast(caster, casterPos, context, input.id, repeating);
    }

    if (input.secondaryFire && extraFlags.secondaryFire && m_secondaryFire && m_secondaryFire->canBeCasted(caster->getStatus())) {
        m_secondaryFire->C_onCast(caster, casterPos, context, input.id, repeating);
    }

    if (input.altAbility && extraFlags.altAbility && m_altAbility && m_altAbility->canBeCasted(caster->getStatus())) {
        m_altAbility->C_onCast(caster, casterPos, context, input.id, repeating);
    }

    if (input.ultimate && extraFlags.ultimate && m_ultimate && m_ultimate->canBeCasted(caster->getStatus())) {
        m_ultimate->C_onCast(caster, casterPos, context, input.id, repeating);
    }
}

CooldownAbility* CasterComponent::getPrimaryFire() const
{
    return m_primaryFire.get();
}

Ability* CasterComponent::getSecondaryFire() const
{
    return m_secondaryFire.get();
}

Ability* CasterComponent::getAltAbility() const
{
    return m_altAbility.get();
}

Ability* CasterComponent::getUltimate() const
{
    return m_ultimate.get();
}

void CasterComponent::loadFromJson(const rapidjson::Document& doc)
{
    u8 weaponId = _casterComponent_weaponId();
    const Weapon& weapon = g_weaponData[weaponId];

    m_primaryFire = nullptr;
    m_secondaryFire = nullptr;
    m_altAbility = nullptr;
    m_ultimate = nullptr;

    if (weapon.primaryFire != ABILITY_NONE) {
        //this cast will fail if ability is not a CooldownAbility (but all primary fires are)
        m_primaryFire = std::unique_ptr<CooldownAbility>(static_cast<CooldownAbility*>(m_abilityData[weapon.primaryFire]->clone()));
    }

    if (weapon.secondaryFire != ABILITY_NONE) {
        m_secondaryFire = std::unique_ptr<Ability>(m_abilityData[weapon.secondaryFire]->clone());
    }

    if (doc.HasMember("alt_ability")) {
        u8 abilityType = Ability::stringToType(doc["alt_ability"].GetString());

        if (abilityType != ABILITY_NONE) {
            m_altAbility = std::unique_ptr<Ability>(m_abilityData[abilityType]->clone());
        }
    }

    if (doc.HasMember("ultimate")) {
        u8 abilityType = Ability::stringToType(doc["ultimate"].GetString());

        if (abilityType != ABILITY_NONE) {
            m_ultimate = std::unique_ptr<Ability>(m_abilityData[abilityType]->clone());
        }
    }
}
