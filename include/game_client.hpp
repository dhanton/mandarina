#pragma once

#include <steam/steamnetworkingsockets.h>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <list>

#include "paths.hpp"
#include "net_peer.hpp"
#include "context.hpp"
#include "client_entity_manager.hpp"
#include "player_input.hpp"
#include "tilemap_renderer.hpp"
#include "camera.hpp"

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

    struct Snapshot {
        C_EntityManager entityManager;
        u32 id = 0;
        sf::Time worldTime;

        u32 latestAppliedInput = 0;
    };

    struct InputSnapshot {
        PlayerInput input;
        Vector2 endPosition;
    };

public:
    GameClient(const Context& context, const SteamNetworkingIPAddr& endpoint);
    ~GameClient();

    void mainLoop(bool& running);

    void receiveLoop();
    void update(sf::Time eTime);

    void renderUpdate(sf::Time eTime);

    void setupNextInterpolation();

    //called once for each input in every input frame
    void handleInput(const sf::Event& event, bool focused);

    //called at the end of every input frame
    void saveCurrentInput();

    //check server input was correct and redo all inputs otherwise
    void checkServerInput(u32 inputId, const Vector2& endPosition, u16 movementSpeed);

    void processPacket(HSteamNetConnection connectionId, CRCPacket& packet);
    void handleCommand(u8 command, CRCPacket& packet);

    void removeOldSnapshots(u32 olderThan);
    void writeLatestSnapshotId(CRCPacket& packet);

    Snapshot* findSnapshotById(u32 snapshotId);

private:
    GameClientCallbacks m_gameClientCallbacks;
    HSteamNetConnection m_serverConnectionId;
    bool m_connected;
    sf::Time m_infoTimer;

    sf::RenderTexture m_canvas;
    bool m_canvasCreated;

    sf::Time m_updateRate;
    sf::Time m_inputRate;

    C_EntityManager m_entityManager;
    std::list<Snapshot> m_snapshots;

    //snapshot we're currently using to interpolate
    std::list<Snapshot>::iterator m_interSnapshot_it;

    sf::Time m_worldTime;
    sf::Time m_interElapsed;

    //Number of snapshots required to start rendering
    int m_requiredSnapshotsToRender;

    std::list<InputSnapshot> m_inputSnapshots;
    PlayerInput m_currentInput;

    //timer for the interpolation of controlled entity
    sf::Time m_controlledEntityInterTimer;

    TileMapRenderer m_tileMapRenderer;
    TileMap m_tileMap;

    Camera m_camera;

    //the unit can move freely within this radius without the camera moving
    float m_smoothUnitRadius;
    Vector2 m_smoothUnitPos;
};
