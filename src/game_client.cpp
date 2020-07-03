#include "game_client.hpp"

#include "network_commands.hpp"
#include "helper.hpp"
#include "game_mode_loader.hpp"

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
            parent->m_connected = false;

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
        }
    }
}

GameClient::GameClient(const Context& context, const SteamNetworkingIPAddr& endpoint):
    m_gameClientCallbacks(this),
    InContext(context),
    NetPeer(&m_gameClientCallbacks, false),
    m_entityManager(context),
    m_tileMapRenderer(context, &m_tileMap)
{
    // C_loadUnitsFromJson(context.jsonParser);
    C_loadProjectilesFromJson(context.jsonParser);

    m_context.renderTarget = &m_canvas;
    m_canvasCreated = false;

    m_entityManager.allocateAll();
    m_entityManager.setTileMap(&m_tileMap);
    
    m_interSnapshot_it = m_snapshots.begin();
    m_requiredSnapshotsToRender = 3;

    m_currentInput.id = 1;
    m_smoothUnitRadius = 20.f;

    m_forceFullSnapshotUpdate = false;
    m_fullUpdateReceived = false;

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
    }
}

GameClient::~GameClient()
{
    m_pInterface->CloseConnection(m_serverConnectionId, 0, nullptr, false);
}

void GameClient::mainLoop(bool& running)
{
    // sf::RenderWindow window{{1080, 720}, "Mandarina Prototype", sf::Style::Fullscreen};
    sf::RenderWindow window{{1080, 720}, "Mandarina Prototype", sf::Style::Titlebar | sf::Style::Close};
    
    sf::View view = window.getDefaultView();
    view.zoom(m_camera.getZoom());
    m_smoothUnitPos = view.getCenter();

    m_context.window = &window;
    m_context.view = &view;

    //clientCaster is a pointer because we need to pass it the updated Context (with window and view)
    //so it's construction must be here
    m_clientCaster = std::unique_ptr<ClientCaster>(new ClientCaster(m_context));

    m_camera.setView(&view);

    sf::Clock clock;
    
    //eTime shouldn't be 0 the first frame (weird glitches)
    sf::Time eTime = sf::milliseconds(5);

    sf::Time updateTimer;
    sf::Time inputTimer;

    bool focused = true;

    while (running) {
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
        window.setView(view);

        if (m_canvasCreated) {
            //draw everything to the canvas
            m_canvas.clear();

            m_tileMapRenderer.renderBeforeEntities(m_canvas);
            m_entityManager.renderingEntitiesUI = false;
            m_canvas.draw(m_entityManager);
            m_tileMapRenderer.renderAfterEntities(m_canvas);
            
            if (m_gameMode) {
                m_gameMode->draw(m_canvas, m_context.textures);
            }

            m_canvas.display();

            //draw the canvas to the window
            sf::Sprite sprite(m_canvas.getTexture());
            window.draw(sprite);

            //draw entity UIs
            m_entityManager.renderingEntitiesUI = true;
            window.draw(m_entityManager);

            //draw UI elements
            window.draw(*m_clientCaster);
            
            if (m_gameMode && m_gameMode->hasGameEnded()) {
                m_gameMode->drawGameEndInfo(window, m_context.fonts);
            }
        }

        window.display();

        eTime = clock.restart();
    }
}

void GameClient::receiveLoop()
{
    NetPeer::receiveLoop(m_serverConnectionId);
}

