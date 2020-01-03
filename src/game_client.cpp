#include "game_client.hpp"

#include "network_commands.hpp"
#include "helper.hpp"

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
    m_entityManager.allocateAll();
    m_interSnapshot_it = m_snapshots.begin();
    m_requiredSnapshotsToRender = 3;

    m_currentInput.id = 1;

    ////////////////////// THIINGS TO LOAD FROM JSON FILE ////////////////////////
    m_updateRate = sf::seconds(1.f/30.f);
    m_inputRate = sf::seconds(1.f/30.f);

    //////////////////////////////////////////////////////////////////////////////

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

void GameClient::mainLoop(bool& running)
{
    sf::RenderWindow window{{960, 640}, "Mandarina Prototype", sf::Style::Titlebar | sf::Style::Close};

    sf::Clock clock;

    sf::Time updateTimer;
    sf::Time inputTimer;

    bool focused = true;

    while (running) {
        sf::Time eTime = clock.restart();
        
        m_worldTime += eTime;

        updateTimer += eTime;
        inputTimer += eTime;

        receiveLoop();

        while (inputTimer >= m_inputRate) {
            sf::Event event;

            while (window.pollEvent(event)) {
                if (event.type == sf::Event::GainedFocus) {
                    focused = true;
                }

                if (event.type == sf::Event::LostFocus) {
                    focused = false;
                }
                
                if (event.type == sf::Event::Closed) {
                    running = false;
                }

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                    running = false;
                }

                handleInput(event, focused);
            }

            saveCurrentInput();
            inputTimer -= m_inputRate;
        }

        while (updateTimer >= m_inputRate) {
            update(m_inputRate);
            updateTimer -= m_inputRate;
        }
        
        renderUpdate(eTime);

        window.clear();
        window.draw(m_entityManager);
        window.display();
    }
}

void GameClient::receiveLoop()
{
    NetPeer::receiveLoop(m_serverConnectionId);
}

void GameClient::update(sf::Time eTime)
{
    if (!m_connected) return;

    m_entityManager.update(eTime);

    CRCPacket outPacket;
    writeLatestSnapshotId(outPacket);

    sendPacket(outPacket, m_serverConnectionId, false);
}

void GameClient::renderUpdate(sf::Time eTime)
{
    if (std::next(m_interSnapshot_it, m_requiredSnapshotsToRender) != m_snapshots.end()) {
        m_interElapsed += eTime;

        auto next_it = std::next(m_interSnapshot_it);

        sf::Time totalTime = next_it->worldTime - m_interSnapshot_it->worldTime;

        if (m_interElapsed >= totalTime && std::next(next_it) != m_snapshots.end()) {
            m_interElapsed -= totalTime;

            m_interSnapshot_it = std::next(m_interSnapshot_it);
            next_it = std::next(next_it);

            setupNextInterpolation();

            totalTime = next_it->worldTime - m_interSnapshot_it->worldTime;
        }

        m_entityManager.performInterpolation(&m_interSnapshot_it->entityManager, &next_it->entityManager, 
                                             m_interElapsed.asSeconds(), totalTime.asSeconds());

        //interpolate the controlled entity between the latest two inputs
        C_TestCharacter* entity = m_entityManager.m_characters.atUniqueId(m_entityManager.m_controlledEntityUniqueId);

        if (entity && !m_inputSnapshots.empty()) {
            m_controlledEntityInterTimer += eTime;

            //interpolate using current entity position if there's only one input available
            Vector2 oldPos = entity->pos;
            auto oldIt = std::next(m_inputSnapshots.end(), -2);

            if (oldIt != m_inputSnapshots.end()) {
                oldPos = oldIt->endPosition;
            }

            entity->pos = Helper_lerpVec2(oldPos, m_inputSnapshots.back().endPosition, 
                                          m_controlledEntityInterTimer.asSeconds(), m_inputRate.asSeconds());
        }
    }
}

void GameClient::setupNextInterpolation()
{
    m_entityManager.copySnapshotData(&m_interSnapshot_it->entityManager);

    //we need the end position of controlled entity in the server for this snapshot
    C_TestCharacter* snapshotEntity =  m_interSnapshot_it->entityManager.m_characters.atUniqueId(m_interSnapshot_it->entityManager.m_controlledEntityUniqueId);
    C_TestCharacter* controlledEntity = m_entityManager.m_characters.atUniqueId(m_entityManager.m_controlledEntityUniqueId);

    if (snapshotEntity && controlledEntity) {
        checkServerInput(m_interSnapshot_it->latestAppliedInput, snapshotEntity->pos, controlledEntity->movementSpeed);
    }
}

