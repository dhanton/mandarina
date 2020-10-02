#include "status.hpp"

#include "texture_ids.hpp"

bool Status::m_jsonLoaded = false;
bool Status::m_inStatusBar[STATUS_MAX_TYPES];
float Status::m_scale[STATUS_MAX_TYPES];
Vector2 Status::m_offset[STATUS_MAX_TYPES];

Status::Status()
{
    resetData();
}

void Status::preUpdate()
{
    resetData();
}

bool Status::canMove() const
{
    return !m_data[STATUS_STUNNED] && !m_data[STATUS_ROOTED];
}

bool Status::canAttack() const
{
    return !m_data[STATUS_STUNNED] && !m_data[STATUS_DISARMED];
}

bool Status::canCast() const
{
    return !m_data[STATUS_STUNNED] && !m_data[STATUS_SILENCED];
}

bool& Status::operator[](const unsigned int& index)
{
    return m_data[index];
}

const bool& Status::operator[](const unsigned int& index) const
{
    return m_data[index];
}

void Status::packData(CRCPacket& outPacket) const
{
    #define DoStatus(type, json_id) \
        outPacket << m_data[STATUS_##type];
    #include "status.inc"
    #undef DoStatus
}

void Status::loadFromData(CRCPacket& inPacket)
{
    #define DoStatus(type, json_id) \
        inPacket >> m_data[STATUS_##type];
    #include "status.inc"
    #undef DoStatus
}

void Status::loadJsonData(const JsonParser* jsonParser)
{
    if (m_jsonLoaded) return;

    #define DoStatus(type, json_id) \
        if (jsonParser->isLoaded(json_id)) { \
            loadFromJson(jsonParser->getDocument(json_id), STATUS_##type); \
        } else { \
            loadFromJson(nullptr, STATUS_##type); \
        }
    #include "status.inc"
    #undef DoStatus

    m_jsonLoaded = true;
}

u16 Status::getTextureId(u8 _type)
{
    #define DoStatus(type, json_id) \
        if (_type == STATUS_##type) { \
            return TextureId::type; \
        }
    #include "status.inc"
    #undef DoStatus
}

bool Status::getInStatusBar(u8 type)
{
    return m_inStatusBar[type];
}

float Status::getScale(u8 type)
{
    return m_scale[type];
}

const Vector2& Status::getOffset(u8 type)
{
    return m_offset[type];
}

void Status::loadFromJson(const rapidjson::Document* doc, unsigned int index)
{
    if (doc && doc->HasMember("in_status_bar")) {
        m_inStatusBar[index] = (*doc)["in_status_bar"].GetBool();
    } else {
        m_inStatusBar[index] = true;
    }

    if (doc && doc->HasMember("scale")) {
        m_scale[index] = (*doc)["scale"].GetFloat();
    } else {
        m_scale[index] = 1.f;
    }

    if (!m_inStatusBar && doc && doc->HasMember("offset")) {
        m_offset[index] = Vector2((*doc)["offset"][0].GetFloat(), (*doc)["offset"][1].GetFloat());
    }
}

void Status::resetData()
{
    #define DoStatus(type, json_id) \
        m_data[STATUS_##type] = false;
    #include "status.inc"
    #undef DoStatus
}
