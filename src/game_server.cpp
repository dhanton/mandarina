#include "game_server.hpp"

#include <SFML/System/Clock.hpp>
#include <csignal>
#include <cmath>

#include "network_commands.hpp"
#include "player_input.hpp"
#include "helper.hpp"

GameServerCallbacks::GameServerCallbacks(GameServer* p)
{
    parent = p;
}

void GameServerCallbacks::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info)
{
    switch (info->m_info.m_eState)
    {
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        {
            int index = parent->getIndexByConnectionId(info->m_hConn);

            if (index != -1) {
                u32 clientId = parent->m_clients[index].uniqueId;

                if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) {
                    parent->printMessage("Connection with client %d closed by peer", clientId);

                } else {
                    parent->printMessage("Connection with client %d lost", clientId);
                }

                parent->m_clients.removeElement(clientId);
                parent->m_pInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
            }

            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            int index = -1;
            
            if (parent->m_gameStarted) {
                //traverse all clients
                //if one of them is not valid
                //and has the same identity (secret key)
                //then that one is reconnecting
            } else {
                index = parent->addClient();
            }

            if (index != -1) {
                parent->m_clients[index].connectionId = info->m_hConn;

                u32 clientId = parent->m_clients[index].uniqueId;

                //Set the rest of the parameters (displayName, character, team, etc)

                if (parent->m_pInterface->AcceptConnection(info->m_hConn) != k_EResultOK) {
                    parent->printMessage("There was an error accepting connection with client %d", clientId);
                    parent->m_pInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
                    break;
                }

                if (!parent->addClientToPoll(index)) {
                    parent->printMessage("There was an error adding client %d to poll", clientId);
                    parent->m_pInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
                    break;
                }

                parent->printMessage("Connecting with client %d", clientId);
            }

            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
        {
            parent->onConnectionCompleted(info->m_hConn);

            break;
        }
    }
}

bool GameServer::SIGNAL_SHUTDOWN = false;

GameServer::GameServer(const Context& context, int playersNeeded):
    m_gameServerCallbacks(this),
    InContext(context),
    NetPeer(&m_gameServerCallbacks, true),

    //TODO: Initial size should be the expected value of the amount of clients that will try to connect
    //This can be hight/low independently of playersNeeded 
    //for example if a streamer sets up a server maybe 10k people try to connect (even for a 2-man party)
    //Still has to always be larger than playersNeeded
    INITIAL_CLIENTS_SIZE(playersNeeded * 2 + 10),
    m_entityManager(context.jsonParser)
{
    loadProjectilesFromJson(context.jsonParser);

    m_gameStarted = false;
    m_playersNeeded = playersNeeded;
    m_lastClientId = 0;
    m_lastSnapshotId = 0;

    //@TODO:
    ////////////////////////// THINGS TO LOAD FROM JSON FILE ////////////////////////////////////
    m_updateRate = sf::seconds(1.f/30.f);
    m_snapshotRate = sf::seconds(1.f/20.f);
    m_defaultInputRate = sf::seconds(1.f/30.f);

    m_canClientsChangeInputRate = false;
    m_canClientsChangeSnapshotRate = false;

    m_maxInputRate = sf::seconds(1.f/120.f);
    m_minInputRate = sf::seconds(1.f/20.f);
    m_maxSnapshotRate = m_updateRate;
    m_minSnapshotRate = sf::seconds(1.f/10.f);

    m_maxPingCorrection = sf::milliseconds(70);

    m_tileMapFilename = "test_small";

    /////////////////////////////////////////////////////////////////////////////////////////////

    //Resize this vector to avoid dynamically adding elements
    //(this will still happen if more clients connect)
    m_clients.resize(INITIAL_CLIENTS_SIZE);

    m_entityManager.setCollisionManager(&m_collisionManager);
    m_entityManager.setTileMap(&m_tileMap);
    m_entityManager.allocateAll();

    m_tileMap.loadFromFile(MAPS_PATH + m_tileMapFilename + "." + MAP_FILENAME_EXT);
    
    //@DELETE (TESTING)
    m_entityManager.createEntity(ENTITY_RED_DEMON, Vector2(1550.f, 800.f), 1);
    for (int i = 0; i < 100; ++i) {
        m_entityManager.createEntity(ENTITY_RED_DEMON, Vector2(rand() % 1500 + 200, rand() % 1500 + 200.f), rand()%2);
    }

    if (!context.local) {
        m_endpoint.ParseString("127.0.0.1:7000");
        m_pollId = createListenSocket(m_endpoint);

    } else {
        m_isServer = false;
        m_pollId.listenSocket = context.localCon2;
        
        int index = addClient();
        m_clients[index].connectionId = context.localCon2;

        printMessage("Adding client in local connection");

        onConnectionCompleted(context.localCon2);
    }

#ifndef _WIN32
    //Shutdown when Ctrl+C on linux
    struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = [] (int s) -> void {SIGNAL_SHUTDOWN = true;};
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);
#endif // _WIN32
}

