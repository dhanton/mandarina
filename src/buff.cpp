#include "buff.hpp"

#include "unit.hpp"
#include "ability.hpp"
#include "bit_stream.hpp"

void Status::preUpdate()
{
    stunned = false;
    silenced = false;
    disarmed = false;
    rooted = false;
    slowed = false;
}

bool Status::canMove() const
{
    return !stunned && !rooted;
}

bool Status::canAttack() const
{
    return !stunned && !disarmed;
}

bool Status::canCast() const
{
    return !stunned && !silenced;
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

void Buff::onDeath()
{
    
}
void Buff::onTakeDamage(u16 damage, Entity* source)
{

}

void Buff::onDealDamage(u16 damage, Entity* receiver)
{

}

void Buff::onBeHealed(u16 amount, Entity* source)
{

}

void Buff::onHeal(u16 amount, Entity* receiver)
{

}

void Buff::setUnit(Unit* unit)
{
    m_unit = unit;
}

bool Buff::isDead() const
{
    return m_dead;
}

u8 Buff::getType() const
{
    return m_type;
}

float Buff::getCurrentTime() const
{
    return m_currentTime;
}
