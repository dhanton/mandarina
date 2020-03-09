#pragma once

#include <string>
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

    #define DoWeapon(weapon_name, json_id) \
        WEAPON_##weapon_name,
    #include "weapons.inc"
    #undef DoWeapon

    WEAPON_MAX_TYPES
};

extern Weapon g_weaponData[WEAPON_MAX_TYPES];

u8 Weapon_stringToType(const std::string& typeStr);

void loadWeaponsFromJson(JsonParser* jsonParser);
