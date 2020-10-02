#pragma once

#include "json_parser.hpp"
#include "defines.hpp"
#include "crcpacket.hpp"

enum StatusType
{
    #define DoStatus(type, json_id) \
        STATUS_##type,
    #include "status.inc"
    #undef DoStatus

    STATUS_MAX_TYPES
};

//Status is like a buffer that buffs can write to
//it's used to quickly check if a unit is being affected in a certain way
//and is the only buff information a client receives
class Status
{
public:
    Status();

    void preUpdate();

    bool canMove() const;
    bool canAttack() const;
    bool canCast() const;

    bool& operator[](const unsigned int& index);
    const bool& operator[](const unsigned int& index) const;

    void packData(CRCPacket& outPacket) const;
    void loadFromData(CRCPacket& inPacket);

    static void loadJsonData(const JsonParser* jsonParser);

    static u16 getTextureId(u8 type);
    static bool getInStatusBar(u8 type);
    static float getScale(u8 type);
    static const Vector2& getOffset(u8 type);

private:
    bool m_data[STATUS_MAX_TYPES];

    static void loadFromJson(const rapidjson::Document* doc, unsigned int index);
    void resetData();

    static bool m_jsonLoaded;
    static bool m_inStatusBar[STATUS_MAX_TYPES];
    static float m_scale[STATUS_MAX_TYPES];
    static Vector2 m_offset[STATUS_MAX_TYPES];
};
