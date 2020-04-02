#pragma once

#include "context.hpp"
#include "ability.hpp"
#include "component.hpp"
#include "player_input.hpp"

//Used to tell the Caster extra conditions for each ability
struct CanCast_ExtraFlags 
{
    bool primaryFire = true;
    bool secondaryFire = true;
    bool altAbility = true;
    bool ultimate = true;
};

class CasterComponent
{
public:
    static void loadAbilityData(const JsonParser* jsonParser);

    CasterComponent() = default;

    //rule of 5 (methods needed because this class has unique_ptr and we want to copy it)
    virtual ~CasterComponent() = default;
    CasterComponent(CasterComponent const& other);
    CasterComponent(CasterComponent && other) = default;
    CasterComponent& operator=(CasterComponent const& other);
    CasterComponent& operator=(CasterComponent && other) = default;

    void update(sf::Time eTime);
    void applyInput(Unit* caster, const PlayerInput& input, const ManagersContext& context, u16 clientDelay, const CanCast_ExtraFlags& extraFlags);

    //Since this class is not parent of C_Unit, this method has to be called for the controlledEntity only
    //by an interface on GameClient. This makes more sense than having each C_Unit keep track of their abilities.
    //Also in client we don't modify the position of the unit directly but rather casterPos
    //If the input is being repeated we don't create new entities, only update the caster's position
    void C_applyInput(C_Unit* caster, Vector2& casterPos, const PlayerInput& input, const C_ManagersContext& context, bool repeating, const CanCast_ExtraFlags& extraFlags);

    CooldownAbility* getPrimaryFire() const;
    Ability* getSecondaryFire() const;
    Ability* getAltAbility() const;
    Ability* getUltimate() const;

protected:
    void loadFromJson(const rapidjson::Document& doc);

    std::unique_ptr<CooldownAbility> m_primaryFire;
    std::unique_ptr<Ability> m_secondaryFire;
    std::unique_ptr<Ability> m_altAbility;
    std::unique_ptr<Ability> m_ultimate;

private:
    static std::unique_ptr<Ability> m_abilityData[ABILITY_MAX_TYPES];
    static bool m_abilitiesLoaded;

    COMP_CROSS_VIRTUAL(casterComponent, u8, weaponId)
};

#define CASTER_COMPONENT() \
    COMP_CROSS_VARIABLE(casterComponent, u8, weaponId)
