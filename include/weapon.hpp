#pragma once

#include "defines.hpp"

class JsonParser;

struct Weapon {
    u8 primaryFire;
    u8 secondaryFire;
    float scale;
    float angleOffset;
    u16 textureId;
};

enum WeaponType {
    WEAPON_NONE,

    #define LoadWeapon(weapon_name, callback_func, json_id, primary_fire_id, secondary_fire_id) \
        WEAPON_##weapon_name,
    #include "weapons.inc"
    #undef LoadWeapon

    WEAPON_MAX_TYPES
};

extern Weapon g_weaponData[WEAPON_MAX_TYPES];

void loadWeaponsFromJson(JsonParser* jsonParser);

void WeaponCallback_devilsBow();
