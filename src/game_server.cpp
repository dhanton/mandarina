#include "game_server.hpp"

#include <SFML/System/Clock.hpp>
#include <csignal>
#include <cmath>

#include "network_commands.hpp"
#include "player_input.hpp"
#include "helper.hpp"
#include "game_mode_loader.hpp"

//@DELETE
#include "entities/food.hpp"

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
            if (parent->canNewClientConnect()) {
                int index = parent->addClient();

                if (index != -1) {
                    parent->m_clients[index].connectionId = info->m_hConn;

                    u32 clientId = parent->m_clients[index].uniqueId;

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

            } else {
                //@TODO: Implement reconnections
                //traverse all clients
                //if one of them is not valid
                //and has the same identity (secret key)
                //then that one is reconnecting

                //Regarding disconnecting:
                //If the game hasn't started or players can join mid match the disconnection should just remove the client
                //Otherwise the client should be marked as disconnected so if they reconnect their data is not lost
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

GameServer::GameServer(const Context& context, u8 gameModeType):
    m_gameServerCallbacks(this),
    InContext(context),
    NetPeer(&m_gameServerCallbacks, true),
    m_entityManager(context.jsonParser),
	m_heroesDistr(0, g_numberOfHeroes - 1)
{
    createGameMode(gameModeType);

    loadProjectilesFromJson(context.jsonParser);

    m_gameStarted = false;
    m_lastClientId = 0;
    m_lastSnapshotId = 0;
    m_gameEnded = false;

    const rapidjson::Document& doc = *context.jsonParser->getDocument("server_config");

    loadFromJson(doc);

    //Resize this vector to avoid dynamically adding elements
    //(this will still happen if more clients connect)
    m_clients.resize(m_gameMode->getMaxPlayers() + 10);

    m_entityManager.setManagersContext(ManagersContext(nullptr, &m_collisionManager, &m_tileMap, m_gameMode.get()));

    m_entityManager.allocateAll();

    m_tileMap.loadFromFile(m_gameMode->getLobbyMapFilename());

    if (!context.local) {
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
    printMessage("Cleaning up...");

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

    printMessage("Main loop ended");
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
        int clientsReady = 0;

        for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
            if (m_clients[i].connectionCompleted) clientsReady++;
        }

        //@WIP: Add other ways to start the game (when the host wants)
        //This condition should still be met though
        if (clientsReady >= m_gameMode->getRequiredPlayers()) {
            m_gameStarted = true;

            printMessage("Game starting!");

            m_tileMap.loadFromFile(m_gameMode->getMapFilename());

            //remove all entities and projectiles created during the lobby
            m_entityManager.entities.clear();
            m_entityManager.projectiles.clear();
            m_collisionManager.clear();

            m_gameMode->startGame();

            for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
                //create heroes again
                createClientHeroEntity(i, true);
            }

            ManagersContext context(&m_entityManager, &m_collisionManager, &m_tileMap, m_gameMode.get());

            m_gameMode->onGameStarted(m_clients.firstInvalidIndex(), context);

            for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
                CRCPacket outPacket;
                const Entity* entity = m_entityManager.entities.atUniqueId(m_clients[i].controlledEntityUniqueId);

                outPacket << (u8) ClientCommand::GameStarted << m_clients[i].controlledEntityUniqueId << m_clients[i].teamId;

                if (entity) {
                    outPacket << (u8) ClientCommand::PlayerCoords << entity->getPosition().x << entity->getPosition().y;
                }

                sendPacket(outPacket, m_clients[i].connectionId, true);
            }

#ifdef MANDARINA_DEBUG
            //@DELETE (TESTING)
            // m_entityManager.createEntity(ENTITY_RED_DEMON, Vector2(1400.f, 1450.f), 1);
            for (int i = 0; i < 30; ++i) {
                m_entityManager.createEntity(ENTITY_BLONDIE, Vector2(rand() % 1500 + 200, rand() % 1500 + 200.f), 2);
                m_entityManager.createEntity(ENTITY_RED_DEMON, Vector2(rand() % 1500 + 200, rand() % 1500 + 200.f), 2); }
#endif
        }
    }

    if (m_gameMode->hasGameEnded()) {
        if (!m_gameEnded) {
            m_gameEnded = true;
            printMessage("Game ended");
            
            for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
                CRCPacket outPacket;
                outPacket << (u8) ClientCommand::GameEnded;

                //send winner team, leaderboard, etc
                m_gameMode->packGameEndData(outPacket);

                sendPacket(outPacket, m_clients[i].connectionId, true);
            }
        }

        m_gameEndTimer += eTime;

        if (m_gameEndTimer >= m_gameEndLingeringTime) {
            running = false;
            printMessage("Lingering time ended");

            return;
        }
    }

    m_worldTime += eTime;

    m_gameMode->onUpdate(eTime);

    /////////////////////////////////////////////////////////////////////////////

    //Important to note that even if the game hasn't started yet
    //the idea is to be able to walk around and do a bunch of activities
    //with your character (without lethal damage or with infinite respawns)

    //So the game "lobby" is like a tavern for players to gather before the game
    //Players in the lobby are assumed to be ready

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
    
    handleDeadHeroes();
    //@TODO: handleRespawnedHeroes
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
        outPacket << m_clients[i].forceFullUpdate;

        EntityManager* snapshotManager = nullptr;

        if (!m_clients[i].forceFullUpdate) {
            if (m_clients[i].snapshotId < oldestSnapshotId) {
                oldestSnapshotId = m_clients[i].snapshotId;
            }

            auto it = m_snapshots.find(m_clients[i].snapshotId);

            if (it != m_snapshots.end()) {
                snapshotManager = &it->second.entityManager;
            }
        }

        u8 teamId = (m_clients[i].heroDead ? m_clients[i].spectatingTeamId : m_clients[i].teamId);

        m_entityManager.packData(snapshotManager, teamId, m_clients[i].controlledEntityUniqueId, outPacket);

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

        case ServerCommand::LatestSnapshotId:
        {
            u32 latestId;
            packet >> latestId;

            if (m_clients[index].forceFullUpdate) {
                //only accept snapshots with id = 0 if forceFullUpdate is true
                if (latestId == 0) {
                    m_clients[index].forceFullUpdate = false;
                }

            } else if (latestId <= m_lastSnapshotId && latestId >= m_clients[index].snapshotId) {
                m_clients[index].snapshotId = latestId;
            }

            break;
        }

        case ServerCommand::PlayerInput:
        {
            PlayerInput playerInput;

            float maxInputNumber = m_clients[index].inputRate.asSeconds()/m_updateRate.asSeconds();

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

                entity->applyInput(playerInput, ManagersContext(&m_entityManager, &m_collisionManager, &m_tileMap, m_gameMode.get()), clientDelay);

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

        case ServerCommand::DisplayName:
        {
            std::string displayName;
            packet >> displayName;

            if (displayName.size() > HeroBase::maxDisplayNameSize) {
                displayName.resize(HeroBase::maxDisplayNameSize);
            }

            m_clients[index].displayName = displayName;

            Hero* hero = static_cast<Hero*>(m_entityManager.entities.atUniqueId(m_clients[index].controlledEntityUniqueId));

            if (hero) {
                hero->setDisplayName(displayName);
            }

            break;
        }

		case ServerCommand::PickedHero:
		{
			u8 pickedHero;
			packet >> pickedHero;

			if (pickedHero == 0 || pickedHero > g_numberOfHeroes) {
				pickedHero = m_heroesDistr(Helper_Random::gen()) + 1;
			}

			m_clients[index].pickedHeroType = g_heroTypes[pickedHero - 1];
			m_clients[index].heroDead = false;
			
			Hero* hero = createClientHeroEntity(index);

			if (hero) {
				CRCPacket outPacket;
				outPacket << (u8) ClientCommand::HeroCreated << m_clients[index].controlledEntityUniqueId << m_clients[index].teamId;
				outPacket << (u8) ClientCommand::PlayerCoords << hero->getPosition().x << hero->getPosition().y;

                m_clients[index].connectionCompleted = true;

				sendPacket(outPacket, m_clients[index].connectionId, true);
			}

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
        outPacket << (u8) ClientCommand::RequestInitialInfo; 
        outPacket << (u8) ClientCommand::GameModeType << m_gameMode->getType() << m_gameStarted;

        sendPacket(outPacket, connectionId, true);
        printMessage("Connection completed with client %d", m_clients[index].uniqueId);
    }
}

