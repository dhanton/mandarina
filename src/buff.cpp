#include "buff.hpp"

#include "unit.hpp"
#include "ability.hpp"
#include "bit_stream.hpp"

u8 Buff::stringToType(const std::string& typeStr)
{
    if (typeStr == "NONE") return BUFF_NONE;

    #define DoBuff(class_name, type, json_id) \
        if (typeStr == #type) return BUFF_##type;
    #include "buffs.inc"
    #undef DoBuff

    return BUFF_NONE;
}

void Buff::loadFromJson(const rapidjson::Document& doc)
{
    m_dead = false;
    m_currentTime = 0.f;

    if (doc.HasMember("duration")) {
        m_duration = doc["duration"].GetFloat();
    } else {
        //passive buff
        m_duration = -1.f;
    }
}

void Buff::update(sf::Time eTime)
{
    if (m_duration != -1.f) {
        if (m_currentTime < m_duration) {
            m_currentTime += eTime.asSeconds();
        } else {
            m_currentTime = m_duration;
            m_dead = true;
        }
    }
}

void Buff::onStart()
{
    
}

void Buff::onEnd()
{

}

void Buff::onPreUpdate(sf::Time eTime)
{

}

void Buff::onUpdate(sf::Time eTime)
{

}

void Buff::onDeath(bool& dead)
{
    
}

void Buff::onTakeDamage(u16& damage, Entity* source, u32 uniqueId, u8 teamId)
{

}

void Buff::onProjectileHit(Projectile& projectile, Entity* target)
{

}

void Buff::onDealDamage(u16 damage, Entity* target)
{

}

void Buff::onBeHealed(u16 amount, Entity* source)
{

}

void Buff::onHeal(u16 amount, Entity* target)
{

}

void Buff::onEntityKill(Entity* target)
{

}

void Buff::onGetDamageMultiplier(float& multiplier)
{

}

void Buff::onMovement()
{

}
void Buff::onPrimaryFireCasted()
{

}

void Buff::onSecondaryFireCasted()
{

}

void Buff::onAltAbilityCasted()
{

}

void Buff::onUltimateCasted()
{

}

void Buff::setUnit(Unit* unit)
{
    m_unit = unit;
}

void Buff::kill()
{
    m_dead = true;
}

bool Buff::isDead() const
{
    return m_dead;
}

u8 Buff::getType() const
{
    return m_type;
}

void Buff::setBuffType(u8 type)
{
    m_type = type;
}

float Buff::getCurrentTime() const
{
    return m_currentTime;
}
