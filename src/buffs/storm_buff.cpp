#include "buffs/storm_buff.hpp"

#include "unit.hpp"

StormBuff* StormBuff::clone() const
{
    return new StormBuff(*this);
}

void StormBuff::onUpdate(sf::Time eTime)
{
    m_tresholdTimer += eTime;
    m_timer += eTime;

    if (m_tresholdTimer >= m_multiplierTimeTreshold) {
        m_tresholdTimer -= m_multiplierTimeTreshold;

        m_damagePerSecond *= m_damageMultiplier;
    }

    if (m_timer >= sf::seconds(1.f)) {
        m_timer -= sf::seconds(1.f);
        
        m_unit->takeDamage(m_damagePerSecond, nullptr, 0, 0);
    }
}

void StormBuff::setDamagePerSecond(u16 dmgPerSecond)
{
    m_damagePerSecond = dmgPerSecond;
}

void StormBuff::setMultiplierTimeTreshold(sf::Time multiplierTimeTreshold)
{
    m_multiplierTimeTreshold = multiplierTimeTreshold;
}

void StormBuff::setDamageMultiplier(float dmgMultiplier)
{
    m_damageMultiplier = dmgMultiplier;
}