Hero* GameServer::createClientHeroEntity(int index, bool keepOldUniqueId)
{
    if (index == -1) return nullptr;

    Entity* entity = nullptr;

    //remove one if it already exists
    u32 uniqueId = m_clients[index].controlledEntityUniqueId;

    if (uniqueId != 0) {
        entity = m_entityManager.entities.atUniqueId(uniqueId);

        if (entity) {
            m_entityManager.entities.removeEntity(uniqueId);
        }
    }

    if (keepOldUniqueId) {
        //we can force the entity to conserve the old client controlledEntityUniqueId
        entity = m_entityManager.createEntity(m_clients[index].pickedHeroType, Vector2(), m_clients[index].teamId, uniqueId);
        static_cast<Hero*>(entity)->setDisplayName(m_clients[index].displayName);

    } else {
        //or we can use a new one
        entity = m_entityManager.createEntity(m_clients[index].pickedHeroType, Vector2(), m_clients[index].teamId);

        if (entity) {
            //and update the client info
            m_clients[index].controlledEntityUniqueId = entity->getUniqueId();
        }
    }

    if (entity) {
        //the game mode might move the unit or change its teamId
        m_gameMode->onHeroCreated(static_cast<Hero*>(entity));

        //update the client teamId using the unit 
        m_clients[index].teamId = entity->getTeamId();
    }

    //make sure the clients can't choose an entity type that's not a hero
    return static_cast<Hero*>(entity);
}

