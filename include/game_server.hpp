#ifndef GAME_SERVER_HPP
#define GAME_SERVER_HPP

#include <steam/steamnetworkingsockets.h>
#include <SFML/System/Time.hpp>

#include "context.hpp"
#include "net_peer.hpp"
#include "data_oriented_manager.hpp"

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
    void handleCommand(u8 command, int clientId, CRCPacket* packet);

    void addClientToPoll(int id);

    void resetClient(int id);
    int getFreeClientId();
    int getClientIdByConnectionId(HSteamNetConnection connectionId) const;
    void swapClientData(int clientId1, int clientId2);

private:
    GameServerCallbacks m_gameServerCallbacks;

    PollId m_pollId;
    SteamNetworkingIPAddr m_endpoint;

    bool m_gameStarted;

    //Number of party members (clients) required to start the game
    int m_partyNumber;

    //used to safely shutdown the server with Ctrl+C
    static bool SIGNAL_SHUTDOWN;

    //All client info (using DOD)
    DataOrientedManager m_dataOrientedManager;

    std::vector<bool> mClients_isValid;
    std::vector<HSteamNetConnection> mClients_connectionId;
    std::vector<std::string> mClients_displayName;
    std::vector<NetStatus> mClients_netStatus;
    std::vector<bool> mClients_ready;
};

#endif //GAME_SERVER_HPP