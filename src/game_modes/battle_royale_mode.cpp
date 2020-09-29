#include "game_modes/battle_royale_mode.hpp"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include "tilemap.hpp"
#include "texture_ids.hpp"
#include "res_loader.hpp"
#include "server_entity_manager.hpp"
#include "buffs/storm_buff.hpp"

void BattleRoyaleMode::loadFromJson(const rapidjson::Document& doc)
{
    GameMode::loadFromJson(doc);

    m_playersPerTeam = doc["players_per_team"].GetUint();

    m_stormSpeed = doc["storm_tiles_per_second"].GetFloat();
    m_stormDmgPerSecond = doc["storm_damage_per_second"].GetFloat();
    
    if (doc.HasMember("storm_time_treshold")) {
        m_stormTimeTreshold = sf::seconds(doc["storm_time_treshold"].GetFloat());
    } else {
        m_stormTimeTreshold = sf::Time::Zero;
    }

    if (doc.HasMember("storm_damage_multiplier")) {
        m_stormDamageMultiplier = doc["storm_damage_multiplier"].GetFloat();
    } else {
        m_stormDamageMultiplier = 1.f;
    }

    m_spawnPointsFilename = m_mapFilename + "_spawnpoints";
    m_spawnPointsLoaded = false;
    m_numberOfTeamsRemaining = 0;

    m_heroes.resize(getMaxPlayers() + 1, nullptr);

    //maybe this could be loaded from json file?
    m_stormPatterns = {Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0), Vector2i(0, -1)};
    m_currentPattern = 0;

    m_winnerTeamId = 0;
    m_stormFull = false;
}

void BattleRoyaleMode::packGameEndData(CRCPacket& outPacket)
{
    GameMode::packGameEndData(outPacket);

    outPacket << m_winnerTeamId << m_winnerDisplayName;
}

void BattleRoyaleMode::loadGameEndData(CRCPacket& inPacket)
{
    GameMode::loadGameEndData(inPacket);

    inPacket >> m_winnerTeamId >> m_winnerDisplayName;
}

void BattleRoyaleMode::draw(sf::RenderTexture& renderTexture, const TextureLoader* textures)
{
    renderTexture.draw(m_stormVertices, &textures->getResource(TextureId::STORM));
}

void BattleRoyaleMode::drawGameEndInfo(sf::RenderTarget& target, const FontLoader* fonts)
{
    const Vector2u windowSize = target.getSize();

    sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    sf::Text winnerText;
    winnerText.setFont(fonts->getResource("keep_calm_font"));
    winnerText.setCharacterSize(70);
    winnerText.setString("The winner is " + m_winnerDisplayName + "!");
    winnerText.setFillColor(sf::Color::White);
    winnerText.setOutlineColor(sf::Color::Black);
    winnerText.setOutlineThickness(2.f);
    winnerText.setOrigin(winnerText.getLocalBounds().width/2.f + winnerText.getLocalBounds().left,
                         winnerText.getLocalBounds().height/2.f + winnerText.getLocalBounds().top);

    float yOffset = 1.5f * winnerText.getLocalBounds().height;
    winnerText.setPosition({windowSize.x/2.f, windowSize.y/2.f - yOffset});

    target.draw(winnerText);

    target.setView(previousView);
}

void BattleRoyaleMode::onGameStarted(u8 numberOfPlayers, const ManagersContext& context)
{
    GameMode::onGameStarted(numberOfPlayers, context);

    m_numberOfTeamsRemaining = numberOfPlayers;

    resetStorm(m_tileMap->getSize(), m_tileMap->getTileSize(), m_tileMap->getTileScale());

    //load crates
    std::list<Vector2> crates = m_tileMap->loadSpawnPoints(m_spawnPointsFilename, sf::Color::Blue);

    for (const auto& cratePos : crates) {
        context.entityManager->createEntity(ENTITY_NORMAL_CRATE, cratePos, 0);
    }
}

void BattleRoyaleMode::C_onGameStarted()
{
    GameMode::C_onGameStarted();

    resetStorm(m_tileMap->getSize(), m_tileMap->getTileSize(), m_tileMap->getTileScale());

    // @storm_remove
    m_stormVertices.clear();
    m_stormVertices.setPrimitiveType(sf::Quads);
    m_stormVertices.resize(m_stormSize.x * m_stormSize.y * 4);

    for (int i = 0; i < m_stormSize.x; ++i) {
        for (int j = 0; j < m_stormSize.y; ++j) {
            sf::Vertex *quad = &m_stormVertices[(i + j * m_stormSize.x) * 4];

            quad[0].position = Vector2(i * m_tileSize, j * m_tileSize);
            quad[1].position = Vector2((i + 1) * m_tileSize, j * m_tileSize);
            quad[2].position = Vector2((i + 1) * m_tileSize, (j + 1) * m_tileSize);
            quad[3].position = Vector2(i * m_tileSize, (j + 1) * m_tileSize);
        }
    }
}

void BattleRoyaleMode::onUpdate(sf::Time eTime)
{
    updateStorm(eTime);
}

void BattleRoyaleMode::C_onUpdate(sf::Time eTime)
{
    updateStorm(eTime);
}

void BattleRoyaleMode::onHeroCreated(Hero* hero)
{
    GameMode::onHeroCreated(hero);

    if (!hasGameStarted()) return;

    if (!m_spawnPointsLoaded) {
        m_spawnPoints = m_tileMap->loadSpawnPoints(m_spawnPointsFilename, sf::Color::Yellow);
        m_spawnPointsLoaded = true;
    }

    //@TODO: Team position is a bit more complicated
    //Should probably add a method to spawn all members close a specific point but without touching
    //This general method could be useful for certain spells as well

    if (!m_spawnPoints.empty()) {
        auto it = std::next(m_spawnPoints.begin(), rand() % m_spawnPoints.size());

        //move the unit randomly to one of the spawn points
        hero->setPosition(*it);
        m_spawnPoints.erase(it);

    } else {
        std::cout << "BattleRoyaleMode::onHeroCreated error - Not enough spawn points" << std::endl;
    }

    m_heroes[hero->getTeamId() + m_playersPerTeam - 1] = hero;
}

