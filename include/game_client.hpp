#pragma once

#include <steam/steamnetworkingsockets.h>
#include <SFML/System/Time.hpp>
#include <list>

#include "net_peer.hpp"
#include "context.hpp"
#include "client_entity_manager.hpp"

class GameClient;

struct GameClientCallbacks : public ISteamNetworkingSocketsCallbacks
{
    GameClientCallbacks(GameClient* p);
    virtual ~GameClientCallbacks() {}

    virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info) override;

    GameClient* parent;
};

class GameClient : public InContext, public NetPeer, public sf::Drawable
{
public:
    friend struct GameClientCallbacks;

    struct Snapshot {
        C_EntityManager entityManager;
        u32 id = 0;
        sf::Time worldTime;
    };

public:
    GameClient(const Context& context, const SteamNetworkingIPAddr& endpoint);
    ~GameClient();

    void receiveLoop();
    void update(sf::Time eTime);
    void renderUpdate(sf::Time eTime);
    void updateWorldTime(sf::Time eTime);

    void processPacket(HSteamNetConnection connectionId, CRCPacket& packet);
    void handleCommand(u8 command, CRCPacket& packet);
    
    void setupNextInterpolation();

    void removeOldSnapshots(u32 olderThan);
    void sendLatestSnapshotId();

    Snapshot* findSnapshotById(u32 snapshotId);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    GameClientCallbacks m_gameClientCallbacks;
    HSteamNetConnection m_serverConnectionId;
    bool m_connected;

    C_EntityManager m_entityManager;
    std::list<Snapshot> m_snapshots;

    //snapshot we're currently using to interpolate
    std::list<Snapshot>::iterator m_interSnapshotIt;

    sf::Time m_worldTime;
    sf::Time m_interElapsed;

    //Number of snapshots required to start rendering
    int m_requiredSnapshotsToRender;
};
