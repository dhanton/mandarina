#pragma once

#include "buff.hpp"

class InvisBuff : public Buff
{
public:
    virtual InvisBuff* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void onPreUpdate(sf::Time eTime);
    virtual void onGetDamageMultiplier(float& multiplier);

    virtual void onMovement();
    virtual void onPrimaryFireCasted();
    virtual void onSecondaryFireCasted();
    virtual void onAltAbilityCasted();
    virtual void onUltimateCasted();

private:
    bool m_unitSolid;
    float m_damageMultiplier;
    u16 m_bonusMovementSpeed;
    
    bool m_movingBreaksInvis;

    bool m_primaryFireBreaksInvis;
    bool m_secondaryFireBreaksInvis;
    bool m_altAbilityBreaksInvis;
    bool m_ultimateBreaksInvis;
};
