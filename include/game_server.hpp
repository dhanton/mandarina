#pragma once

#include <steam/steamnetworkingsockets.h>
#include <SFML/System/Time.hpp>

#include "context.hpp"
#include "net_peer.hpp"

// #include "entity_manager.hpp"

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

public:
    GameServer(const Context& context, int partyNumber);
    ~GameServer();

    void receiveLoop();
    void update(const sf::Time& eTime, bool& running);

    void processPacket(HSteamNetConnection connectionId, CRCPacket* packet);
    void handleCommand(u8 command, int index, CRCPacket* packet);

    int addClient();
    void resetClient(int index);
    void removeClient(int index);
    

    bool addClientToPoll(int index);
    int getIndexByConnectionId(HSteamNetConnection connectionId) const;
    bool isIndexValid(int index) const;

private:
    void _popClient_impl();
    void _pushClient_impl();
    void _resizeClients_impl(int size);

private:
    GameServerCallbacks m_gameServerCallbacks;

    PollId m_pollId;
    SteamNetworkingIPAddr m_endpoint;

    bool m_gameStarted;

    //Number of party members (clients) required to start the game
    int m_partyNumber;

    //used to safely shutdown the server with Ctrl+C
    static bool SIGNAL_SHUTDOWN;

    //All client data
    std::vector<int> mClients_clientId;
    std::vector<HSteamNetConnection> mClients_connectionId;
    //TODO: Make displayName fixed size
    std::vector<std::string> mClients_displayName;
    std::vector<bool> mClients_isReady;
    std::vector<u8> mClients_teamId;
    int m_firstInvalidIndex;
    int m_lastClientId;

    //initial size of the clients vector
    //will grow to accomodate more if needed
    const int INITIAL_CLIENTS_SIZE;
};
