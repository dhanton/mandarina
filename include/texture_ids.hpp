#pragma once

#include "defines.hpp"

//we use a namespace instead of enum class
//to be able to automatically convert to u16

namespace TextureId 
{
    enum _TextureId {
        TEST_TILESET,

        //textures used by units
        #define LoadUnit(unit_name, texture_id, json_filename, weapon_id) \
            texture_id,
        #include "units.inc"
        #undef LoadUnit

        //textures used by weapons
        #define LoadWeapon(weapon_name, callback_name, json_file) \
            weapon_name,
        #include "weapons.inc"
        #undef LoadWeapon

        //textures used by projectiles
        #define LoadProjectile(projectile_name, texture_id, json_file) \
            texture_id,
        #include "projectiles.inc"
        #undef LoadProjectile
    };
}
