#pragma once

#include "caster_component.hpp"
#include "ability_ui.hpp"
#include "caster_snapshot.hpp"

class ClientCaster : public sf::Drawable, public InContext
{
public:
    ClientCaster(const Context& context);

    void update(sf::Time eTime, GameMode* gameMode);
    void applyInputs(const PlayerInput& input, Vector2& casterPos, const C_ManagersContext& context);
    void reapplyInputs(const PlayerInput& input, Vector2& casterPos, const C_ManagersContext& context);

    CasterSnapshot takeSnapshot() const;
    void applyServerCorrection(const CasterSnapshot& diffSnapshot);

    void setCaster(C_Unit* caster, GameMode* gameMode);
    void forceCasterUpdate();
    C_Unit* getCaster() const;

    //needed by controlled unit to render its UI
    const CooldownAbility* getPrimaryFire() const;

    void setSpectating(bool spectating);
    bool getSpectating() const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    CanCast_ExtraFlags getExtraFlags() const;

    //We need to overwrite the method below for the class to work
    //Even though we're not going to use it (used only by CasterComponent::loadFromJson)
    class DummyCaster : public CasterComponent {
    public:
        void setPrimaryFire(CooldownAbility* ability);
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

    bool m_spectating;
};