void GameServer::handleDeadHeroes()
{
    auto& deadHeroes = m_gameMode->getNewDeadHeroes();

    for (auto heroData : deadHeroes) {
        int index = -1;
        int killerIndex = -1;

        //find which client controlls this hero
        for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
            if (m_clients[i].controlledEntityUniqueId == heroData.uniqueId) {
                index = i;
            }

            if (m_clients[i].controlledEntityUniqueId == heroData.killerUniqueId) {
                killerIndex = i;
            }

            if (index != -1 && killerIndex != -1) break;
        }

        if (index != -1) {
            m_clients[index].heroDead = true;

            //next entity and team the dead player (and everyone that was spectating them)
            //is going to spectate now
            u32 spectateUniqueId;
            u8 spectateTeamId;

            if (!heroData.teamEliminated) {
                //@TODO: Implement teams

                //if the team is not eliminated, choose someone on the team
                spectateTeamId = heroData.teamId;

                //find next player on the team to spectate
                //spectateUniqueId = ...

            } else {
                spectateUniqueId = heroData.killerUniqueId;
                spectateTeamId = heroData.killerTeamId;

                //if the killer hero is dead we spectate whatever they were spectating
                if (killerIndex != -1 && m_clients[killerIndex].heroDead) {
                    spectateUniqueId = m_clients[killerIndex].spectatingUniqueId;
                    spectateTeamId = m_clients[killerIndex].spectatingTeamId;
                }

                //if the killer is a neutral entity or 
                //if the killer was spectating us (cause we killed them first)
                //choose new team and entity to spectate altogether
                if (spectateTeamId == 0 || spectateUniqueId == heroData.uniqueId || spectateTeamId == heroData.teamId) {
                    //choose the first client that qualifies
                    for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
                        if (!m_clients[i].heroDead) {
                            spectateUniqueId = m_clients[i].controlledEntityUniqueId;
                            spectateTeamId = m_clients[i].teamId;
                            break;
                        }
                    }
                }
            }

            for (int i = 0; i < m_clients.firstInvalidIndex(); ++i) {
                //if a client was spectating or controlling the hero that just died change spectator
                if (i == index || m_clients[i].spectatingUniqueId == heroData.uniqueId) {
                    bool changedTeam = (m_clients[i].spectatingTeamId != spectateTeamId);

                    m_clients[i].spectatingUniqueId = spectateUniqueId;
                    m_clients[i].spectatingTeamId = spectateTeamId;

                    CRCPacket outPacket;
                    outPacket << (u8) ClientCommand::ChangeSpectator << spectateUniqueId << spectateTeamId;

                    if (m_clients[i].teamId == m_clients[index].teamId) {
                        //tell all players of that team that they're eliminated
                        outPacket << (u8) ClientCommand::TeamEliminated;
                    }

                    sendPacket(outPacket, m_clients[i].connectionId, true);

                    if (changedTeam) {
                        m_clients[i].snapshotId = 0;
                        m_clients[i].forceFullUpdate = true;
                    }
                }
            }
        }
    }

    deadHeroes.clear();
}

