#include "game_mode_loader.hpp"

//all modes have to be included for the loading to work
#include "game_modes/battle_royale_mode.hpp"

//There's no need to create an array to hold all types and then use clone()
//since game there's only game mode per game

namespace GameModeLoader 
{

GameMode* create(u8 gameModeType, const Context& context)
{
    if (gameModeType >= GAME_MODE_MAX_TYPES) {
        throw std::runtime_error("GameModeLoader::create error - Invalid type");
    }

    GameMode* gameMode = nullptr;

    #define DoGameMode(class_name, type, json_id) \
        if (gameModeType == GAME_MODE_##type) { \
            gameMode = new class_name(); \
            gameMode->loadFromJson(*context.jsonParser->getDocument(json_id)); \
        }
    #include "game_modes.inc"
    #undef DoGameMode

    return gameMode;
}

}