GameServer::~GameServer()
{
    for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
        m_pInterface->CloseConnection(m_clients[i].connectionId, 0, nullptr, false);
    }

    m_pInterface->CloseListenSocket(m_pollId.listenSocket);
    m_pInterface->DestroyPollGroup(m_pollId.pollGroup);
}

void GameServer::mainLoop(bool& running)
{
    sf::Clock clock;

    sf::Time updateTimer;
    sf::Time snapshotTimer;

    while (running) {
        sf::Time eTime = clock.restart();

        updateTimer += eTime;
        snapshotTimer += eTime;

        receiveLoop();

        if (updateTimer >= m_updateRate) {
            update(m_updateRate, running);
            updateTimer -= m_updateRate;
        }

        //@TODO: Send snapshots at different rates for different clients
        //(if m_canClientsChangeSnapshotRate == true)
        if (snapshotTimer >= m_snapshotRate) {
            sendSnapshots();
            snapshotTimer -= m_snapshotRate;
        }

        //Remove this for maximum performance (more CPU usage)
        // std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void GameServer::receiveLoop()
{
    if (m_context.local) {
        NetPeer::receiveLoop(m_pollId.listenSocket);
    } else {
        NetPeer::receiveLoop(m_pollId.pollGroup);
    }
}

void GameServer::update(const sf::Time& eTime, bool& running)
{
#ifndef _WIN32
    if (SIGNAL_SHUTDOWN) {
        running = false;
        std::cout << std::endl;
        return;
    }
#endif //_WIN32

    if (!m_gameStarted) {
        int playersReady = 0;

        for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
            if (m_clients[i].isReady) {
                playersReady++;
            }
        }

        if (playersReady >= m_playersNeeded) {
            m_gameStarted = true;

            printMessage("Game starting");

            //remove valid clients that didn't make it into the party
            while (m_clients.firstInvalidIndex() > m_playersNeeded) {
                m_pInterface->CloseConnection(m_clients[m_playersNeeded].connectionId, 0, nullptr, false);
                m_clients.removeElement(m_clients[m_playersNeeded].uniqueId);
            }

            //to save space, since no new clients are gonna be added
            m_clients.removeInvalidData();

            //inform the players that the game is starting
            //start the main game level
        }
    }

    m_worldTime += eTime;

    /////////////////////////////////////////////////////////////////////////////

    //Important to note that even if the game hasn't started yet
    //the idea is to be able to walk around and do a bunch of activities
    //with your character

    //So the game "lobby" is like a tavern for players to gather before the game
    //Players can perform an action to inform the server that they're ready
    
    //When all party members are ready the game starts 
    //All the other players are kicked out

    for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
        //reset number of inputs sent (for next update)
        m_clients[i].inputsSent = 0;

        //get the ping 
        //Maybe we should average previous pings to get more accurate info??
        SteamNetworkingQuickConnectionStatus status;
        SteamNetworkingSockets()->GetQuickConnectionStatus(m_clients[i].connectionId, &status);
        m_clients[i].ping = status.m_nPing;
    }

    m_entityManager.update(eTime);
}

void GameServer::sendSnapshots()
{
    //will store oldest snapshot used by clients
    u32 oldestSnapshotId = -1;

    //Take current snapshot
    u32 snapshotId = ++m_lastSnapshotId;
    Snapshot& snapshot = m_snapshots.emplace(snapshotId, Snapshot()).first->second;

    snapshot.worldTime = m_worldTime;
    snapshot.id = snapshotId;

    m_entityManager.takeSnapshot(&snapshot.entityManager);

    for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
        CRCPacket outPacket;
        outPacket << (u8) ClientCommand::Snapshot;

        //@TODO: Should we use delta encoding to send all this data?

        outPacket << m_lastSnapshotId;
        outPacket << m_clients[i].snapshotId;
        outPacket << m_clients[i].latestInputId;
        outPacket << m_clients[i].controlledEntityUniqueId;

        if (m_clients[i].snapshotId < oldestSnapshotId) {
            oldestSnapshotId = m_clients[i].snapshotId;
        }

        auto it = m_snapshots.find(m_clients[i].snapshotId);
        EntityManager* snapshotManager = nullptr;

        if (it != m_snapshots.end()) {
            snapshotManager = &it->second.entityManager;
        }

        m_entityManager.packData(snapshotManager, m_clients[i].teamId, outPacket);

        sendPacket(outPacket, m_clients[i].connectionId, false);
    }

    //snapshots no longer needed are deleted
    auto it = m_snapshots.begin();
    while (it != m_snapshots.end()) {
        if (it->first < oldestSnapshotId) {
            it = m_snapshots.erase(it);
        } else {
            it = std::next(it);
        }
    }
}

void GameServer::processPacket(HSteamNetConnection connectionId, CRCPacket& packet)
{
    int index = getIndexByConnectionId(connectionId);

    if (index == -1) {
        printMessage("processPacket error - Invalid connection id %d", connectionId);
        return;
    }

    //@TODO: Check packets are not super big to avoid
    //attackers slowing down the server with the proccessing of big packets
    //packets from clients are supposed to be very small (only inputs and snapshotId)
    // if (packet.getDataSize() > 20000) {
    //     display some message;
    //     ??disconnect the client??
    //     return;
    // }

    while (!packet.endOfPacket()) {
        u8 command = 0;
        packet >> command;

        handleCommand(command, index, packet);
    }
}