void GameClient::update(sf::Time eTime)
{
    m_infoTimer += eTime;

    if (m_infoTimer >= sf::seconds(5.f)) {
        SteamNetworkingQuickConnectionStatus status;
        SteamNetworkingSockets()->GetQuickConnectionStatus(m_serverConnectionId, &status);

        printMessage("-------------------------");
        printMessage("Ping: %d", status.m_nPing);
        printMessage("In/s: %f", status.m_flInBytesPerSec);
        printMessage("Out/s: %f", status.m_flOutBytesPerSec);

        m_infoTimer = sf::Time::Zero;
    }

    if (m_gameMode) {
        m_gameMode->C_onUpdate(eTime);
    }

    m_entityManager.update(eTime);

    m_clientCaster->setSpectating(m_entityManager.isHeroDead());
    
    if (!m_clientCaster->getSpectating()) {
        m_clientCaster->update(eTime, m_gameMode.get());
    }

    if (m_connected) {
        CRCPacket outPacket;
        writeLatestSnapshotId(outPacket);

        sendPacket(outPacket, m_serverConnectionId, false);
    }
}

void GameClient::renderUpdate(sf::Time eTime)
{
    C_Entity* entity = m_entityManager.entities.atUniqueId(m_entityManager.getControlledEntityUniqueId());

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

            //the entity might have been destroyed while setting up the next interpolation
            entity = m_entityManager.entities.atUniqueId(m_entityManager.getControlledEntityUniqueId());
        }

        m_entityManager.performInterpolation(&m_interSnapshot_it->entityManager, &next_it->entityManager, 
                                             m_interElapsed.asSeconds(), totalTime.asSeconds());

        //interpolate the controlled entity between the latest two inputs
        if (entity) {
            if (!m_inputSnapshots.empty()) {
                m_controlledEntityInterTimer += eTime;

                //interpolate using current entity position if there's only one input available
                Vector2 oldPos = entity->getPosition();

                auto oldIt = std::next(m_inputSnapshots.end(), -2);

                if (oldIt != m_inputSnapshots.end()) {
                    oldPos = oldIt->endPosition;
                }

                Vector2 newPos = Helper_lerpVec2(oldPos, m_inputSnapshots.back().endPosition, 
                                                 m_controlledEntityInterTimer.asSeconds(), m_inputRate.asSeconds());
                entity->setPosition(newPos);
            }

            Vector2i mousePixel = sf::Mouse::getPosition(*m_context.window);
            Vector2 mousePos = m_camera.mapPixelToCoords(mousePixel);

            PlayerInput_updateAimAngle(m_currentInput, entity->getPosition(), mousePos);
            entity->updateControlledAngle(m_currentInput.aimAngle);
        }
    }

    //follow the entity with the camera (either controlled or spectating)
    if (entity) {
        Vector2 pos = entity->getPosition();

        if (Helper_vec2length(m_smoothUnitPos - pos) > m_smoothUnitRadius) {
            float speed = (float) entity->getControlledMovementSpeed();

            m_smoothUnitPos = pos;

            //anything <= smoothRadius/speed causes jittering
            m_camera.snapSmooth(pos, sf::seconds(m_smoothUnitRadius/speed * 1.5f));
        }
    } else {
        entity = m_entityManager.entities.atUniqueId(m_entityManager.getLocalUniqueId());

        if (entity) {
            //this could be smoother but it's fine for now
            m_camera.snapInstant(entity->getPosition());
        }
    }

    m_camera.renderUpdate(eTime);

    //calling this from here might be a performance hit
    m_entityManager.updateRevealedUnits();

    m_entityManager.renderUpdate(eTime);
}

void GameClient::setupNextInterpolation()
{
    const u32 controlledEntityId = m_entityManager.getControlledEntityUniqueId();
    const u32 snapshotEntityId = m_interSnapshot_it->entityManager.getControlledEntityUniqueId();

    //Update the abilities if the controlled entity changes (also in the first iteration)
    if (!m_clientCaster->getCaster() || (controlledEntityId != snapshotEntityId)) {
        //if for some reason controlled entity is not a unit the ClientCaster will
        //receive nullptr and nothing will change
        C_Unit* controlledUnit = dynamic_cast<C_Unit*>(m_entityManager.entities.atUniqueId(controlledEntityId));
        
        if (controlledUnit) {
            m_clientCaster->setCaster(controlledUnit, m_gameMode.get());
            controlledUnit->getUnitUI()->setClientCaster(m_clientCaster.get());
            controlledUnit->getHealthUI()->setIsControlledEntity(true);
        }
    }

    m_entityManager.copySnapshotData(&m_interSnapshot_it->entityManager, m_interSnapshot_it->latestAppliedInput);

    //we need the end position of controlled entity in the server for this snapshot
    C_Entity* snapshotEntity = m_interSnapshot_it->entityManager.entities.atUniqueId(snapshotEntityId);
    C_Entity* controlledEntity = m_entityManager.entities.atUniqueId(controlledEntityId);

    if (snapshotEntity && controlledEntity) {
        checkServerInput(m_interSnapshot_it->latestAppliedInput, snapshotEntity->getPosition(), 
                         controlledEntity->getControlledMovementSpeed(), m_interSnapshot_it->caster);
    }
}

