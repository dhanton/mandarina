#include "game_modes/battle_royale_mode.hpp"

#include <SFML/Graphics/Text.hpp>
#include "tilemap.hpp"

void BattleRoyaleMode::loadFromJson(const rapidjson::Document& doc)
{
    GameMode::loadFromJson(doc);

    m_playersPerTeam = doc["players_per_team"].GetUint();
    m_spawnPointsLoaded = false;
    m_numberOfTeamsRemaining = 0;

    m_teamsEliminated.resize(getMaxPlayers() + 1, false);

    //for simplicity we add the neutral team and set it to be eliminated from the start
    m_teamsEliminated[0] = true;

    m_winnerTeamId = 0;
}

void BattleRoyaleMode::packGameEndData(CRCPacket& outPacket)
{
    GameMode::packGameEndData(outPacket);

    outPacket << m_winnerTeamId;
    //@WIP: Send winner's display name as well
}

void BattleRoyaleMode::loadGameEndData(CRCPacket& inPacket)
{
    GameMode::loadGameEndData(inPacket);

    inPacket >> m_winnerTeamId;
}

void BattleRoyaleMode::drawGameEndInfo(sf::RenderWindow& window, const FontLoader* fonts)
{
    const Vector2u windowSize = window.getSize();

    sf::View previousView = window.getView();
    window.setView(window.getDefaultView());

    sf::Text winnerText;
    winnerText.setFont(fonts->getResource("keep_calm_font"));
    winnerText.setCharacterSize(70);
    //@WIP: Display winner name as well
    winnerText.setString("The winner is team " + std::to_string((int) m_winnerTeamId) + "!");
    winnerText.setFillColor(sf::Color::White);
    winnerText.setOutlineColor(sf::Color::Black);
    winnerText.setOutlineThickness(2.f);
    winnerText.setOrigin(winnerText.getLocalBounds().width/2.f + winnerText.getLocalBounds().left, 
                            winnerText.getLocalBounds().height/2.f + winnerText.getLocalBounds().top);

    float yOffset = 1.5f * winnerText.getLocalBounds().height;
    winnerText.setPosition({windowSize.x/2.f, windowSize.y/2.f - yOffset});
    
    window.draw(winnerText);

    window.setView(previousView);
}

void BattleRoyaleMode::onGameStarted(u8 numberOfPlayers)
{
    GameMode::onGameStarted(numberOfPlayers);

    m_numberOfTeamsRemaining = numberOfPlayers;
}

void BattleRoyaleMode::onUpdate(sf::Time eTime)
{
    //Check if there is only one team remaining
    //Update the storm
}

void BattleRoyaleMode::onHeroCreated(Unit* controlledUnit)
{
    GameMode::onHeroCreated(controlledUnit);

    if (!hasGameStarted()) return;

    if (!m_spawnPointsLoaded) {
        m_spawnPoints = m_tileMap->loadSpawnPoints(m_mapFilename + "_spawnpoints");
        m_spawnPointsLoaded = true;
    }

    if (!m_spawnPoints.empty()) {
        auto it = std::next(m_spawnPoints.begin(), rand() % m_spawnPoints.size());

        //move the unit randomly to one of the spawn points
        controlledUnit->setPosition(*it);
        m_spawnPoints.erase(it);

    } else {
        std::cout << "BattleRoyaleMode::onHeroCreated error - Not enough spawn points" << std::endl;
    }

    //@TODO: Team position is a bit more complicated
    //Should probably add a method to spawn all members close a specific point but without touching
    //This general method could be useful for certain spells as well
}

void BattleRoyaleMode::onHeroDeath(Unit* hero, bool& dead)
{
    if (m_gameEnded) return;

    if (m_playersPerTeam == 1) {
        m_numberOfTeamsRemaining--;
        m_teamsEliminated[hero->getTeamId()] = true;

        HeroData data;

        data.uniqueId = hero->getUniqueId();
        data.killerUniqueId = hero->getLatestDamageDealer();
        data.killerTeamId = hero->getLatestDamageDealerTeamId();
        data.teamEliminated = true;

        m_deadHeroes.push_back(data);

        if (m_numberOfTeamsRemaining == 1) {
            m_gameEnded = true;
            
            for (int i = 0; i < m_teamsEliminated.size(); ++i) {
                if (!m_teamsEliminated[i]) {
                    m_winnerTeamId = i;
                    //@WIP: save its display name as well

                    break;
                }
            }
        }

    } else {
        //@TODO: Teams
    }
}

u8 BattleRoyaleMode::getWinnerTeamId() const
{
    return m_winnerTeamId;
}
