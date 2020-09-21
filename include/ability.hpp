#pragma once

#include <SFML/System/Time.hpp>
#include "managers_context.hpp"
#include "json_parser.hpp"
#include "defines.hpp"
#include "buff.hpp"

class Unit;
class C_Unit;
class CasterComponent;

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
    virtual void C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& pos, const C_ManagersContext& context, u32 inputId, bool repeating) = 0;
    virtual void update(sf::Time eTime, GameMode* gameMode) = 0;

    virtual void refresh() = 0;
    
    //casting should depend only on internals of ability and the status of the unit
    //(both of which are displayed in client, which is important)
    virtual bool canBeCasted(const Status& status) const = 0;

    virtual void applyServerCorrection(float diff);

    virtual u16 takeSnapshot() const;
    void packData(CRCPacket& outPacket) const;

    //these are used to properly render UI
    //we should probably remove them when we add the option to have recharge abilites as alt ability or secondary fire
    virtual float getPercentage() const;
    virtual u16 getMaxTime() const;

    virtual float getTotalPercentage() const;

    //Abilities can add buffs to their casters when the caster is created
    //This can be used for triggers like: passive abilities, recharged when damage dealt, etc
    bool areBuffsAdded() const;
    virtual void addBuffsToCaster(Unit* unit, const ManagersContext& context);

    virtual void loadFromJson(const rapidjson::Document& doc);

private:
    u8 m_type;
    u8 m_iconTextureId;

    bool m_buffsAdded;
};

class CooldownAbility : public Ability
{
public:
    virtual CooldownAbility* clone() const = 0;

    virtual void update(sf::Time eTime, GameMode* gameMode);
    virtual bool canBeCasted(const Status& status) const;

    virtual void refresh();

    virtual void applyServerCorrection(float diff);

    virtual u16 takeSnapshot() const;

    void toZero();

    virtual float getPercentage() const;
    virtual u16 getMaxTime() const;

    virtual float getTotalPercentage() const;

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

    virtual void update(sf::Time eTime, GameMode* gameMode);
    virtual bool canBeCasted(const Status& status) const;
    virtual void refresh();

    virtual void applyServerCorrection(float diff);

    virtual u16 takeSnapshot() const;

    virtual float getPercentage() const;
    virtual void addToPercentage(float amount);

    virtual float getTotalPercentage() const;

    virtual void addBuffsToCaster(Unit* unit, const ManagersContext& context);

    virtual void loadFromJson(const rapidjson::Document& doc);

    float getRechargeMultiplier() const;
    float getTimeRechargeMultiplier() const;

protected:
    void onCastUpdate();

private:
    float m_percentage;

    //recharge caused by damage, stuns, kills, heal, etc
    float m_rechargeMultiplier;

    //recharge caused by time
    float m_timeRechargeMultiplier;
};

class PassiveAbility : public Ability
{
public:
    virtual PassiveAbility* clone() const = 0;

	virtual void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
    virtual void C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& pos, const C_ManagersContext& context, u32 inputId, bool repeating);

    virtual void update(sf::Time eTime, GameMode* gameMode);
    virtual bool canBeCasted(const Status& status) const;
    virtual void refresh();

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

#define ABILITY_SET_PROJECTILE_DAMAGE_MULTIPLIER(multiplier) if (projectile) {projectile->damage *= multiplier;}
