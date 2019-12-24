#pragma once

#include <SFML/Network/Packet.hpp>
#include <cstring>
#include <cwchar>
#include <iostream>

//Network packet that implements the CRC32 algorithm
//To prevent corruption of the data the algorithm must be also applied on receive and checked against the received value

class CRCPacket : public sf::Packet
{
public:
    virtual const void *onSend(std::size_t &size);
    virtual void onReceive(const void* data, std::size_t size);
};
