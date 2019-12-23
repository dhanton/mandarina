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
        // case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        // case k_ESteamNetworkingConnectionState_Connecting:
        
        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        {
            parent->m_pInterface->CloseConnection(parent->m_serverConnectionId, 0, nullptr, false);
            parent->m_serverConnectionId = k_HSteamNetConnection_Invalid;

            std::cout << "Connection with server closed by peer" << std::endl;

            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
        {
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
    NetPeer(&m_gameClientCallbacks, false)
{
    if (!context.local) {
        m_serverConnectionId = connectToServer(endpoint);

    } else {
        m_serverConnectionId = context.localCon1;

        std::cout << "Adding server in local connection" << std::endl;
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

void GameClient::update(const sf::Time& eTime)
{
}

void GameClient::processPacket(HSteamNetConnection connectionId, CRCPacket* packet)
{
    while (!packet->endOfPacket()) {
        u8 command = 0;
        *packet >> command;

        handleCommand(command, packet);
    }
}

void GameClient::handleCommand(u8 command, CRCPacket* packet)
{
    switch (ClientCommand(command))
    {
        case ClientCommand::Null:
        {
            std::cerr << "handleCommand error - NULL command" << std::endl;

            //receiving null command invalidates the rest of the packet
            packet->clear();
            break;
        }
    }
}