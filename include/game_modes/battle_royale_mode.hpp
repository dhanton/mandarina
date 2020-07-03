#pragma once

//@storm_remove: removed when storm implemented as a tile
#include <SFML/Graphics/VertexArray.hpp>

#include <SFML/Graphics/Color.hpp>
#include <vector>
#include "game_mode.hpp"

class BattleRoyaleMode : public GameMode
{
public:
    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void packGameEndData(CRCPacket& outPacket);
    virtual void loadGameEndData(CRCPacket& inPacket);

    //@storm_remove: remove this when the storm gets implemented with tiles
    virtual void draw(sf::RenderTexture& renderTexture, const TextureLoader* textures);

    virtual void drawGameEndInfo(sf::RenderWindow& window, const FontLoader* fonts);

    virtual void onGameStarted(u8 numberOfPlayers, const ManagersContext& context);
    virtual void C_onGameStarted();

    virtual void onUpdate(sf::Time eTime);
    virtual void C_onUpdate(sf::Time eTime);

    virtual void onHeroCreated(Hero* hero);
    virtual void onHeroDeath(Hero* hero, bool& dead);
    virtual void onUnitUpdate(Unit* unit);

    u8 getWinnerTeamId() const;

private:
    void addStormBuff(Unit* unit);

    void resetStorm(const Vector2u& mapSize, u16 tileSize, u16 tileScale);
    void updateStorm(sf::Time eTime);
    void moveToNextTile();
    bool atStorm(size_t i, size_t j);

    //@storm_remove
    void _setTileTextureCoords(size_t i, size_t j);

private:
    u8 m_playersPerTeam;

    float m_stormDmgPerSecond;
    sf::Time m_stormTimeTreshold;
    float m_stormDamageMultiplier;

    u8 m_numberOfTeamsRemaining;
    std::vector<Hero*> m_heroes;

    u8 m_winnerTeamId;
    std::string m_winnerDisplayName;

    std::string m_spawnPointsFilename;
    bool m_spawnPointsLoaded;
    std::list<Vector2> m_spawnPoints;

    //@storm_remove: 
    //@WIP: Storm is probably better implemented as a Tile (TILE_STORM), add in a new layer in renderer
    //(do it when tiles can be destroyed and are updated to client)
    std::vector<std::vector<bool>> m_stormTiles;
    Vector2u m_stormSize;
    u16 m_tileSize;

    Vector2u m_currentTile;
    std::vector<Vector2i> m_stormPatterns;
    size_t m_currentPattern;

    bool m_stormFull;
    float m_stormSpeed;
    sf::Time m_stormTimer;

    //@storm_remove: removed when implemented as a Tile
    sf::VertexArray m_stormVertices;

    //@WIP: Add teams and team respawn if at least one player of the team is alive
};
