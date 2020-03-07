#pragma once

#include <string>
#include <SFML/System/Time.hpp>
#include "defines.hpp"

//@BRANCH_WIP: Most of how the abilities are done has to be changed
//Using virtual classes for more flexibility
//That includes removing the RechargeType

enum class RechargeType {
    None,

    //recharged by cooldown
    Cooldown,

    //never recharged nor casted
    Passive,

    //recharged by doing thing (dealing damage, gettings kills, objectives)
    Points
};

RechargeType rechargeTypeFromStr(const std::string& str);

enum AbilityType {
    ABILITY_NONE,

    #define DoAbility(ability_name, json_id) \
        ABILITY_##ability_name,
    #include "abilities.inc"
    #undef DoAbility

    ABILITY_MAX_TYPES
};

struct _AbilityCooldown
{
    u8 maxCharges;
    u8 currentCharges;
    float cooldown;
    float currentCooldown;

    //time that has to pass to use the next charge available
    float chargeRate;
};

struct _AbilityPoints
{
    float pointsMultiplier;
    float pointPercentage;
};

struct _AbilityPassive
{
    u16 passiveBuff;
};

struct Ability
{
    u8 type;
    RechargeType rechargeType;

    //we use one or the other depending on rechargeType
    union {
        _AbilityCooldown abCooldown;
        _AbilityPoints abPoints;
        _AbilityPassive abPassive;
    };  
};

class JsonParser;

extern Ability g_abilities[ABILITY_MAX_TYPES];

void loadAbilitiesFromJson(JsonParser* jsonParser);

u8 Ability_strToType(const std::string& typeStr);

void Ability_update(Ability& ability, sf::Time eTime);
void Ability_onCallback(Ability& ability);
bool Ability_canBeCasted(const Ability& ability);