#include "packet.hpp"

#include <cstring>
#include <cwchar>

//ntohs function (converts bytes from TCP/IP network order to host order)
#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif


Packet::Packet() :
m_readPos(0),
m_sendPos(0),
m_isValid(true),
m_boolReadPos(0),
m_boolSendPos(0)
{

}

Packet::~Packet()
{

}

void Packet::append(const void* data, std::size_t sizeInBytes)
{
    if (data && (sizeInBytes > 0))
    {
        std::size_t start = m_data.size();
        m_data.resize(start + sizeInBytes);
        std::memcpy(&m_data[start], data, sizeInBytes);
        
        m_boolSendPos = 0;
    }
}

void Packet::clear()
{
    m_data.clear();
    m_readPos = 0;
    m_isValid = true;

    m_boolReadPos = 0;
    m_boolSendPos = 0;
}

const void* Packet::getData() const
{
    return !m_data.empty() ? &m_data[0] : NULL;
}

std::size_t Packet::getDataSize() const
{
    return m_data.size();
}

bool Packet::endOfPacket() const
{
    return m_readPos >= m_data.size();
}

Packet::operator BoolType() const
{
    return m_isValid ? &Packet::checkSize : NULL;
}

Packet& Packet::operator >>(bool& data)
{
    if (m_boolReadPos == 0)
    {
        //only check data size for the first bit
        if (!checkSize(sizeof(data))) return *this;

        m_readPos += 1;
    }

    sf::Uint8 byte = *reinterpret_cast<const sf::Uint8*>(&m_data[m_readPos - 1]);

    data = (1 << m_boolReadPos) & byte;

    m_boolReadPos = (m_boolReadPos + 1) % 8;

    return *this;
}

