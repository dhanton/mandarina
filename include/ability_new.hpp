#pragma once

#include <SFML/System/Time.hpp>
#include "managers_context.hpp"
#include "json_parser.hpp"
#include "defines.hpp"

class Unit;
class C_Unit;

//@BRANCH_WIP
//The main difference with entities is that most abilities 
//need specific funcionality associated with them
//Each new ability needs new onCast method, and may also override the rest of the methods
//But all the classes here are pure virtual classes 
//Still each layer of abilities implements their own loadFromJson method
//Building upon the previous ones

//static ability data is stored in the unit classes (??) 
//Or where??? since they're the same for clients and servers

class Ability
{
public:
    virtual void onCast(Unit* caster, const ManagersContext& context) = 0;
    virtual void C_onCast(C_Unit* caster, const C_ManagersContext& context) = 0;
    virtual void update(sf::Time eTime) = 0;
    virtual bool canBeCasted() = 0;

protected:
    virtual void loadFromJson(const rapidjson::Document& doc) = 0;
};

class CooldownAbility : public Ability
{
public:
    CooldownAbility();

    virtual void onCast(Unit* caster, const ManagersContext& context);
    virtual void C_onCast(C_Unit* caster, const C_ManagersContext& context);
    virtual void update(sf::Time eTime);
    virtual bool canBeCasted();

private:
    u8 m_maxCharges;
    u8 m_currentCharges;
    float m_cooldown;
    float m_currentCooldown;

    float m_nextChargeTime;
};

class RechargeAbility : public Ability
{
public:
    RechargeAbility();

    virtual void onCast(Unit* caster, const ManagersContext& context);
    virtual void C_onCast(C_Unit* caster, const C_ManagersContext& context);
    virtual void update(sf::Time eTime);
    virtual bool canBeCasted();

    //The ability needs to be charged when the unit deals damage
    //or performs different in-game actions
    //(use a buff for this??)

private:
    float m_percentage;
    float m_multiplier;
};

class PassiveAbility : public Ability
{
public:
    PassiveAbility();

    virtual void onCast(Unit* caster, const ManagersContext& context);
    virtual void C_onCast(C_Unit* caster, const C_ManagersContext& context);
    virtual void update(sf::Time eTime);
    virtual bool canBeCasted();

private:
    //passive buff to be applied to the unit 
};