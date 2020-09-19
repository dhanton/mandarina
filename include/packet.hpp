#pragma once

/**
 * Modification of sf::Packet to decrease bandwith usage
 * when packing multiple booleans in a row.
 * 
 * (see https://github.com/SFML/SFML/pull/1689)
 */

#include <SFML/System/String.hpp>
#include <string>
#include <vector>

class Packet
{
    // A bool-like type that cannot be converted to integer or pointer types
    typedef bool (Packet::*BoolType)(std::size_t);

public:
    Packet();
    virtual ~Packet();

    void append(const void* data, std::size_t sizeInBytes);
    void clear();

    const void* getData() const;
    std::size_t getDataSize() const;

    bool endOfPacket() const;

public:
    operator BoolType() const;

    Packet& operator >>(bool&         data);
    Packet& operator >>(sf::Int8&     data);
    Packet& operator >>(sf::Uint8&    data);
    Packet& operator >>(sf::Int16&    data);
    Packet& operator >>(sf::Uint16&   data);
    Packet& operator >>(sf::Int32&    data);
    Packet& operator >>(sf::Uint32&   data);
    Packet& operator >>(sf::Int64&    data);
    Packet& operator >>(sf::Uint64&   data);
    Packet& operator >>(float&        data);
    Packet& operator >>(double&       data);
    Packet& operator >>(char*         data);
    Packet& operator >>(std::string&  data);
    Packet& operator >>(wchar_t*      data);
    Packet& operator >>(std::wstring& data);
    Packet& operator >>(sf::String&   data);

    Packet& operator <<(bool                data);
    Packet& operator <<(sf::Int8            data);
    Packet& operator <<(sf::Uint8           data);
    Packet& operator <<(sf::Int16           data);
    Packet& operator <<(sf::Uint16          data);
    Packet& operator <<(sf::Int32           data);
    Packet& operator <<(sf::Uint32          data);
    Packet& operator <<(sf::Int64           data);
    Packet& operator <<(sf::Uint64          data);
    Packet& operator <<(float               data);
    Packet& operator <<(double              data);
    Packet& operator <<(const char*         data);
    Packet& operator <<(const std::string&  data);
    Packet& operator <<(const wchar_t*      data);
    Packet& operator <<(const std::wstring& data);
    Packet& operator <<(const sf::String&   data);

protected:
    virtual const void* onSend(std::size_t& size);
    virtual void onReceive(const void* data, std::size_t size);

private:
    bool operator ==(const Packet& right) const;
    bool operator !=(const Packet& right) const;

    bool checkSize(std::size_t size);

    // Member data
    std::vector<char> m_data;
    std::size_t       m_readPos;
    std::size_t       m_sendPos;
    bool              m_isValid;

    std::size_t m_boolReadPos;
    std::size_t m_boolSendPos;
};
