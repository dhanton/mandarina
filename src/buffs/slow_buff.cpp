#include "buffs/slow_buff.hpp"

#include "unit.hpp"

SlowBuff* SlowBuff::clone() const
{
    return new SlowBuff(*this);
}

void SlowBuff::loadFromJson(const rapidjson::Document& doc)
{
    Buff::loadFromJson(doc);

    m_slowAmount = doc["slow_amount"].GetUint();
}

void SlowBuff::onPreUpdate(sf::Time eTime)
{
    m_unit->setMovementSpeed(std::max(0, static_cast<int>(m_unit->getMovementSpeed()) - m_slowAmount));
}
