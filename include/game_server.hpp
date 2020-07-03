#pragma once

#include <steam/steamnetworkingsockets.h>
#include <SFML/System/Time.hpp>
#include <unordered_map>

#include "paths.hpp"
#include "context.hpp"
#include "bucket.hpp"
#include "net_peer.hpp"

#include "server_entity_manager.hpp"
#include "collision_manager.hpp"

#include "tilemap.hpp"
#include "game_mode.hpp"

class GameServer;

struct GameServerCallbacks : public ISteamNetworkingSocketsCallbacks
{
    GameServerCallbacks(GameServer* p);
    virtual ~GameServerCallbacks() {}

    virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info) override;

    GameServer* parent;
};

class GameServer : public InContext, public NetPeer
{
public:
    friend struct GameServerCallbacks;

    struct ClientInfo {
        u32 uniqueId = 0;
        HSteamNetConnection connectionId = k_HSteamNetConnection_Invalid;

        u32 snapshotId = 0;
        bool forceFullUpdate = false;

        sf::Time snapshotRate;
        u32 latestInputId = 0;
        sf::Time inputRate;
        u8 inputsSent = 0; //this update

        u8 teamId = 0;
        u32 controlledEntityUniqueId = 0;
        std::string displayName;

        u8 spectatingTeamId = 0;
        u32 spectatingUniqueId = 0;
        bool heroDead = true;

        //EntityType of hero selected (before/after matchmaking)
        u8 selectedHeroType = ENTITY_MAX_TYPES;

        int ping = -1;
    };

    //data we're not using as much
    //@TODO: Should we use this??
    struct ClientInfo_cold {
        //display name
        //cosmetics??
    };

    struct Snapshot {
        EntityManager entityManager;
        u32 id = 0;
        sf::Time worldTime;
    };

public:
    GameServer(const Context& context, u8 gameModeType);
    ~GameServer();

    void mainLoop(bool& running);

    void receiveLoop();
    void update(const sf::Time& eTime, bool& running);
    void sendSnapshots();

    void processPacket(HSteamNetConnection connectionId, CRCPacket& packet);
    void handleCommand(u8 command, int index, CRCPacket& packet);
    void onConnectionCompleted(HSteamNetConnection connectionId);

    Hero* createClientHeroEntity(int index, bool keepOldUniqueId = false);
    void handleDeadHeroes();

    int addClient();
    bool canNewClientConnect() const;
    
    bool addClientToPoll(int index);
    int getIndexByConnectionId(HSteamNetConnection connectionId) const;

    void createGameMode(u8 gameModeType);
    std::string getCurrentMapFilename() const;

    void loadFromJson(const rapidjson::Document& doc);

private:
    GameServerCallbacks m_gameServerCallbacks;

    PollId m_pollId;
    SteamNetworkingIPAddr m_endpoint;

    bool m_gameStarted;

    sf::Time m_updateRate;
    sf::Time m_snapshotRate;
    sf::Time m_defaultInputRate;
    
    bool m_canClientsChangeInputRate;
    bool m_canClientsChangeSnapshotRate;
    sf::Time m_maxInputRate;
    sf::Time m_minInputRate;
    sf::Time m_maxSnapshotRate;
    sf::Time m_minSnapshotRate;
    sf::Time m_maxPingCorrection;

    //used to safely shutdown the server with Ctrl+C
    static bool SIGNAL_SHUTDOWN;

    Bucket<ClientInfo> m_clients;
    u32 m_lastClientId;

    std::unordered_map<u32, Snapshot> m_snapshots;
    EntityManager m_entityManager;
    u32 m_lastSnapshotId;

    CollisionManager m_collisionManager;

    TileMap m_tileMap;

    sf::Time m_worldTime;

    std::unique_ptr<GameMode> m_gameMode;

    bool m_gameEnded;
    sf::Time m_gameEndLingeringTime;
    sf::Time m_gameEndTimer;
};
