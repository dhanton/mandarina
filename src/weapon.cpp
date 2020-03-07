#include "weapon.hpp"

#include "json_parser.hpp"
#include "texture_ids.hpp"
#include "ability.hpp"

Weapon g_weaponData[WEAPON_MAX_TYPES];

void _loadWeapon(JsonParser* jsonParser, Weapon& weapon, const char* json_id, u16 textureId)
{
    auto* doc = jsonParser->getDocument(json_id);

    weapon.textureId = textureId;

    //All weapons have a primary fire ability
    weapon.primaryFire = Ability_strToType((*doc)["primary_fire"].GetString());

    //(different from WEAPON_NONE)
    if (weapon.primaryFire == WEAPON_NONE) {
        throw std::runtime_error("LoadWeaponsFromJson error - Weapon must have primary fire in file " + std::string(json_id));
    }

    //but they might not have a secondary fire
    if (doc->HasMember("secondary_fire")) {
        weapon.secondaryFire = Ability_strToType((*doc)["secondary_fire"].GetString());
    } else {
        weapon.secondaryFire = ABILITY_NONE;
    }

    if (doc->HasMember("scale")) {
        weapon.scale = (*doc)["scale"].GetFloat();
    } else {
        weapon.scale = 1.f;
    }

    if (doc->HasMember("angle_offset")) {
        weapon.angleOffset = (*doc)["angle_offset"].GetFloat();
    } else {
        weapon.angleOffset = 0.f;
    }
}

u8 Weapon_stringToType(const std::string& typeStr)
{
    if (typeStr == "NONE") return WEAPON_NONE;
    
    #define DoWeapon(weapon_name, json_id) \
        if (typeStr == #weapon_name) return WEAPON_##weapon_name; 
    #include "weapons.inc"
    #undef DoWeapon

    return WEAPON_NONE;
}

void loadWeaponsFromJson(JsonParser* jsonParser)
{
    #define DoWeapon(weapon_name, json_id) \
        _loadWeapon(jsonParser, g_weaponData[WEAPON_##weapon_name], json_id, TextureId::weapon_name);
    #include "weapons.inc"
    #undef DoWeapon
}

//Placeholder callback
void WeaponCallback_devilsBow()
{
    printf("Devils bow fired!\n");
}