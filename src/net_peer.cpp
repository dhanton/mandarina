#include "net_peer.hpp"

#include <cstdarg>

const NetPeer::PollId NetPeer::PollId::Invalid = PollId(k_HSteamNetPollGroup_Invalid, k_HSteamListenSocket_Invalid);

NetPeer::PollId::PollId()
{
    pollGroup = k_HSteamNetPollGroup_Invalid;
    listenSocket = k_HSteamListenSocket_Invalid;
}

NetPeer::PollId::PollId(HSteamNetPollGroup pollGroup, HSteamListenSocket listenSocket)
{
    this->pollGroup = pollGroup;
    this->listenSocket = listenSocket;
}

bool NetPeer::PollId::operator==(const NetPeer::PollId& other) const
{
    return other.pollGroup == pollGroup && other.listenSocket == listenSocket;
}

bool NetPeer::PollId::operator!=(const NetPeer::PollId& other) const
{
    return !(*this == other);
}

NetPeer::NetPeer(ISteamNetworkingSocketsCallbacks *callbacks, bool server):
    m_isServerMsg(server)
{
    m_callbacks = callbacks;
    m_isServer = server;

    m_pInterface = SteamNetworkingSockets();
}

HSteamNetConnection NetPeer::connectToServer(const SteamNetworkingIPAddr &endpoint)
{
    if (m_isServer) {
        printMessage("Can't connect to another server");
        return k_HSteamNetConnection_Invalid;
    }

    printMessage("Trying to reach server");

    return m_pInterface->ConnectByIPAddress(endpoint, 0, nullptr);
}

NetPeer::PollId NetPeer::createListenSocket(const SteamNetworkingIPAddr &endpoint)
{
    if (!m_isServer) {
        printMessage("Only servers can create listen sockets");
        return PollId::Invalid;
    }

    //PollGroup is used to receive data
    HSteamNetPollGroup pollGroup = m_pInterface->CreatePollGroup();
    printMessage("PollGroup created");

    //ListenSocket is used to allow clients to connect
    HSteamListenSocket connection = m_pInterface->CreateListenSocketIP(endpoint, 0, nullptr);
    printMessage("ListenSocket created");

    return PollId(pollGroup, connection);
}

void NetPeer::checkConnectionStatus(SteamNetworkingQuickConnectionStatus &status, HSteamNetConnection connectionId)
{
    m_pInterface->GetQuickConnectionStatus(connectionId, &status);
}

void NetPeer::sendPacket(CRCPacket &packet, HSteamNetConnection connectionId, bool reliable)
{
    size_t dataSize = packet.getDataSize();

    //We used shared pointer to ensure the message lives while its being sent
    //TODO: Test if this is still needed
    auto packet_ptr = std::make_shared<CRCPacket>(packet);

    //TODO: Change message type to unreliable no delay if delay is high
    //Those 4 extra bytes are the CRC Key (that's not included in dataSize)
    EResult result = m_pInterface->SendMessageToConnection(connectionId, packet.onSend(dataSize), dataSize + 4,
                                                                       reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable,
                                                                       nullptr);

    if (result != k_EResultOK) {
        printMessage("Error (%d) sending CRCPacket. Size: %u bytes.", result, dataSize);
    }
}

void NetPeer::receiveLoop(u32 id)
{
    while (true) {
        if (m_callbacks != nullptr) {
            m_pInterface->RunCallbacks(m_callbacks);
        }

        ISteamNetworkingMessage *msg = nullptr;
        int msgNum = 0;

        if (m_isServer) {
            msgNum = m_pInterface->ReceiveMessagesOnPollGroup(id, &msg, 1);
        } else {
            msgNum = m_pInterface->ReceiveMessagesOnConnection(id, &msg, 1);
        }

        if (msgNum <= 0) return;

        HSteamNetConnection senderId = msg->GetConnection();

        CRCPacket* inPacket = new CRCPacket();
        inPacket->onReceive(msg->GetData(), msg->GetSize());

        processPacket(senderId, inPacket);

        msg->Release();
    }
}

void NetPeer::printMessage(const char* format, ...) const
{
    va_list vl;
    va_start(vl, format);

    printf(m_isServerMsg ?  "[server] " : "[client] ");
    vprintf(format, vl);
    printf("\n");

    va_end(vl);
}