#ifndef GAME_CLIENT_HPP
#define GAME_CLIENT_HPP

#include <steam/steamnetworkingsockets.h>
#include <SFML/System/Time.hpp>

#include "net_peer.hpp"
#include "context.hpp"

class GameClient;

struct GameClientCallbacks : public ISteamNetworkingSocketsCallbacks
{
    GameClientCallbacks(GameClient* p);
    virtual ~GameClientCallbacks() {}

    virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info) override;

    GameClient* parent;
};

class GameClient : public InContext, public NetPeer
{
public:
    friend struct GameClientCallbacks;
public:
    GameClient(const Context& context, const SteamNetworkingIPAddr& endpoint);
    ~GameClient();

    void receiveLoop();
    void update(const sf::Time& eTime);

    void processPacket(HSteamNetConnection connectionId, CRCPacket* packet);
    void handleCommand(u8 command, CRCPacket* packet);

private:
    GameClientCallbacks m_gameClientCallbacks;

    HSteamNetConnection m_serverConnectionId;
};

#endif //GAME_CLIENT_HPP