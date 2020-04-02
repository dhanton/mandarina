#pragma once

#include <SFML/System/Time.hpp>
#include "managers_context.hpp"
#include "json_parser.hpp"
#include "defines.hpp"
#include "buff.hpp"

class Unit;
class C_Unit;

enum AbilityType {
    ABILITY_NONE,

    #define DoAbility(class_name, type, json_id) \
        ABILITY_##type,
    #include "abilities.inc"
    #undef DoAbility

    ABILITY_MAX_TYPES
};

class Ability
{
public:
    static u8 stringToType(const std::string& typeStr);

    virtual Ability* clone() const = 0;

    u8 getAbilityType() const;
    void setAbilityType(u8 abilityType);

    u8 getIconTextureId() const;
    void setIconTextureId(u8 iconTextureId);

    virtual void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay) = 0;
    virtual void C_onCast(C_Unit* caster, Vector2& pos, const C_ManagersContext& context, u32 inputId, bool repeating) = 0;
    virtual void update(sf::Time eTime) = 0;
    
    //casting should depend only on internals of ability and the status of the unit
    //(both of which are displayed in client, which is important)
    virtual bool canBeCasted(const Status& status) const = 0;

    //these are used to properly render UI
    //we should probably remove them when we add the option to have recharge abilites as alt ability or secondary fire
    virtual float getPercentage() const;
    virtual u16 getMaxTime() const;

    virtual void loadFromJson(const rapidjson::Document& doc) = 0;

private:
    u8 m_type;
    u8 m_iconTextureId;
};

class CooldownAbility : public Ability
{
public:
    virtual CooldownAbility* clone() const = 0;

    virtual void update(sf::Time eTime);
    virtual bool canBeCasted(const Status& status) const;

    virtual float getPercentage() const;
    virtual u16 getMaxTime() const;

    virtual void loadFromJson(const rapidjson::Document& doc);

    u8 getMaxCharges() const;
    u8 getCurrentCharges() const;
    float getCooldown() const;
    float getCurrentCooldown() const;

protected:
    void onCastUpdate();

private:
    u8 m_maxCharges;
    u8 m_currentCharges;
    float m_cooldown;
    float m_currentCooldown;

    float m_nextChargeDelay;
    float m_currentNextChargeDelay;
};

class RechargeAbility : public Ability
{
public:
    virtual RechargeAbility* clone() const = 0;

    virtual void update(sf::Time eTime);
    virtual bool canBeCasted(const Status& status) const;

    virtual float getPercentage() const;

    virtual void loadFromJson(const rapidjson::Document& doc);

    //The ability needs to be charged when the unit deals damage
    //or performs different in-game actions
    //(use a buff for this??)

protected:
    void onCastUpdate();

private:
    float m_percentage;
    float m_rechargeMultiplier;
};

class PassiveAbility : public Ability
{
public:
    virtual PassiveAbility* clone() const = 0;

    virtual void update(sf::Time eTime);
    virtual bool canBeCasted(const Status& status) const;

    virtual void loadFromJson(const rapidjson::Document& doc);

private:
    //passive buff to be applied to the unit 
};

//These macros can be used by both onCast and C_onCast to create projectiles
//To use them, a projectile pointer must be defined previously

#define ABILITY_CREATE_PROJECTILE(type, pos, angle, teamId) \
    projectile = context.entityManager->createProjectile(type, pos, angle, teamId);
    
#define ABILITY_BACKTRACK_PROJECTILE(delay) if (projectile) {Projectile_backtrackCollisions(*projectile, context, delay);}

#define ABILITY_SET_PROJECTILE_SHOOTER(caster) if (projectile && caster) {projectile->shooterUniqueId = caster->getUniqueId();}

#define ABILITY_SET_PROJECTILE_INPUT_ID(inputId) if (projectile) {projectile->createdInputId = inputId;}