Packet& Packet::operator >>(sf::Int8& data)
{
    if (checkSize(sizeof(data)))
    {
        data = *reinterpret_cast<const sf::Int8*>(&m_data[m_readPos]);
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(sf::Uint8& data)
{
    if (checkSize(sizeof(data)))
    {
        data = *reinterpret_cast<const sf::Uint8*>(&m_data[m_readPos]);
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(sf::Int16& data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohs(*reinterpret_cast<const sf::Int16*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(sf::Uint16& data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohs(*reinterpret_cast<const sf::Uint16*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(sf::Int32& data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohl(*reinterpret_cast<const sf::Int32*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(sf::Uint32& data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohl(*reinterpret_cast<const sf::Uint32*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(sf::Int64& data)
{
    if (checkSize(sizeof(data)))
    {
        // Since ntohll is not available everywhere, we have to convert
        // to network byte order (big endian) manually
        const sf::Uint8* bytes = reinterpret_cast<const sf::Uint8*>(&m_data[m_readPos]);
        data = (static_cast<sf::Int64>(bytes[0]) << 56) |
               (static_cast<sf::Int64>(bytes[1]) << 48) |
               (static_cast<sf::Int64>(bytes[2]) << 40) |
               (static_cast<sf::Int64>(bytes[3]) << 32) |
               (static_cast<sf::Int64>(bytes[4]) << 24) |
               (static_cast<sf::Int64>(bytes[5]) << 16) |
               (static_cast<sf::Int64>(bytes[6]) <<  8) |
               (static_cast<sf::Int64>(bytes[7])      );
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(sf::Uint64& data)
{
    if (checkSize(sizeof(data)))
    {
        // Since ntohll is not available everywhere, we have to convert
        // to network byte order (big endian) manually
        const sf::Uint8* bytes = reinterpret_cast<const sf::Uint8*>(&m_data[m_readPos]);
        data = (static_cast<sf::Uint64>(bytes[0]) << 56) |
               (static_cast<sf::Uint64>(bytes[1]) << 48) |
               (static_cast<sf::Uint64>(bytes[2]) << 40) |
               (static_cast<sf::Uint64>(bytes[3]) << 32) |
               (static_cast<sf::Uint64>(bytes[4]) << 24) |
               (static_cast<sf::Uint64>(bytes[5]) << 16) |
               (static_cast<sf::Uint64>(bytes[6]) <<  8) |
               (static_cast<sf::Uint64>(bytes[7])      );
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(float& data)
{
    if (checkSize(sizeof(data)))
    {
        data = *reinterpret_cast<const float*>(&m_data[m_readPos]);
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(double& data)
{
    if (checkSize(sizeof(data)))
    {
        data = *reinterpret_cast<const double*>(&m_data[m_readPos]);
        m_readPos += sizeof(data);
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(char* data)
{
    // First extract string length
    sf::Uint32 length = 0;
    *this >> length;

    if ((length > 0) && checkSize(length))
    {
        // Then extract characters
        std::memcpy(data, &m_data[m_readPos], length);
        data[length] = '\0';

        // Update reading position
        m_readPos += length;
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(std::string& data)
{
    // First extract string length
    sf::Uint32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length))
    {
        // Then extract characters
        data.assign(&m_data[m_readPos], length);

        // Update reading position
        m_readPos += length;
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(wchar_t* data)
{
    // First extract string length
    sf::Uint32 length = 0;
    *this >> length;

    if ((length > 0) && checkSize(length * sizeof(sf::Uint32)))
    {
        // Then extract characters
        for (sf::Uint32 i = 0; i < length; ++i)
        {
            sf::Uint32 character = 0;
            *this >> character;
            data[i] = static_cast<wchar_t>(character);
        }
        data[length] = L'\0';
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(std::wstring& data)
{
    // First extract string length
    sf::Uint32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length * sizeof(sf::Uint32)))
    {
        // Then extract characters
        for (sf::Uint32 i = 0; i < length; ++i)
        {
            sf::Uint32 character = 0;
            *this >> character;
            data += static_cast<wchar_t>(character);
        }
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator >>(sf::String& data)
{
    // First extract the string length
    sf::Uint32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length * sizeof(sf::Uint32)))
    {
        // Then extract characters
        for (sf::Uint32 i = 0; i < length; ++i)
        {
            sf::Uint32 character = 0;
            *this >> character;
            data += character;
        }
        m_boolReadPos = 0;
    }

    return *this;
}

Packet& Packet::operator <<(bool data)
{
    if (m_boolSendPos == 0) 
    {
        m_data.resize(m_data.size() + 1, '\0');
    }

    if (data)
    {
        sf::Uint8 byte = *reinterpret_cast<const sf::Uint8*>(&m_data.back());
        byte |= (1 << m_boolSendPos);

        m_data.back() = *reinterpret_cast<char*>(&byte);
    }

    m_boolSendPos = (m_boolSendPos + 1) % 8;

    return *this;
}

Packet& Packet::operator <<(sf::Int8 data)
{
    append(&data, sizeof(data));
    return *this;
}

Packet& Packet::operator <<(sf::Uint8 data)
{
    append(&data, sizeof(data));
    return *this;
}

Packet& Packet::operator <<(sf::Int16 data)
{
    sf::Int16 toWrite = htons(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator <<(sf::Uint16 data)
{
    sf::Uint16 toWrite = htons(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator <<(sf::Int32 data)
{
    sf::Int32 toWrite = htonl(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator <<(sf::Uint32 data)
{
    sf::Uint32 toWrite = htonl(data);
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator <<(sf::Int64 data)
{
    // Since htonll is not available everywhere, we have to convert
    // to network byte order (big endian) manually
    sf::Uint8 toWrite[] =
    {
        static_cast<sf::Uint8>((data >> 56) & 0xFF),
        static_cast<sf::Uint8>((data >> 48) & 0xFF),
        static_cast<sf::Uint8>((data >> 40) & 0xFF),
        static_cast<sf::Uint8>((data >> 32) & 0xFF),
        static_cast<sf::Uint8>((data >> 24) & 0xFF),
        static_cast<sf::Uint8>((data >> 16) & 0xFF),
        static_cast<sf::Uint8>((data >>  8) & 0xFF),
        static_cast<sf::Uint8>((data      ) & 0xFF)
    };
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator <<(sf::Uint64 data)
{
    // Since htonll is not available everywhere, we have to convert
    // to network byte order (big endian) manually
    sf::Uint8 toWrite[] =
    {
        static_cast<sf::Uint8>((data >> 56) & 0xFF),
        static_cast<sf::Uint8>((data >> 48) & 0xFF),
        static_cast<sf::Uint8>((data >> 40) & 0xFF),
        static_cast<sf::Uint8>((data >> 32) & 0xFF),
        static_cast<sf::Uint8>((data >> 24) & 0xFF),
        static_cast<sf::Uint8>((data >> 16) & 0xFF),
        static_cast<sf::Uint8>((data >>  8) & 0xFF),
        static_cast<sf::Uint8>((data      ) & 0xFF)
    };
    append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator <<(float data)
{
    append(&data, sizeof(data));
    return *this;
}

Packet& Packet::operator <<(double data)
{
    append(&data, sizeof(data));
    return *this;
}

Packet& Packet::operator <<(const char* data)
{
    // First insert string length
    sf::Uint32 length = static_cast<sf::Uint32>(std::strlen(data));
    *this << length;

    // Then insert characters
    append(data, length * sizeof(char));

    return *this;
}

Packet& Packet::operator <<(const std::string& data)
{
    // First insert string length
    sf::Uint32 length = static_cast<sf::Uint32>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
        append(data.c_str(), length * sizeof(std::string::value_type));

    return *this;
}

Packet& Packet::operator <<(const wchar_t* data)
{
    // First insert string length
    sf::Uint32 length = static_cast<sf::Uint32>(std::wcslen(data));
    *this << length;

    // Then insert characters
    for (const wchar_t* c = data; *c != L'\0'; ++c)
        *this << static_cast<sf::Uint32>(*c);
    
    m_boolSendPos = 0;

    return *this;
}

Packet& Packet::operator <<(const std::wstring& data)
{
    // First insert string length
    sf::Uint32 length = static_cast<sf::Uint32>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (std::wstring::const_iterator c = data.begin(); c != data.end(); ++c)
            *this << static_cast<sf::Uint32>(*c);
        
        m_boolSendPos = 0;
    }

    return *this;
}

Packet& Packet::operator <<(const sf::String& data)
{
    // First insert the string length
    sf::Uint32 length = static_cast<sf::Uint32>(data.getSize());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (sf::String::ConstIterator c = data.begin(); c != data.end(); ++c)
            *this << *c;
        
        m_boolSendPos = 0;
    }

    return *this;
}

bool Packet::checkSize(std::size_t size)
{
    m_isValid = m_isValid && (m_readPos + size <= m_data.size());

    return m_isValid;
}

const void* Packet::onSend(std::size_t& size)
{
    size = getDataSize();
    return getData();
}

void Packet::onReceive(const void* data, std::size_t size)
{
    append(data, size);
}
