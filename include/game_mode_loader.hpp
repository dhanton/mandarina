#pragma once

#include "game_mode.hpp"

namespace GameModeLoader 
{
    GameMode* create(u8 gameModeType, const Context& context);
}
