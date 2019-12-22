#include "crcpacket.hpp"

const void* CRCPacket::onSend(std::size_t &size)
{
    const char* buf = static_cast<const char*>(getData());
    std::size_t bufSize = getDataSize();

    const u32 crc32 = dh::crc(buf, buf + bufSize);

    size = sizeof(crc32) + bufSize;

    append(&crc32, sizeof(crc32));

    return getData();
}

//The CRC32 key is positioned at keyPos (the end)
//0101...0111 [KEY]
//If the keys match we append everything but the key

void CRCPacket::onReceive(const void* data, std::size_t size)
{
    const char* buf = static_cast<const char*>(data);
    const size_t keyPos = size - 4;

    u32 receivedKey = 0;
    std::memcpy(&receivedKey, buf + keyPos, 4);

    const u32 trueKey = dh::crc(buf, buf + keyPos);

    //This is true unless data is corrupted
    if (trueKey == receivedKey) {
        append(static_cast<const void*>(&buf[0]), keyPos);

    } else {
        append(NULL, 0);
        std::cerr << "Packet data is corrupted" << std::endl;
    }
}