void BattleRoyaleMode::onHeroDeath(Hero* hero, bool& dead)
{
    if (m_gameEnded) return;

    //@TODO: Generalize this when playersPerTeam != 1

    if (m_playersPerTeam == 1) {
        m_numberOfTeamsRemaining--;
        m_heroes[hero->getTeamId()] = nullptr;

        HeroData data;

        data.uniqueId = hero->getUniqueId();
        data.killerUniqueId = hero->getLatestDamageDealer();
        data.killerTeamId = hero->getLatestDamageDealerTeamId();
        data.teamEliminated = true;

        m_deadHeroes.push_back(data);

        if (m_numberOfTeamsRemaining == 1) {
            m_gameEnded = true;

            for (int i = 0; i < m_heroes.size(); ++i) {
                if (m_heroes[i]) {
                    m_winnerTeamId = i;
                    m_winnerDisplayName = m_heroes[i]->getDisplayName();
                    break;
                }
            }
        }
    }
}

void BattleRoyaleMode::onUnitUpdate(Unit *unit)
{
    if (!hasGameStarted()) return;

    //@storm_remove
    //copied from TileMap::_colliding_impl

    Circlef circle = Circlef(unit->getPosition(), unit->getCollisionRadius());

    Vector2i min = {(int) (circle.center.x - circle.radius)/m_tileSize, (int) (circle.center.y - circle.radius)/m_tileSize};
    Vector2i max = {(int) (circle.center.x + circle.radius)/m_tileSize, (int) (circle.center.y + circle.radius)/m_tileSize};

    if (min.x < 0 || min.x >= m_stormSize.x) return;
    if (min.y < 0 || min.y >= m_stormSize.y) return;
    if (max.x < 0 || max.x >= m_stormSize.x) return;
    if (max.y < 0 || max.y >= m_stormSize.y) return;

    sf::FloatRect rect;

    rect.height = m_tileSize;
    rect.width = m_tileSize;

    for (size_t i = min.x; i <= max.x; ++i) {
        for (size_t j = min.y; j <= max.y; ++j) {
            rect.left = i * m_tileSize;
            rect.top = j * m_tileSize;

            if (m_stormTiles[i][j] && circle.intersects(rect)) {
                addStormBuff(unit);
                return;
            }
        }
    }

    unit->removeUniqueBuff(BUFF_STORM);
}

u8 BattleRoyaleMode::getWinnerTeamId() const
{
    return m_winnerTeamId;
}

void BattleRoyaleMode::addStormBuff(Unit* unit)
{
    StormBuff* buff = static_cast<StormBuff*>(unit->addUniqueBuff(BUFF_STORM));

    if (buff) {
        buff->setDamagePerSecond(m_stormDmgPerSecond);
        buff->setMultiplierTimeTreshold(m_stormTimeTreshold);
        buff->setDamageMultiplier(m_stormDamageMultiplier);
    }
}

void BattleRoyaleMode::resetStorm(const Vector2u &mapSize, u16 tileSize, u16 tileScale)
{
    m_stormTiles.clear();
    m_stormTiles.resize(mapSize.x, std::vector<bool>(mapSize.y, false));
    m_stormSize = mapSize;
    m_tileSize = tileSize * tileScale;
}

void BattleRoyaleMode::updateStorm(sf::Time eTime)
{
    if (m_stormTiles.empty() || m_stormFull)
        return;

    sf::Time timeToNext = sf::seconds(1 / m_stormSpeed);

    m_stormTimer += eTime;

    if (m_stormTimer >= timeToNext) {
        m_stormTimer -= timeToNext;

        m_stormTiles[m_currentTile.x][m_currentTile.y] = true;

        //this will only execute on client
        _setTileTextureCoords(m_currentTile.x, m_currentTile.y);

        moveToNextTile();
    }
}

void BattleRoyaleMode::moveToNextTile()
{
    //traverse all patterns starting with the current one
    for (int i = m_currentPattern; i < m_currentPattern + m_stormPatterns.size(); ++i) {
        int index = i % m_stormPatterns.size();

        Vector2i nextTile = Vector2i(m_currentTile.x, m_currentTile.y) + m_stormPatterns[index];
        bool onLimits = nextTile.x >= 0 && nextTile.y >= 0 && nextTile.x < m_stormSize.x && nextTile.y < m_stormSize.y;

        //if current pattern is good, use it
        //otherwise move to the next one
        if (onLimits && !atStorm(nextTile.x, nextTile.y)) {
            m_currentTile = {nextTile.x, nextTile.y};
            m_currentPattern = index;
            return;
        }
    }

    //if there are no tiles left to go, the storm covers everything
    m_stormFull = false;
}

bool BattleRoyaleMode::atStorm(size_t i, size_t j)
{
    return m_stormTiles[i][j];
}

void BattleRoyaleMode::_setTileTextureCoords(size_t i, size_t j)
{
    if (m_stormVertices.getVertexCount() == 0) return;

    sf::Vertex *quad = &m_stormVertices[(i + j * m_stormSize.x) * 4];

    //hardcoded for temporary storm
    size_t texSize = 62;

    quad[0].texCoords = Vector2(0, 0);
    quad[1].texCoords = Vector2(texSize, 0);
    quad[2].texCoords = Vector2(texSize, texSize);
    quad[3].texCoords = Vector2(0, texSize);
}
