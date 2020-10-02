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
#include "client_caster.hpp"
#include "caster_snapshot.hpp"
#include "connection_status_render.hpp"

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

    struct ConfigData {
        SteamNetworkingIPAddr endpoint;
        sf::Time inputRate;
        std::string displayName;
        u8 pickedHero;
    };

    struct Snapshot {
        C_EntityManager entityManager;
        u32 id = 0;
        sf::Time worldTime;

        u32 latestAppliedInput = 0;

        CasterSnapshot caster;
    };

    struct InputSnapshot {
        PlayerInput input;
        Vector2 endPosition;
        bool forceSnap;

        CasterSnapshot caster;
    };

public:
    GameClient(const Context& context, const ConfigData& configData);
    ~GameClient();

    void receiveLoop();
    void update(sf::Time eTime);
    void updateWorldTime(sf::Time eTime);

    void renderUpdate(sf::Time eTime);

    void setupNextInterpolation();

    //called once for each input in every input frame
    void handleInput(const sf::Event& event, bool focused);

    //called at the end of every input frame
    void saveCurrentInput();

    //check server input was correct and redo all inputs otherwise
    void checkServerInput(u32 inputId, const Vector2& endPosition, u16 movementSpeed, const CasterSnapshot& casterSnapshot);

    void sendInitialInfo();

    void processPacket(HSteamNetConnection connectionId, CRCPacket& packet);
    void handleCommand(u8 command, CRCPacket& packet);

    void removeOldSnapshots(u32 olderThan);
    void writeLatestSnapshotId(CRCPacket& packet);

    Snapshot* findSnapshotById(u32 snapshotId);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    void loadMap(const std::string& filename);

private:
    GameClientCallbacks m_gameClientCallbacks;
    HSteamNetConnection m_serverConnectionId;
    SteamNetworkingIPAddr m_endpoint;

    bool m_connected;
    sf::Time m_infoTimer;

    mutable sf::RenderTexture m_canvas;
    bool m_canvasCreated;

    C_EntityManager m_entityManager;
    std::list<Snapshot> m_snapshots;

    //snapshot we're currently using to interpolate
    std::list<Snapshot>::iterator m_interSnapshot_it;

    sf::Time m_worldTime;
    sf::Time m_interElapsed;
    sf::Time m_inputRate;

    //Number of snapshots required to start rendering
    int m_requiredSnapshotsToRender;

    std::list<InputSnapshot> m_inputSnapshots;
    PlayerInput m_currentInput;

    //timer for the interpolation of controlled entity
    sf::Time m_controlledEntityInterTimer;

    TileMapRenderer m_tileMapRenderer;
    TileMap m_tileMap;

    std::unique_ptr<GameMode> m_gameMode;

    Camera m_camera;

    //the unit can move freely within this radius without the camera moving
    float m_smoothUnitRadius;
    Vector2 m_smoothUnitPos;

    ClientCaster m_clientCaster;

    bool m_forceFullSnapshotUpdate;
    bool m_fullUpdateReceived;

    std::string m_displayName;
    u8 m_pickedHero;

    sf::Sprite m_mouseSprite;

    ConnectionStatusRender m_connectionStatusRender;
};