void GameClient::handleInput(const sf::Event& event, bool focused)
{
    if (!m_connected) return;

    if (focused) {
#ifdef MANDARINA_DEBUG
        if (event.type == sf::Event::KeyPressed) {
            //render collision shapes
            if (event.key.code == sf::Keyboard::F1) {
                m_entityManager.renderingDebug = !m_entityManager.renderingDebug;
            }

            //check camera free view
            if (event.key.code == sf::Keyboard::F2) {
                m_camera.changeState();

                if (m_camera.isFreeView()) {
                    PlayerInput_clearKeys(m_currentInput);
                }
            }

            //render locally hidden entities
            if (event.key.code == sf::Keyboard::F3) {
                m_entityManager.renderingLocallyHidden = !m_entityManager.renderingLocallyHidden;
            }

            //render some data above each entity
            if (event.key.code == sf::Keyboard::F4) {
                m_entityManager.renderingEntityData = !m_entityManager.renderingEntityData;
            }
        }

        if (m_camera.isFreeView()) {
            m_camera.handleInput(event);

        } else {
#endif

        PlayerInput_handleInput(m_currentInput, event);

#ifdef MANDARINA_DEBUG
        }
#endif

    } else {
        PlayerInput_clearKeys(m_currentInput);
    }
}

void GameClient::saveCurrentInput()
{
    C_Entity* entity = m_entityManager.entities.atUniqueId(m_entityManager.getControlledEntityUniqueId());

    //@TODO: Should we send inputs even if there's no entity
    //to ensure players can move the entity as soon as available?
    if (!entity) return;

    //we don't want to modify entity position just yet (we want to interpolate it smoothly)
    Vector2 entityPos = entity->getPosition();

    //apply input using the previous input position if possible
    if (!m_inputSnapshots.empty()) {
        entityPos = m_inputSnapshots.back().endPosition;
    }

    C_EntityManager* manager = &m_entityManager;

    //use the most recent EntityManager available
    //to check for collisions more accurately
    if (!m_snapshots.empty()) {
        manager = &m_snapshots.back().entityManager;
    }

    //we dont modify the unit since we intepolate its position
    //between two inputs (result is stored in entityPos)
    entity->applyMovementInput(entityPos, m_currentInput, C_ManagersContext(manager, &m_tileMap, m_gameMode.get()), m_inputRate);

    //when casting abilities we use the normal entity manager
    //since local entities might be created
    m_clientCaster->applyInputs(m_currentInput, entityPos, C_ManagersContext(&m_entityManager, &m_tileMap, m_gameMode.get()));

    //send this input
    if (m_connected) {
        CRCPacket outPacket;
        outPacket << (u8) ServerCommand::PlayerInput;
        PlayerInput_packData(m_currentInput, outPacket, static_cast<C_Unit*>(entity)->getStatus());
        sendPacket(outPacket, m_serverConnectionId, false);
    }

    m_inputSnapshots.push_back(InputSnapshot());
    m_inputSnapshots.back().input = m_currentInput;
    m_inputSnapshots.back().endPosition = entityPos;
    m_inputSnapshots.back().forceSnap = false;
    m_inputSnapshots.back().caster = m_clientCaster->takeSnapshot();

    //reset input timer
    m_currentInput.timeApplied = sf::Time::Zero;
    m_controlledEntityInterTimer = sf::Time::Zero;

    m_currentInput.id++;
}

