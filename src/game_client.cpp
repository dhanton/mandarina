#include "game_client.hpp"

#include "network_commands.hpp"

GameClientCallbacks::GameClientCallbacks(GameClient* p)
{
    parent = p;
}

void GameClientCallbacks::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info)
{
    switch (info->m_info.m_eState)
    {
        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
                parent->printMessage("Unable to reach server");

            } else if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) {
                parent->printMessage("Connection with server closed by peer");

            } else {
                parent->printMessage("There was a problem with the connection");
            }

            parent->m_pInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
            parent->m_serverConnectionId = k_HSteamNetConnection_Invalid;

            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            parent->printMessage("Connecting...");
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
        {
            parent->printMessage("Connection completed with server");
            parent->m_connected = true;

            //Eventually, sending ready command should be based on input from the player
            //Like interacting with something in the tavern
            //Or clicking a button in a menu

            CRCPacket packet;
            packet << (u8) ServerCommand::PlayerReady << true;
            parent->sendPacket(packet, parent->m_serverConnectionId, true);
            break;
        }
    }
}

GameClient::GameClient(const Context& context, const SteamNetworkingIPAddr& endpoint):
    m_gameClientCallbacks(this),
    InContext(context),
    NetPeer(&m_gameClientCallbacks, false),
    m_entityManager(context)
{
    m_entityManager.allocate();
    m_interSnapshotIt = m_snapshots.begin();
    m_requiredSnapshotsToRender = 3;

    if (!context.local) {
        m_serverConnectionId = connectToServer(endpoint);
        m_connected = false;

    } else {
        m_connected = true;
        m_serverConnectionId = context.localCon1;

        printMessage("Adding server in local connection");

        //local connection doesn't trigger callbacks
        CRCPacket packet;
        packet << (u8) ServerCommand::PlayerReady << true;
        sendPacket(packet, m_serverConnectionId, true);
    }
}

GameClient::~GameClient()
{
    m_pInterface->CloseConnection(m_serverConnectionId, 0, nullptr, false);
}

void GameClient::receiveLoop()
{
    NetPeer::receiveLoop(m_serverConnectionId);
}

void GameClient::update(sf::Time eTime)
{
    if (!m_connected) return;

    m_worldTime += eTime;

    sendLatestSnapshotId();
}

void GameClient::renderUpdate(sf::Time eTime)
{
    if (std::next(m_interSnapshotIt, m_requiredSnapshotsToRender) != m_snapshots.end()) {
        m_interElapsed += eTime;

        auto next_it = std::next(m_interSnapshotIt);

        sf::Time totalTime = next_it->worldTime - m_interSnapshotIt->worldTime;

        if (m_interElapsed >= totalTime && std::next(next_it) != m_snapshots.end()) {
            m_interElapsed -= totalTime;

            m_interSnapshotIt = std::next(m_interSnapshotIt);
            next_it = std::next(next_it);

            setupNextInterpolation();

            totalTime = next_it->worldTime - m_interSnapshotIt->worldTime;
        }

        m_entityManager.performInterpolation(&m_interSnapshotIt->entityManager, &next_it->entityManager, m_interElapsed.asSeconds(), totalTime.asSeconds());
    }
}

void GameClient::processPacket(HSteamNetConnection connectionId, CRCPacket& packet)
{
    while (!packet.endOfPacket()) {
        u8 command = 0;
        packet >> command;

        handleCommand(command, packet);
    }
}

void GameClient::handleCommand(u8 command, CRCPacket& packet)
{
    switch (ClientCommand(command))
    {
        case ClientCommand::Null:
        {
            printMessage("handleCommand error - NULL command");

            //receiving null command invalidates the rest of the packet
            packet.clear();
            break;
        }

        case ClientCommand::Snapshot:
        {
            u32 snapshotId, prevSnapshotId;
            packet >> snapshotId >> prevSnapshotId;

            Snapshot* prevSnapshot = findSnapshotById(prevSnapshotId);
            C_EntityManager* prevEntityManager = nullptr;

            if (!prevSnapshot) {
                if (prevSnapshotId != 0) {
                    printMessage("Snapshot error - Previous snapshot doesn't exist");
                    packet.clear();
                    break;

                }

            } else {
                prevEntityManager = &prevSnapshot->entityManager;
            }

            m_snapshots.emplace_back();

            Snapshot& snapshot = m_snapshots.back();
            snapshot.id = snapshotId;
            snapshot.entityManager.loadFromData(prevEntityManager, packet);
            snapshot.worldTime = m_worldTime;

            removeOldSnapshots(prevSnapshotId);
            sendLatestSnapshotId();

            //populate C_EntityManager if we had no previous snapshots
            //(this happens when we receive the first snapshot, but can also happen if there's a lot of packet loss)
            if (m_snapshots.size() == 1) {
                m_interSnapshotIt = m_snapshots.begin();
                setupNextInterpolation();
            }

            break;
        }
    }
}

void GameClient::setupNextInterpolation()
{
    m_entityManager.copySnapshotData(&m_interSnapshotIt->entityManager);
}

void GameClient::removeOldSnapshots(u32 olderThan)
{
    auto it = m_snapshots.begin();

    //only erase elements that no longer need to be rendered
    while(it != m_interSnapshotIt) {
        if (it->id < olderThan) {
            it = m_snapshots.erase(it);
        } else {
            it = std::next(it);
        }
    }
}

GameClient::Snapshot* GameClient::findSnapshotById(u32 snapshotId)
{
    for (Snapshot& snapshot : m_snapshots) {
        if (snapshot.id == snapshotId) {
            return &snapshot;
        }
    }

    return nullptr;
}

void GameClient::sendLatestSnapshotId()
{
    CRCPacket outPacket;
    outPacket << (u8) ServerCommand::LatestSnapshotId;
    outPacket << m_snapshots.back().id;

    sendPacket(outPacket, m_serverConnectionId, false);
}

void GameClient::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_entityManager, states);
}
