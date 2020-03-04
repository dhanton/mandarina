#include "weapon.hpp"

#include "json_parser.hpp"
#include "texture_ids.hpp"
#include "ability.hpp"

Weapon g_weaponData[WEAPON_MAX_TYPES];

void _loadWeapon(JsonParser* jsonParser, Weapon& weapon, const char* json_id, u16 textureId, u8 primaryFire, u8 secondaryFire)
{
    auto* doc = jsonParser->getDocument(json_id);

    weapon.textureId = textureId;
    weapon.primaryFire = primaryFire;
    weapon.secondaryFire = secondaryFire;

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

void loadWeaponsFromJson(JsonParser* jsonParser)
{
    #define LoadWeapon(weapon_name, callback_func, json_id, primary_fire_id, secondary_fire_id) \
        _loadWeapon(jsonParser, g_weaponData[WEAPON_##weapon_name], json_id, TextureId::weapon_name, primary_fire_id, secondary_fire_id); \
        // g_weaponData[WEAPON_##weapon_name].callback = &WeaponCallback_##callback_func;
    #include "weapons.inc"
    #undef LoadWeapon
}

//Placeholder callback
void WeaponCallback_devilsBow()
{
    printf("Devils bow fired!\n");
}