void GameClient::handleInput(const sf::Event& event, bool focused)
{
    if (!m_connected) return;

    if (focused) {
        PlayerInput_handleKeyboardInput(m_currentInput, event);
    } else {
        PlayerInput_clearKeys(m_currentInput);
    }
}

void GameClient::saveCurrentInput()
{
    C_TestCharacter* entity = m_entityManager.m_characters.atUniqueId(m_entityManager.m_controlledEntityUniqueId);

    //@TODO: Should we send inputs even if there's no entity
    //to ensure players can move the entity as soon as available?
    if (!entity) return;

    //we don't want to modify entity position just yet (we want to interpolate it smoothly)
    Vector2 entityPos = entity->pos;

    //apply input using the previous input position if possible
    if (!m_inputSnapshots.empty()) {
        entityPos = m_inputSnapshots.back().endPosition;
    }

    PlayerInput_applyInput(m_currentInput, entityPos, entity->movementSpeed, m_inputRate);
    
    //@TODO: Check for collisions

    //send this input
    {
        CRCPacket outPacket;
        outPacket << (u8) ServerCommand::PlayerInput;
        PlayerInput_packData(m_currentInput, outPacket);
        sendPacket(outPacket, m_serverConnectionId, false);
    }

    m_inputSnapshots.push_back(InputSnapshot());
    m_inputSnapshots.back().input = m_currentInput;
    m_inputSnapshots.back().endPosition = entityPos;

    //reset input timer
    m_currentInput.timeApplied = sf::Time::Zero;
    m_controlledEntityInterTimer = sf::Time::Zero;

    m_currentInput.id++;
}

void GameClient::checkServerInput(u32 inputId, const Vector2& endPosition, u16 movementSpeed)
{
    if (inputId == 0) {
        if (!m_inputSnapshots.empty()) {
            m_inputSnapshots.back().endPosition = endPosition;
        }

        return;
    }

    auto it = m_inputSnapshots.begin();

    //find the snapshot corresponding to inputId (while also deleting older ids)
    while (it != m_inputSnapshots.end()) {
        //we want to leave at least two inputs to interpolate properly
        if (it->input.id < inputId && m_inputSnapshots.size() > 2) {
            it = m_inputSnapshots.erase(it);
        } else {
            if (it->input.id == inputId) {
                break;
            }

            it = std::next(it);
        }
    }

    //if there's no input, that means it was already checked
    if (it == m_inputSnapshots.end()) return;

    Vector2 predictedEndPos = it->endPosition;

    //we want to leave at least two inputs to interpolate properly
    if (m_inputSnapshots.size() > 2) {
        it = m_inputSnapshots.erase(it);
    }

    //we correct for even the tiniest of differences, 
    //to make sure floating point error doesn't escalate
    if (predictedEndPos != endPosition) {
        printMessage("Incorrect prediction - Delta: %f", Helper_vec2length(predictedEndPos - endPosition));

        Vector2 newPos = endPosition;

        // recalculate all the positions of all the inputs starting from this one
        while (it != m_inputSnapshots.end()) {
            PlayerInput_repeatAppliedInput(it->input, newPos, movementSpeed);
            it->endPosition = newPos;

            it = std::next(it);
        }
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

            u32 appliedPlayerInputId;
            packet >> appliedPlayerInputId;

            u32 controlledEntityUniqueId;
            packet >> controlledEntityUniqueId;

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
            snapshot.latestAppliedInput = appliedPlayerInputId;
            snapshot.entityManager.m_controlledEntityUniqueId = controlledEntityUniqueId;

            removeOldSnapshots(prevSnapshotId);

            //populate C_EntityManager if we had no previous snapshots
            //(this happens when we receive the first snapshot)
            if (m_snapshots.size() == 1) {
                m_interSnapshot_it = m_snapshots.begin();
                setupNextInterpolation();
            }

            //send latest snapshot id
            CRCPacket outPacket;
            writeLatestSnapshotId(outPacket);
            sendPacket(outPacket, m_serverConnectionId, false);

            break;
        }
    }
}

void GameClient::removeOldSnapshots(u32 olderThan)
{
    auto it = m_snapshots.begin();

    //only erase elements that no longer need to be rendered
    while(it != m_interSnapshot_it) {
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

void GameClient::writeLatestSnapshotId(CRCPacket& packet)
{
    packet << (u8) ServerCommand::LatestSnapshotId;
    packet << m_snapshots.back().id;
}
