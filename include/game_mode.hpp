#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>
#include "context.hpp"
#include "json_parser.hpp"
#include "defines.hpp"
#include "managers_context.hpp"
#include "unit.hpp"

enum GameModeType {
    #define DoGameMode(class_name, type, json_id) \
        GAME_MODE_##type,
    #include "game_modes.inc"
    #undef DoGameMode

    GAME_MODE_MAX_TYPES
};

class GameMode
{
public:
    struct HeroData {
        u32 uniqueId = 0;
        u8 teamId = 0;

        u32 killerUniqueId = 0;
        u8 killerTeamId = 0;

        bool teamEliminated = false;
    };

public:
    void setTileMap(TileMap* tileMap);

    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void packGameEndData(CRCPacket& outPacket);
    virtual void loadGameEndData(CRCPacket& inPacket);

    //remove this when the storm gets implemented with tiles
    virtual void draw(sf::RenderTexture& renderTexture, const TextureLoader* textures);

    //we don't use drawable since this class is shared by client and server
    virtual void drawGameEndInfo(sf::RenderWindow& window, const FontLoader* fonts);

    //creates chests and other game mode specific entities
    //(called after heroes are created)
    //prepares the storm
    virtual void onGameStarted(u8 numberOfPlayers);

    //only prepares the storm
    virtual void C_onGameStarted();

    //can update special events (like the storm for battle royale)
    //will also check if the game win condition has been met
    //in which case the game will end with a command for all clients
    virtual void onUpdate(sf::Time eTime);

    virtual void C_onUpdate(sf::Time eTime);

    //called when a hero is created (when player joins game or when game starts)
    //moves the unit to its initial position and sets its teamId if needed
    virtual void onHeroCreated(Unit* controlledUnit);

    //will keep track of players to respawn them or 
    //remove them completely from the game
    virtual void onHeroDeath(Unit* hero, bool& dead);

    //called by units every update
    virtual void onUnitUpdate(Unit* unit);

    //assign recharge for abilities that require it
    //calculate hero damage multiplier based on XP or something

    std::list<HeroData>& getNewDeadHeroes();

    u8 getType() const;
    void setType(u8 type);

    std::string getLobbyMapFilename() const;
    void setLobbyMapFilename(const std::string& mapFilename);

    std::string getMapFilename() const;
    void setMapFilename(const std::string& mapFilename);

    //this is called first, just after all entities/projectiles are removed
    void startGame();

    bool hasGameStarted() const;
    bool hasGameEnded() const;

    u8 getMaxPlayers() const;
    u8 getRequiredPlayers() const;
    bool canJoinMidMatch() const;

    float getDamageMultiplier() const;
    float getAbilityTimeMultiplier() const;
    float getRechargeMultiplier() const;

protected:
    TileMap* m_tileMap;
    
    //@TODO: There should be an option to manually select a tilemap from 
    //those that are available for a specific game mode
    std::string m_lobbyMapFilename;
    std::string m_mapFilename;

    bool m_gameEnded;

    std::list<HeroData> m_deadHeroes;

private:
    u8 m_type;

    bool m_gameStarted;

    u8 m_maxPlayers;
    u8 m_requiredPlayers;
    bool m_canJoinMidMatch;
    u8 m_lastTeamId;

    float m_damageMultiplier;
    float m_lobbyDamageMultiplier;

    //this multiplier affects time-based recharge
    float m_abilityTimeMultiplier;
    float m_lobbyAbilityTimeMultiplier;

    //this multplier affects only ability-based recharge (like damage, stun, heal, etc)
    float m_rechargeMultiplier;
    float m_lobbyRechargeMultiplier;
};
