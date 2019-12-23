#ifndef NET_PEER_HPP
#define NET_PEER_HPP

#include <iostream>
#include <thread>
#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include "defines.hpp"
#include "crcpacket.hpp"

class NetPeer
{
public:
    struct PollId {
        PollId();
        PollId(HSteamNetPollGroup pollGroup, HSteamListenSocket listenSocket);

        HSteamNetPollGroup pollGroup;
        HSteamListenSocket listenSocket;

        const static PollId Invalid;

        bool operator==(const PollId& other) const;
        bool operator!=(const PollId& other) const;
    };

public:
    NetPeer(ISteamNetworkingSocketsCallbacks* callbacks, bool server);

    virtual void processPacket(HSteamNetConnection connectionId, CRCPacket* packet) = 0;

    //Store returns of these functions (in child class for example)
    //Important to send/receive messages
    HSteamNetConnection connectToServer(const SteamNetworkingIPAddr& endpoint);

    //Also creates a poll group, and returns both ids
    PollId createListenSocket(const SteamNetworkingIPAddr& endpoint);

    void checkConnectionStatus(SteamNetworkingQuickConnectionStatus& status, HSteamNetConnection connectionId);
    void sendPacket(CRCPacket& packet, HSteamNetConnection connectionId, bool reliable);

protected:
    //wrap it in child classes
    void receiveLoop(u32 id);

    ISteamNetworkingSocketsCallbacks* m_callbacks;
    ISteamNetworkingSockets* m_pInterface;
    bool m_isServer;
};

#endif // NET_PEER_HPP
