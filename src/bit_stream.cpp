#include "bit_stream.hpp"

#include <iostream>

BitStream::BitStream()
{
    clear();
}

void BitStream::pushByte(u8 byte)
{
    if (m_currentIndex > 56) {
        std::cerr << "pushByte error - Maximum size of 64 bits exceeded." << std::endl;
        return;
    }

    m_stream |= (static_cast<size_t>(byte) << m_currentIndex);
    m_currentIndex += 8;
}

void BitStream::pushBit(bool bit)
{
    if (m_currentIndex > 64) {
        std::cerr << "pushBit error - Maximum size of 64 bits exceeded." << std::endl;
        return;
    }

    m_stream |= (bit ? (1 << m_currentIndex) : 0);
    m_currentIndex++;
}

u8 BitStream::popByte()
{
    if (m_currentIndex == 0) {
        throw std::runtime_error("popByte error - Can't read data from empty bit stream.");
    }

    u8 byte = m_stream & 0xff;

    m_currentIndex = std::max(0, static_cast<int>(m_currentIndex) - 8);
    m_stream >>= 8;

    return byte;
}

bool BitStream::popBit()
{
    if (m_currentIndex == 0) {
        throw std::runtime_error("popBit error - Can't read data from empty bit stream.");
    }

    int bit = m_stream & 1;

    m_currentIndex--;
    m_stream >>= 1;

    return static_cast<bool>(bit);
}

bool BitStream::endOfStream() const
{
    return (m_currentIndex == 0);
}

void BitStream::clear()
{
    m_stream = 0;
    m_currentIndex = 0;
}