void GameClient::checkServerInput(u32 inputId, const Vector2& endPosition, u16 movementSpeed, const CasterSnapshot& casterSnapshot)
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

    //Correct the position

    Vector2 predictedEndPos = it->endPosition;
    CasterSnapshot predictedCaster = it->caster;

    //we want to leave at least two inputs to interpolate properly
    if (m_inputSnapshots.size() > 2) {
        it = m_inputSnapshots.erase(it);
    }

    //correct wrong predictions
    if (predictedEndPos != endPosition) {

#if 0 && defined MANDARINA_DEBUG
        //this message can get annoying because there are a lot of minor prediction errors sometimes
        printMessage("Incorrect prediction - Delta: %f", Helper_vec2length(predictedEndPos - endPosition));
#endif

        Vector2 newPos = endPosition;
        C_Entity* entity = m_entityManager.entities.atUniqueId(m_entityManager.getControlledEntityUniqueId());

        //get the newest entityManager to perform collision better
        C_ManagersContext context(m_snapshots.empty() ? &m_entityManager : &m_snapshots.back().entityManager, &m_tileMap, m_gameMode.get());

        bool forceSnap = false;

        // recalculate all the positions of all the inputs starting from this one
        while (it != m_inputSnapshots.end()) {
            if (entity) {
                //repeat the input colliding with the map and other entities if possible
                entity->reapplyMovementInput(newPos, it->input, context);

                //apply only ability inputs that move the unit
                //@TODO: If the controlledEntity is not the unit this shouldn't be called really
                m_clientCaster->reapplyInputs(it->input, newPos, context);

            } else {
                //if there's no entity just repeat the input with no prediction
                PlayerInput_repeatAppliedInput(it->input, newPos, movementSpeed);
            }

            //forceSnap propagates to newer inputs
            if (it->forceSnap) {
                forceSnap = true;
            }

            //try to smoothly correct the input one step at a time
            //(this weird method is the one that gets the best results apparently)
            Vector2 dirVec = newPos - it->endPosition;
            float distance = Helper_vec2length(dirVec);

            //@TODO: These values (0.5, 10, 200) have to be tinkered to make it look as smooth as possible
            //The method used could also change if this one's not good enough
            if (!forceSnap && distance < 200.f) {
                float offset = std::max(0.5, Helper_lerp(0.0, 10.0, distance, 200.0));
                it->endPosition += Helper_vec2unitary(dirVec) * std::min(offset, distance);

            } else {
                it->endPosition = newPos;
            }

            it = std::next(it);
        }
    }

    //Correct the abilities
    if (predictedCaster.valid) {
        float delta = 0.015;
        CasterSnapshot diffSnapshot = casterSnapshot.getDiff(predictedCaster, delta);

        if (!diffSnapshot.isAllZero()) {
            while (it != m_inputSnapshots.end()) {
                it->caster.primaryPercentage += diffSnapshot.primaryPercentage;
                it->caster.secondaryPercentage += diffSnapshot.secondaryPercentage;
                it->caster.altPercentage += diffSnapshot.altPercentage;
                it->caster.ultimatePercentage += diffSnapshot.ultimatePercentage;

                it = std::next(it);
            }

            //Update ClientCaster using the diffSnapshot
            m_clientCaster->applyServerCorrection(diffSnapshot);
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

            bool forceFullUpdate;
            packet >> forceFullUpdate;

            Snapshot* prevSnapshot = findSnapshotById(prevSnapshotId);
            C_EntityManager* prevEntityManager = nullptr;

            m_forceFullSnapshotUpdate = forceFullUpdate;

            if (!prevSnapshot) {
                if (prevSnapshotId != 0) {
                    printMessage("Snapshot error - Previous snapshot doesn't exist (id %i)", prevSnapshotId);
                    packet.clear();
                    break;
                }

            } else {
                prevEntityManager = &prevSnapshot->entityManager;
            }

            m_snapshots.emplace_back();

            Snapshot& snapshot = m_snapshots.back();
            snapshot.id = snapshotId;
            snapshot.entityManager.setControlledEntityUniqueId(controlledEntityUniqueId);
            snapshot.entityManager.loadFromData(prevEntityManager, packet, snapshot.caster);
            snapshot.worldTime = m_worldTime;
            snapshot.latestAppliedInput = appliedPlayerInputId;

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

        case ClientCommand::InitialInfo:
        {
            u32 uniqueId;
            packet >> uniqueId;

            u8 teamId;
            packet >> teamId;

            m_entityManager.setControlledEntityUniqueId(uniqueId);
            m_entityManager.setControlledEntityTeamId(teamId);

            break;
        }

        case ClientCommand::PlayerCoords:
        {
            //initially the camera will be looking at the center of the map
            m_camera.snapInstant((Vector2) m_tileMap.getWorldSize()/2.f);

            Vector2 initialPos;
            packet >> initialPos.x >> initialPos.y;

            //then we move the camera smoothly towards the controlled unit
            m_camera.snapSmooth(initialPos, sf::seconds(1.5f), true);

            break;
        }

        case ClientCommand::GameModeType:
        {
            u8 gameModeType;
            packet >> gameModeType;

            bool started;
            packet >> started;

            m_gameMode = std::unique_ptr<GameMode>(GameModeLoader::create(gameModeType, m_context));
            m_gameMode->setTileMap(&m_tileMap);

            if (started) {
                m_gameMode->startGame();
                loadMap(m_gameMode->getMapFilename());

                m_gameMode->C_onGameStarted();
            } else {
                loadMap(m_gameMode->getLobbyMapFilename());
            }

            break;
        }

        case ClientCommand::GameStarted:
        {
            u32 uniqueId;
            packet >> uniqueId;

            u8 teamId;
            packet >> teamId;

            m_entityManager.setControlledEntityUniqueId(uniqueId);
            m_entityManager.setControlledEntityTeamId(teamId);

            m_gameMode->startGame();
            loadMap(m_gameMode->getMapFilename());

            m_gameMode->C_onGameStarted();

            //force ClientCaster to restart cooldown and ability info next update
            m_clientCaster->forceCasterUpdate();

            for (auto& inputSnapshot : m_inputSnapshots) {
                //This helps eliminate an interpolation artifact that happened
                //if distance between old position (in lobby) and new position was < 200
                inputSnapshot.forceSnap = true;
            }

            break;
        }

        case ClientCommand::ChangeSpectator:
        {
            u32 uniqueId;
            packet >> uniqueId;

            u8 teamId;
            packet >> teamId;

            m_entityManager.setSpectatingEntityUniqueId(uniqueId);

            if (teamId != m_entityManager.getLocalTeamId()) {
                m_entityManager.setSpectatingEntityTeamId(teamId);
            }

            break;
        }

        case ClientCommand::TeamEliminated:
        {
            //display team eliminated message (depends on game mode)

            break;
        }

        case ClientCommand::GameEnded:
        {
            m_gameMode->loadGameEndData(packet);

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

void GameClient::writeLatestSnapshotId(CRCPacket& packet)
{
    packet << (u8) ServerCommand::LatestSnapshotId;
    packet << (m_forceFullSnapshotUpdate ? 0 : m_snapshots.back().id);
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

void GameClient::loadMap(const std::string& filename)
{
    m_tileMap.loadFromFile(filename);
    m_tileMapRenderer.generateLayers();

    const Vector2u totalSize = m_tileMap.getWorldSize();

    m_canvas.create(totalSize.x, totalSize.y);
    m_canvasCreated = true;

    m_camera.setMapSize(totalSize);
}
