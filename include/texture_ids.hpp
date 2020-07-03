#pragma once

#include "defines.hpp"

//we use a namespace instead of enum class
//to be able to automatically convert to u16

namespace TextureId 
{
    enum _TextureId {
        TEST_TILESET,

        CROSSHAIR,

        //textures used to display buffs in client
        STUNNED,
        SILENCED,
        DISARMED,
        ROOTED,
        SLOWED,

        //storm modifier
        STORM,

        //textures used by entities
        #define DoEntity(class_name, type, json_id) \
            type,
        #include "entities.inc"
        #undef DoEntity

        //textures used by weapons
        #define DoWeapon(weapon_name, json_file) \
            weapon_name,
        #include "weapons.inc"
        #undef DoWeapon

        //textures used by projectiles
        #define DoProjectile(projectile_name, json_file) \
            projectile_name,
        #include "projectiles.inc"
        #undef DoProjectile

        //textures used by abilities (icons)
        #define DoAbility(class_name, type, json_id) \
            ICON_##type,
        #include "abilities.inc"
        #undef DoAbility
    };
}
