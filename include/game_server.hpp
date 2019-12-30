#pragma once

#include <steam/steamnetworkingsockets.h>
#include <SFML/System/Time.hpp>
#include <unordered_map>

#include "context.hpp"
#include "bucket.hpp"
#include "net_peer.hpp"

#include "server_entity_manager.hpp"

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
        u32 clientId = 0;
        HSteamNetConnection connectionId = k_HSteamNetConnection_Invalid;
        bool isReady = false;
        u32 snapshotId = 0;
    };

    struct ClientInfo_cold {
        u8 teamId = 0;
        //display name as well
        //other data that we're not gonna use that much
    };

    struct Snapshot {
        EntityManager entityManager;
        u32 id = 0;
        sf::Time worldTime;
    };

public:
    GameServer(const Context& context, int partyNumber);
    ~GameServer();

    void receiveLoop();
    void update(const sf::Time& eTime, bool& running);
    void sendSnapshots();

    void processPacket(HSteamNetConnection connectionId, CRCPacket& packet);
    void handleCommand(u8 command, int index, CRCPacket& packet);

    int addClient();
    
    bool addClientToPoll(int index);
    int getIndexByConnectionId(HSteamNetConnection connectionId) const;
    bool isIndexValid(int index) const;

private:
    GameServerCallbacks m_gameServerCallbacks;

    PollId m_pollId;
    SteamNetworkingIPAddr m_endpoint;

    bool m_gameStarted;

    //Number of party members (clients) required to start the game
    int m_partyNumber;

    //used to safely shutdown the server with Ctrl+C
    static bool SIGNAL_SHUTDOWN;

    Bucket<ClientInfo> m_clients;
    u32 m_lastClientId;

    std::unordered_map<u32, Snapshot> m_snapshots;
    EntityManager m_entityManager;
    u32 m_lastSnapshotId;

    sf::Time m_worldTime;

    //initial size of the clients vector
    //will grow to accomodate more if needed
    const int INITIAL_CLIENTS_SIZE;
};