int GameServer::addClient()
{
    u32 clientId = ++m_lastClientId;
    int index = m_clients.addElement(clientId);

    m_clients[index].uniqueId = clientId;

    return index;
}

bool GameServer::canNewClientConnect() const
{
    return (!m_gameStarted || m_gameMode->canJoinMidMatch()) && m_clients.firstInvalidIndex() < m_gameMode->getMaxPlayers();
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

void GameServer::createGameMode(u8 gameModeType)
{
    m_gameMode = std::unique_ptr<GameMode>(GameModeLoader::create(gameModeType, m_context));
    m_gameMode->setTileMap(&m_tileMap);
}

std::string GameServer::getCurrentMapFilename() const
{
    return (m_gameStarted ? m_gameMode->getMapFilename() : m_gameMode->getLobbyMapFilename());
}

void GameServer::loadFromJson(const rapidjson::Document& doc)
{
    if (doc.HasMember("ip_address")) {
        m_endpoint.ParseString(doc["ip_address"].GetString());
    } else {
        m_endpoint.ParseString("127.0.0.1");
    }

    if (doc.HasMember("port")) {
        m_endpoint.m_port = doc["port"].GetUint();
    } else {
        m_endpoint.m_port = 7000;
    }

    m_updateRate = sf::seconds(1.f/doc["update_rate"].GetFloat());
    m_snapshotRate = sf::seconds(1.f/doc["snapshot_rate"].GetFloat());
    m_defaultInputRate = sf::seconds(1.f/doc["default_input_rate"].GetFloat());

    m_canClientsChangeSnapshotRate = doc["can_clients_change_snapshot_rate"].GetBool();
    m_canClientsChangeInputRate = doc["can_clients_change_input_rate"].GetBool();

    if (doc.HasMember("max_input_rate")) {
        m_maxInputRate = sf::seconds(1.f/doc["max_input_rate"].GetFloat());
    } else {
        m_maxInputRate = sf::seconds(1.f/120.f);
    }

    if (doc.HasMember("min_input_rate")) {
        m_minInputRate = sf::seconds(1.f/doc["min_input_rate"].GetFloat());
    } else {
        m_minInputRate = sf::seconds(1.f/20.f);
    }

    if (doc.HasMember("max_snapshot_rate")) {
        m_maxSnapshotRate = sf::seconds(1.f/doc["max_snapshot_rate"].GetFloat());
    } else {
        m_maxSnapshotRate = m_updateRate;
    }

    if (doc.HasMember("min_snapshot_rate")) {
        m_minSnapshotRate = sf::seconds(1.f/doc["min_snapshot_rate"].GetFloat());
    } else {
        m_minSnapshotRate = sf::seconds(1.f/10.f);
    }

    if (doc.HasMember("max_ping_correction")) {
        m_maxPingCorrection = sf::milliseconds(doc["max_ping_correction"].GetUint());
    } else {
        m_maxPingCorrection = sf::milliseconds(70);
    }

    if (doc.HasMember("game_end_lingering_time")) {
        m_gameEndLingeringTime = sf::seconds(doc["game_end_lingering_time"].GetFloat());
    } else {
        m_gameEndLingeringTime = sf::seconds(20.f);
    }
}
