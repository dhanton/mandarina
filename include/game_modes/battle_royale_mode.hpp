#pragma once

#include <vector>
#include "game_mode.hpp"

class BattleRoyaleMode : public GameMode
{
public:
    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void packGameEndData(CRCPacket& outPacket);
    virtual void loadGameEndData(CRCPacket& inPacket);

    virtual void drawGameEndInfo(sf::RenderWindow& window, const FontLoader* fonts);

    virtual void onGameStarted(u8 numberOfPlayers);
    virtual void onUpdate(sf::Time eTime);
    virtual void onHeroCreated(Unit* controlledUnit);
    virtual void onHeroDeath(Unit* hero, bool& dead);

    u8 getWinnerTeamId() const;

private:
    u8 m_playersPerTeam;

    u8 m_numberOfTeamsRemaining;
    std::vector<bool> m_teamsEliminated;

    u8 m_winnerTeamId;

    bool m_spawnPointsLoaded;
    std::list<Vector2> m_spawnPoints;

    //@WIP: Add teams and team respawn if at least one player of the team is alive
};
