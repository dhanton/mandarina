#include "crcpacket.hpp"

#include <array>
#include <algorithm>
#include <numeric>

#include "defines.hpp"

std::array<uint_fast32_t, 256> generate_crc_lookup_table() noexcept
{
    auto const reversed_polynomial = std::uint_fast32_t{0xEDB88320uL};

    struct byte_checksum
    {
        std::uint_fast32_t operator()() noexcept
        {
            auto checksum = static_cast<std::uint_fast32_t>(n++);

            for (auto i = 0; i < 8; ++i)
                checksum = (checksum >> 1) ^ ((checksum & 0x1u) ? reversed_polynomial : 0);

            return checksum;
        }

        unsigned n = 0;
    };

    auto table = std::array<std::uint_fast32_t, 256>{};
    std::generate(table.begin(), table.end(), byte_checksum{});

    return table;
}

template<typename InputIterator>
uint_fast32_t crc(InputIterator first, InputIterator last)
{
    //Generate table only the first time this is called
    static auto const table = generate_crc_lookup_table();

    return std::uint_fast32_t{0xFFFFFFFFuL} &
        ~std::accumulate(first, last,
          ~std::uint_fast32_t{0} & std::uint_fast32_t{0xFFFFFFFFuL},
            [](std::uint_fast32_t checksum, std::uint_fast8_t value)
              { return table[(checksum ^ value) & 0xFFu] ^ (checksum >> 8); });
}

const void* CRCPacket::onSend(std::size_t &size)
{
    const char* buf = static_cast<const char*>(getData());
    std::size_t bufSize = getDataSize();

    const u32 crc32 = crc(buf, buf + bufSize);

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

    const u32 trueKey = crc(buf, buf + keyPos);

    //This is true unless data is corrupted
    if (trueKey == receivedKey) {
        append(static_cast<const void*>(&buf[0]), keyPos);

    } else {
        append(NULL, 0);
        std::cerr << "CRCPacket receive error - Packet data is corrupted" << std::endl;
    }
}