void GameServer::handleCommand(u8 command, int index, CRCPacket& packet)
{
    switch (ServerCommand(command))
    {
        case ServerCommand::Null:
        {
            printMessage("handleCommand error - NULL command (client %d)", m_clients[index].uniqueId);

            //receiving null command invalidates the rest of the packet
            packet.clear();
            break;
        }

        case ServerCommand::PlayerReady:
        {
            bool ready;
            packet >> ready;

            m_clients[index].isReady = ready;
            break;
        }

        case ServerCommand::LatestSnapshotId:
        {
            u32 latestId;
            packet >> latestId;

            if (latestId <= m_lastSnapshotId && latestId >= m_clients[index].snapshotId) {
                m_clients[index].snapshotId = latestId;
            }

            break;
        }

        case ServerCommand::PlayerInput:
        {
            PlayerInput playerInput;

            float maxInputNumber = m_updateRate.asSeconds()/m_clients[index].inputRate.asSeconds();

            if (m_clients[index].inputsSent >= std::floor(maxInputNumber)) {
                printMessage("Client %d has already sent too many inputs this update", index);
                packet.clear();
                break;
            }

            //we still have to load the data even if the entity doesn't exist
            PlayerInput_loadFromData(playerInput, packet);
            playerInput.timeApplied = m_clients[index].inputRate;

            Entity* entity = m_entityManager.entities.atUniqueId(m_clients[index].controlledEntityUniqueId);

            //only apply inputs that haven't been applied yet
            if (entity && playerInput.id > m_clients[index].latestInputId) {
                //@WIP: Don't hardcode 150, use the same interpolation delay the client is using
                //Client::snapshotsRequiredToRender / Server::m_snapshotRate  = 0.150 seconds = renderDelay
                //clientDelay = pingDelay + renderDelay
                int clientDelay = Helper_clamp(m_clients[index].ping, 0, m_maxPingCorrection.asMilliseconds()) + 150;

                entity->applyInput(playerInput, ManagersContext(&m_entityManager, &m_collisionManager, &m_tileMap), clientDelay);

                m_clients[index].latestInputId = playerInput.id;
            }

            break;
        }

        case ServerCommand::ChangeInputRate:
        {
            u64 inputRate_u64;
            packet >> inputRate_u64;

            sf::Time inputRate = sf::microseconds(inputRate_u64);

            m_clients[index].inputRate = Helper_clamp(inputRate, m_maxInputRate, m_minInputRate);

            break;
        }

        case ServerCommand::ChangeSnapshotRate:
        {
            //@TODO: Make this command actually do something
            u64 snapshotRate_u64;
            packet >> snapshotRate_u64;

            sf::Time snapshotRate = sf::microseconds(snapshotRate_u64);

            m_clients[index].snapshotRate = Helper_clamp(snapshotRate, m_maxSnapshotRate, m_minSnapshotRate);

            break;
        }
    }
}

void GameServer::onConnectionCompleted(HSteamNetConnection connectionId)
{
    int index = getIndexByConnectionId(connectionId);

    if (index != -1) {
        m_clients[index].snapshotRate = m_snapshotRate;
        m_clients[index].inputRate = m_defaultInputRate;

        CRCPacket outPacket;
        outPacket << (u8) ClientCommand::InitialConditions << m_tileMapFilename;

        //@TODO: Create unit based on game mode settings (position, teamId) and hero selection (class type)
        Vector2 pos = {1500.f, 1500.f};
        Entity* entity = m_entityManager.createEntity(ENTITY_RED_DEMON, pos, m_clients[index].teamId);

        if (entity != nullptr) {
            m_clients[index].controlledEntityUniqueId = entity->getUniqueId();
            outPacket << m_clients[index].controlledEntityUniqueId;
            outPacket << m_clients[index].teamId;
            outPacket << pos.x << pos.y;
        } else {
            outPacket << (u32) 0;
        }

        sendPacket(outPacket, connectionId, true);
        printMessage("Connection completed with client %d", m_clients[index].uniqueId);
    }
}

int GameServer::addClient()
{
    u32 clientId = ++m_lastClientId;
    int index = m_clients.addElement(clientId);

    m_clients[index].uniqueId = clientId;

    return index;
}

bool GameServer::addClientToPoll(int index)
{
    if (!m_clients.isIndexValid(index)) {
        printMessage("addToPoll error - Invalid index %d", index);
        return false;
    }

    return m_pInterface->SetConnectionPollGroup(m_clients[index].connectionId, m_pollId.pollGroup);
}

int GameServer::getIndexByConnectionId(HSteamNetConnection connectionId) const
{
    for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
        if (m_clients[i].connectionId == connectionId) {
            return i;
        }
    }

    return -1;
}
