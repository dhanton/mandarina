#include "game_mode.hpp"

#include "server_entity_manager.hpp"

void GameMode::setManagersContext(const ManagersContext& managersContext)
{
    m_tileMap = managersContext.tileMap;
}

void GameMode::loadFromJson(const rapidjson::Document& doc)
{
    m_gameStarted = false;
    m_gameEnded = false;
    m_lastTeamId = 0;

    m_maxPlayers = doc["max_players"].GetUint();

    if (doc.HasMember("required_players")) {
        m_requiredPlayers = doc["required_players"].GetUint();
    } else {
        m_requiredPlayers = m_maxPlayers;
    }

    if (doc.HasMember("can_join_mid_match")) {
        m_canJoinMidMatch = doc["can_join_mid_match"].GetBool();
    } else {
        m_canJoinMidMatch = false;
    }

    //Damage multiplier
    if (doc.HasMember("damage_multiplier")) {
        m_damageMultiplier = doc["damage_multiplier"].GetFloat();
    } else {
        m_damageMultiplier = 1.f;
    }

    if (doc.HasMember("lobby_damage_multiplier")) {
        m_lobbyDamageMultiplier = doc["lobby_damage_multiplier"].GetFloat();
    } else {
        //by default players can't damage each other in the lobby
        m_lobbyDamageMultiplier = 0.f;
    }

    //Time multiplier (cooldown, recharge, etc)
    if (doc.HasMember("ability_time_multiplier")) {
        m_abilityTimeMultiplier = doc["ability_time_multiplier"].GetFloat();
    } else {
        m_abilityTimeMultiplier = 1.f;
    }

    if (doc.HasMember("lobby_ability_time_multiplier")) {
        m_lobbyAbilityTimeMultiplier = doc["lobby_ability_time_multiplier"].GetFloat();
    } else {
        m_lobbyAbilityTimeMultiplier = 2.5f;
    }

    //Other recharge multiplier
    if (doc.HasMember("recharge_multiplier")) {
        m_rechargeMultiplier = doc["recharge_multiplier"].GetFloat();
    } else {
        m_rechargeMultiplier = 1.f;
    }

    if (doc.HasMember("lobby_recharge_multiplier")) {
        m_lobbyRechargeMultiplier = doc["lobby_recharge_multiplier"].GetFloat();
    } else {
        m_lobbyRechargeMultiplier = 2.5f;
    }

    m_lobbyMapFilename = doc["lobby_map_filename"].GetString();
    m_mapFilename = doc["map_filename"].GetString();
}

void GameMode::packGameEndData(CRCPacket& outPacket)
{

}

void GameMode::loadGameEndData(CRCPacket& outPacket)
{
    m_gameEnded = true;
}

void GameMode::drawGameEndInfo(sf::RenderWindow& window, const FontLoader* fonts)
{

}

void GameMode::onUpdate(sf::Time eTime)
{

}

void GameMode::onGameStarted(u8 numberOfPlayers)
{
    //game mode specific entities can be created by child classes
}

void GameMode::onHeroCreated(Unit* controlledUnit)
{
    if (m_gameStarted) {
        //@TODO: Server should forward some data to GameMode regarding matchmaking parties, mmr, etc
        //so that the game mode can create teams appropiately

    } else {
        controlledUnit->setSolid(false);
        controlledUnit->setPosition({100.f, 100.f});

        //each player is their own team in the lobby
        controlledUnit->setTeamId(++m_lastTeamId);
    }
}

void GameMode::onHeroDeath(Unit* hero, bool& dead)
{

}

float GameMode::getDamageMultiplier() const
{
    return m_gameStarted ? m_damageMultiplier : m_lobbyDamageMultiplier;
}

float GameMode::getAbilityTimeMultiplier() const
{
    return m_gameStarted ? m_abilityTimeMultiplier : m_lobbyAbilityTimeMultiplier;
}

float GameMode::getRechargeMultiplier() const
{
    return m_gameStarted ? m_rechargeMultiplier : m_lobbyRechargeMultiplier;
}

std::list<GameMode::HeroData>& GameMode::getNewDeadHeroes()
{
    return m_deadHeroes;
}

u8 GameMode::getType() const
{
    return m_type;
}

void GameMode::setType(u8 type)
{
    m_type = type;
}

std::string GameMode::getLobbyMapFilename() const
{
    return m_lobbyMapFilename;
}

void GameMode::setLobbyMapFilename(const std::string& mapFilename)
{
    m_lobbyMapFilename = mapFilename;
}

std::string GameMode::getMapFilename() const
{
    return m_mapFilename;
}

void GameMode::setMapFilename(const std::string& mapFilename)
{
    m_mapFilename = mapFilename;
}

void GameMode::startGame()
{
    m_gameStarted = true;
}

bool GameMode::hasGameStarted() const
{
    return m_gameStarted;
}

bool GameMode::hasGameEnded() const
{
    return m_gameEnded;
}

u8 GameMode::getMaxPlayers() const
{
    return m_maxPlayers;
}

u8 GameMode::getRequiredPlayers() const
{
    return m_requiredPlayers;
}

bool GameMode::canJoinMidMatch() const
{
    return m_canJoinMidMatch;
}
