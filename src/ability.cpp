#include "ability.hpp"

#include "helper.hpp"
#include "json_parser.hpp"

RechargeType rechargeTypeFromStr(const std::string& str)
{
    if (str == "Cooldown") {
        return RechargeType::Cooldown;
    } else if (str == "Passive") {
        return RechargeType::Passive;
    } else if (str == "Points") {
        return RechargeType::Points;
    }

    return RechargeType::None;
}

Ability g_abilities[ABILITY_MAX_TYPES];

void _loadAbility(JsonParser* jsonParser, AbilityType type, Ability& ability, const char* json_id)
{
    auto* doc = jsonParser->getDocument(json_id);

    ability.type = type;
    ability.rechargeType = rechargeTypeFromStr((*doc)["recharge_type"].GetString());

    if (ability.rechargeType == RechargeType::Cooldown) {
        ability.abCooldown.maxCharges = (*doc)["charges"].GetInt();
        ability.abCooldown.cooldown = (*doc)["cooldown"].GetFloat();

        if (doc->HasMember("charge_rate")) {
            ability.abCooldown.chargeRate = (*doc)["charge_rate"].GetFloat();
        } else {
            ability.abCooldown.chargeRate = 0.f;
        }

        if (doc->HasMember("starting_charges")) {
            ability.abCooldown.currentCharges = (*doc)["starting_charges"].GetInt();
        } else {
            ability.abCooldown.currentCharges = ability.abCooldown.maxCharges;
        }

        if (doc->HasMember("starting_cooldown")) {
            ability.abCooldown.currentCooldown = (*doc)["starting_cooldown"].GetFloat();
        } else {
            ability.abCooldown.currentCooldown = 0.f;
        }

    } else if (ability.rechargeType == RechargeType::Points) {
        if (doc->HasMember("starting_percentage")) {
            ability.abPoints.pointPercentage = Helper_clamp((*doc)["starting_percentage"].GetFloat(), 0.f, 1.f);
        } else {
            ability.abPoints.pointPercentage = 0.f;
        }

        if (doc->HasMember("points_multiplier")) {
            ability.abPoints.pointPercentage = (*doc)["points_multiplier"].GetFloat();
        } else {
            ability.abPoints.pointPercentage = 1.f;
        }

        ability.abPoints.pointsMultiplier = 1.f;
    }
}

void loadAbilitiesFromJson(JsonParser* jsonParser)
{
    #define LoadAbility(ability_name, json_id) \
        _loadAbility(jsonParser, ABILITY_##ability_name, g_abilities[ABILITY_##ability_name], json_id);
    #include "abilities.inc"
    #undef LoadAbility
}
void Ability_update(Ability& ability, sf::Time eTime)
{
    float dt = eTime.asSeconds();

    switch (ability.rechargeType) 
    {
        case RechargeType::Cooldown:
        {
            if (ability.abCooldown.currentCharges < ability.abCooldown.maxCharges) {
                if (ability.abCooldown.currentCooldown != 0.f) {
                    ability.abCooldown.currentCooldown = std::max(ability.abCooldown.currentCooldown - dt, 0.f);

                    if (std::fmod(ability.abCooldown.currentCooldown, ability.abCooldown.cooldown) == 0) {
                        ability.abCooldown.currentCharges++;
                    }
                }
            }
            break;
        }
    }
}

void Ability_onCallback(Ability& ability)
{
    switch (ability.rechargeType) 
    {
        case RechargeType::Cooldown:
        {
            if (ability.abCooldown.currentCharges != 0) {
                ability.abCooldown.currentCharges--;
                ability.abCooldown.currentCooldown += ability.abCooldown.cooldown;
            }
            break;
        }

        case RechargeType::Points:
        {
            ability.abPoints.pointPercentage = 0;
            break;
        }
    }
}

bool Ability_canBeCasted(const Ability& ability)
{
    switch (ability.rechargeType) 
    {
        case RechargeType::Cooldown:
        {
            return ability.abCooldown.currentCharges > 0;
        }

        case RechargeType::Points:
        {
            return ability.abPoints.pointPercentage >= 1;
        }
    }

    return false;
}
