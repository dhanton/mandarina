#include "game_server.hpp"

#include <csignal>

#include "network_commands.hpp"

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
                int clientId = parent->mClients_clientId[index];

                if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) {
                    parent->printMessage("Connection with client %d closed by peer", clientId);

                } else {
                    parent->printMessage("Connection with client %d lost", clientId);
                }

                parent->removeClient(index);
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
                parent->mClients_connectionId[index] = info->m_hConn;

                int clientId = parent->mClients_clientId[index];

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
            int index = parent->getIndexByConnectionId(info->m_hConn);

            if (index != -1) {
                parent->printMessage("Connection completed with client %d", parent->mClients_clientId[index]);
            }

            break;
        }
    }
}

bool GameServer::SIGNAL_SHUTDOWN = false;

GameServer::GameServer(const Context& context, int partyNumber):
    m_gameServerCallbacks(this),
    InContext(context),
    NetPeer(&m_gameServerCallbacks, true),

    //TODO: Initial size should be the expected value of the amount of clients that will try to connect
    //This can be hight/low independently of partyNumber 
    //for example if a streamer sets up a server maybe 10k people try to connect (even for a 2-man party)
    //Still has to always be larger than partyNumber
    INITIAL_CLIENTS_SIZE(partyNumber * 2 + 10)
{
    m_gameStarted = false;
    m_partyNumber = partyNumber;
    m_firstInvalidIndex = 0;
    m_lastClientId = 0;

    if (!context.local) {
        m_endpoint.ParseString("127.0.0.1:7000");
        m_pollId = createListenSocket(m_endpoint);

    } else {
        m_isServer = false;
        m_pollId.listenSocket = context.localCon2;
        
        int index = addClient();
        mClients_connectionId[index] = context.localCon2;

        printMessage("Adding client in local connection");
    }

#ifndef _WIN32
    //Shutdown when Ctrl+C on linux
    struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = [] (int s) -> void {SIGNAL_SHUTDOWN = true;};
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);
#endif // _WIN32

    //Resize this vector to avoid dynamically adding elements
    //(this will still happen if more clients connect)
    _resizeClients_impl(INITIAL_CLIENTS_SIZE);
}

GameServer::~GameServer()
{
    for (int i = 0; i < m_firstInvalidIndex; ++i) {
        m_pInterface->CloseConnection(mClients_connectionId[i], 0, nullptr, false);
    }

    m_pInterface->CloseListenSocket(m_pollId.listenSocket);
    m_pInterface->DestroyPollGroup(m_pollId.pollGroup);
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

        for (int i = 0; i < m_firstInvalidIndex; ++i) {
            if (mClients_isReady[i]) {
                playersReady++;
            }
        }

        if (playersReady >= m_partyNumber) {
            m_gameStarted = true;

            printMessage("Game starting");

            //remove players not in party
            for (int i = mClients_clientId.size() - 1; i >= m_partyNumber; --i) {
                //close connection with players connected
                if (i < m_firstInvalidIndex) {
                    HSteamNetConnection connection = mClients_connectionId.back();
                    m_pInterface->CloseConnection(connection, 0, nullptr, false);
                }

                _popClient_impl();
            }

            //inform the players that the game is starting
            //start the main game level
        }
    }
    
    //Important to note that even if the game hasn't started yet
    //the idea is to be able to walk around and do a bunch of activities
    //with your character

    //So the game "lobby" is like a tavern for players to gather before the game
    //Players can perform an action to inform the server that they're ready
    
    //When all party members are ready the game starts 
    //All the other players are kicked out
}

void GameServer::processPacket(HSteamNetConnection connectionId, CRCPacket* packet)
{
    int index = getIndexByConnectionId(connectionId);

    if (index == -1) {
        printMessage("processPacket error - Invalid connection id %d", connectionId);
        return;
    }

    while (!packet->endOfPacket()) {
        u8 command = 0;
        *packet >> command;

        handleCommand(command, index, packet);
    }
}

void GameServer::handleCommand(u8 command, int index, CRCPacket* packet)
{
    switch (ServerCommand(command))
    {
        case ServerCommand::Null:
        {
            printMessage("handleCommand error - NULL command (client %d)", mClients_clientId[index]);

            //receiving null command invalidates the rest of the packet
            packet->clear();
            break;
        }

        case ServerCommand::PlayerReady:
        {
            bool ready;
            *packet >> ready;

            mClients_isReady[index] = ready;
            break;
        }
    }
}

int GameServer::addClient()
{
    if (m_firstInvalidIndex >= mClients_clientId.size()) {
        _pushClient_impl();
    }

    _resetClient_impl(m_firstInvalidIndex);
    mClients_clientId[m_firstInvalidIndex] = m_lastClientId++;

    return m_firstInvalidIndex++;
}

void GameServer::removeClient(int index)
{
    if (!isIndexValid(index)){
        printMessage("removeClient error - Invalid index %d", index);
        return;
    }

    if (index >= m_firstInvalidIndex) {
        printMessage("removeClient error - Trying to remove nonexistent client");
        return;
    }

    m_firstInvalidIndex--;

    std::swap(mClients_clientId[index], mClients_clientId[m_firstInvalidIndex]);
    std::swap(mClients_connectionId[index], mClients_connectionId[m_firstInvalidIndex]);
    std::swap(mClients_displayName[index], mClients_displayName[m_firstInvalidIndex]);
    std::swap(mClients_isReady[index], mClients_isReady[m_firstInvalidIndex]);
    std::swap(mClients_teamId[index], mClients_teamId[m_firstInvalidIndex]);
}

bool GameServer::addClientToPoll(int index)
{
    if (!isIndexValid(index)) {
        printMessage("addToPoll error - Invalid index %d", index);
        return false;
    }

    return m_pInterface->SetConnectionPollGroup(mClients_connectionId[index], m_pollId.pollGroup);
}

int GameServer::getIndexByConnectionId(HSteamNetConnection connectionId) const
{
    for (int i = 0; i < mClients_connectionId.size(); ++i) {
        if (mClients_connectionId[i] == connectionId) {
            return i;
        }
    }

    return -1;
}

bool GameServer::isIndexValid(int index) const
{
    return (index >= 0 && index < mClients_clientId.size());
}

void GameServer::_popClient_impl()
{
    mClients_clientId.pop_back();
    mClients_connectionId.pop_back();
    mClients_displayName.pop_back();
    mClients_isReady.pop_back();
    mClients_teamId.pop_back();
}

void GameServer::_pushClient_impl()
{
    mClients_clientId.push_back(-1);
    mClients_connectionId.push_back(k_HSteamNetConnection_Invalid);
    mClients_displayName.push_back("");
    mClients_isReady.push_back(false);
    mClients_teamId.push_back(0);
}

void GameServer::_resetClient_impl(int index)
{
    if (!isIndexValid(index)) {
        printMessage("resetClient error - Invalid index %d", index);
        return;
    }

    mClients_clientId[index] = -1;
    mClients_connectionId[index] = k_HSteamNetConnection_Invalid;
    mClients_displayName[index] = "Default";
    mClients_isReady[index] = false;
    mClients_teamId[index] = 0;
}

void GameServer::_resizeClients_impl(int size)
{
    mClients_clientId.resize(size, -1);
    mClients_connectionId.resize(size, k_HSteamNetConnection_Invalid);
    mClients_displayName.resize(size, "");
    mClients_isReady.resize(size, false);
    mClients_teamId.resize(size, 0);    
}