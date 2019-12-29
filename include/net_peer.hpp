#pragma once

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

    virtual void processPacket(HSteamNetConnection connectionId, CRCPacket& packet) = 0;

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

    void printMessage(const char* format, ...) const;

    ISteamNetworkingSocketsCallbacks* m_callbacks;
    ISteamNetworkingSockets* m_pInterface;
    bool m_isServer;

    //When using local connection isServer = false in server as well
    //but we still want to display messages with [SERVER] prefix
    const bool m_isServerMsg;
};
