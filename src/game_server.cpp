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
            int clientId = parent->getClientIdByConnectionId(info->m_hConn);

            if (clientId != -1) {
                parent->mClients_isValid[clientId] = false;

                if (!parent->m_gameStarted) {
                    parent->m_dataOrientedManager.itemIdRemoved(clientId);
                }
            }

            if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) {
                parent->printMessage("Connection with client %d closed by peer", clientId);

            } else {
                parent->printMessage("Connection with client %d lost", clientId);
            }

            parent->m_pInterface->CloseConnection(info->m_hConn, 0, nullptr, false);

            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            int clientId = -1;
            
            if (parent->m_gameStarted) {
                //traverse all clients
                //if one of them is not valid
                //and has the same identity (secret key)
                //then that one is reconnecting
            } else {
                clientId = parent->getFreeClientId();
            }

            if (clientId != -1) {
                parent->resetClient(clientId);
                parent->mClients_connectionId[clientId] = info->m_hConn;

                //Set the rest of the parameters (displayName, character, team, etc)

                if (parent->m_pInterface->AcceptConnection(info->m_hConn) != k_EResultOK) {
                    parent->printMessage("There was an error accepting connection with client %d", clientId);
                    parent->m_pInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
                    break;
                }

                if (!parent->addClientToPoll(clientId)) {
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
            int clientId = parent->getClientIdByConnectionId(info->m_hConn);

            if (clientId != -1) {
                parent->mClients_isValid[clientId] = true;

                parent->printMessage("Connection completed with client %d", clientId);
            }

            break;
        }
    }
}

bool GameServer::SIGNAL_SHUTDOWN = false;

GameServer::GameServer(const Context& context, int partyNumber):
    m_gameServerCallbacks(this),
    InContext(context),
    NetPeer(&m_gameServerCallbacks, true)
{
    m_gameStarted = false;
    m_partyNumber = partyNumber;

    if (!context.local) {
        m_endpoint.ParseString("127.0.0.1:7000");
        m_pollId = createListenSocket(m_endpoint);

    } else {
        m_isServer = false;
        m_pollId.listenSocket = context.localCon2;
        
        int clientId = getFreeClientId();
        mClients_isValid[clientId] = true;
        mClients_connectionId[clientId] = context.localCon2;

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
}

GameServer::~GameServer()
{
    for (int i = 0; i < mClients_isValid.size(); ++i) {
        if (mClients_isValid[i]) {
            m_pInterface->CloseConnection(mClients_connectionId[i], 0, nullptr, false);
        }
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
        int readyPlayers = 0;

        for (int i = 0; i < mClients_isValid.size(); ++i) {
            if (mClients_isValid[i] && mClients_ready[i]) {
                readyPlayers++;
            }
        }

        if (readyPlayers >= m_partyNumber) {
            m_gameStarted = true;

            printMessage("Game starting");

            //find all the party members
            //and move them at the start of each vector
            for (int i = 0; i < m_partyNumber; ++i) {
                for (int j = i; j < mClients_isValid.size(); ++j) {
                    if (mClients_isValid[j] && mClients_ready[j]) {
                        if (i != j) {
                            swapClientData(i, j);
                        }

                        break;
                    }
                }
            }

            //remove the rest of the players
            while (mClients_isValid.size() > m_partyNumber) {
                int n = mClients_connectionId.size() - 1;

                if (mClients_isValid[n]) {
                    m_pInterface->CloseConnection(mClients_connectionId[n], 0, nullptr, false);
                }

                mClients_isValid.pop_back();
                mClients_connectionId.pop_back();
                mClients_displayName.pop_back();
                mClients_ready.pop_back();
                mClients_team.pop_back();
            }
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
    int clientId = getClientIdByConnectionId(connectionId);

    if (clientId == -1) {
        printMessage("processPacket error - Invalid connection id");
        return;
    }

    while (!packet->endOfPacket()) {
        u8 command = 0;
        *packet >> command;

        handleCommand(command, clientId, packet);
    }
}

void GameServer::handleCommand(u8 command, int clientId, CRCPacket* packet)
{
    switch (ServerCommand(command))
    {
        case ServerCommand::Null:
        {
            printMessage("handleCommand error - NULL command (client %d)", clientId);

            //receiving null command invalidates the rest of the packet
            packet->clear();
            break;
        }

        case ServerCommand::PlayerReady:
        {
            bool ready;
            *packet >> ready;

            mClients_ready[clientId] = ready;
            break;
        }
    }
}

bool GameServer::addClientToPoll(int id)
{
    if (id < 0 || id >= mClients_isValid.size()) {
        printMessage("addToPoll error - Invalid id");
        return false;
    }

    return m_pInterface->SetConnectionPollGroup(mClients_connectionId[id], m_pollId.pollGroup);
}

void GameServer::resetClient(int id)
{
    if (id < 0 || id >= mClients_isValid.size()) {
        printMessage("resetClient error - Invalid id");
        return;
    }

    //Default values for client data
    mClients_isValid[id] = false;
    mClients_connectionId[id] = k_HSteamNetConnection_Invalid;
    mClients_displayName[id] = "Default";
    mClients_ready[id] = false;
    mClients_team[id] = 0;
}

int GameServer::getFreeClientId()
{
    int id = m_dataOrientedManager.getNewItemId();

    if (id < 0 || id > mClients_isValid.size()) {
        printMessage("getFreeClientId error - Invalid id");
        return -1;
    }

    if (id == mClients_isValid.size()) {
        //These are invalid values
        //resetClient should be called before using it
        mClients_isValid.push_back(false);
        mClients_connectionId.push_back(-1);
        mClients_displayName.push_back("");
        mClients_ready.push_back(false);
        mClients_team.push_back(0);
    }

    return id;
}

int GameServer::getClientIdByConnectionId(HSteamNetConnection connectionId) const
{
    for (int i = 0; i < mClients_isValid.size(); ++i) {
        if (mClients_connectionId[i] == connectionId) {
            return i;
        }
    }

    return -1;
}

void GameServer::swapClientData(int clientId1, int clientId2)
{
    if (clientId1 < 0 || clientId1 >= mClients_isValid.size()) {
        printMessage("swapClientData error - Invalid client 1 id");
        return;
    }

    if (clientId2 < 0 || clientId2 >= mClients_isValid.size()) {
        printMessage("swapClientData error - Invalid client 2 id");
        return;
    }

    std::swap(mClients_isValid[clientId1], mClients_isValid[clientId2]);
    std::swap(mClients_connectionId[clientId1], mClients_connectionId[clientId2]);
    std::swap(mClients_displayName[clientId1], mClients_displayName[clientId2]);
    std::swap(mClients_ready[clientId1], mClients_ready[clientId2]);
}