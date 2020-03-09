#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include "context.hpp"
#include "entity.hpp"
#include "ability.hpp"

class CasterComponent
{
public:
    static void loadAbilityData(const JsonParser* jsonParser);

    CasterComponent() = default;

    //rule of 5 (methods needed because this class has unique_ptr and we want to copy it)
    ~CasterComponent() = default;
    CasterComponent(CasterComponent const& other);
    CasterComponent(CasterComponent && other) = default;
    CasterComponent& operator=(CasterComponent const& other);
    CasterComponent& operator=(CasterComponent && other) = default;

    void update(sf::Time eTime);
    void applyInput(Unit* caster, const PlayerInput& input, const ManagersContext& context, u16 clientDelay);

    //Since this class is not parent of C_Unit, this method has to be called for the controlledEntity only
    //by an interface on GameClient. This makes more sense than having each C_Unit keep track of their abilities.
    void C_applyInput(C_Unit* caster, const PlayerInput& input, const C_ManagersContext& context);

    Ability* getPrimaryFire() const;
    Ability* getSecondaryFire() const;
    Ability* getAltAbility() const;
    Ability* getUltimate() const;

protected:
    void loadFromJson(const rapidjson::Document& doc);

    std::unique_ptr<Ability> m_primaryFire;
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
