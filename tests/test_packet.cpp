#include "../include/defines.hpp"
#include "../include/packet.hpp"
#include <iostream>

#define ASSERT(CONDITION) if (!(CONDITION)) {\
        printf("Assertion failure %s:%d ASSERT(%s)\n", __FILE__, __LINE__, #CONDITION);\
    }

void check_multiple(int N)
{
    Packet packet;

    std::vector<bool> sent;
    std::vector<bool> received;

    for (int i = 0; i < N; ++i) {
        bool a = static_cast<bool>(rand() % 2);
        packet << a;

        sent.push_back(a);
    }

    for (int i = 0; i < N; ++i) {
        bool b;
        packet >> b;
        
        received.push_back(b);
    }
    
    ASSERT(sent == received)
    ASSERT(packet.endOfPacket())
}

void check_in_between()
{
    Packet packet;
    bool a;
    u8 byte;

    packet << true << false << (u8) 137 << false << true << false;

    packet >> a;
    ASSERT(a)

    packet >> a;
    ASSERT(!a)

    packet >> byte;
    ASSERT(byte == 137)

    packet >> a;
    ASSERT(!a)

    packet >> a;
    ASSERT(a)

    packet >> a;
    ASSERT(!a)

    ASSERT(packet.endOfPacket())
}

void check_string()
{
    Packet packet;
    bool a;
    std::string test;

    packet << true << true << false << std::string("testtest") << true << false << false;

    packet >> a;
    ASSERT(a)

    packet >> a;
    ASSERT(a)

    packet >> a;
    ASSERT(!a)

    packet >> test;
    ASSERT(test == "testtest")

    packet >> a;
    ASSERT(a)

    packet >> a;
    ASSERT(!a)

    packet >> a;
    ASSERT(!a)

    ASSERT(packet.endOfPacket())
}

void simple_size_test()
{
    Packet packet;

    packet << true << true << false << true;

    // Size is 1 byte (would've been 4 bytes with previous implementation)
    std::cout << packet.getDataSize() << std::endl;

    // It doesn't pack bits outside of booleans
    packet << sf::Int16(133) << true << false << false;

    // Size is 4 bytes now (would've been 9 bytes with previous implementation)
    std::cout << packet.getDataSize() << std::endl;
}

int main()
{
    srand(time(0));

    check_in_between();

    check_multiple(10);    
    check_multiple(100);    
    check_multiple(1000);    

    check_string();

    simple_size_test();
}
