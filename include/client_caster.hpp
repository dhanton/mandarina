#pragma once

#include "caster_component.hpp"
#include "ability_ui.hpp"

//This class keeps track of the 
class ClientCaster : public sf::Drawable, public InContext
{
public:
    ClientCaster(const Context& context);

    void update(sf::Time eTime);
    void applyInputs(const PlayerInput& input, Vector2& casterPos, const C_ManagersContext& context);
    void reapplyInputs(const PlayerInput& input, Vector2& casterPos, const C_ManagersContext& context);

    void setCaster(C_Unit* caster);
    C_Unit* getCaster() const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    //We need to overwrite the method below for the class to work
    //Even though we're not going to use it (used only by CasterComponent::loadFromJson)
    class DummyCaster : public CasterComponent {
    public:
        void setPrimaryFire(Ability* ability);
        void setSecondaryFire(Ability* ability);
        void setAltAbility(Ability* ability);
        void setUltimate(Ability* ability);

        bool isValid() const;
    private:
        u8 _casterComponent_weaponId() const;
    };

    DummyCaster m_casterComponent;
    C_Unit* m_caster;

    AbilityUI m_secondaryUI;
    AbilityUI m_altAbilityUI;
    AbilityUI m_ultimateUI;
};
