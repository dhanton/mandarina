#include "buffs/lifesteal_buff.hpp"

#include "unit.hpp"

LifestealBuff* LifestealBuff::clone() const
{
	return new LifestealBuff(*this);
}

void LifestealBuff::loadFromJson(const rapidjson::Document& doc)
{
	Buff::loadFromJson(doc);

	m_lifestealPerDamage = doc["lifesteal_per_damage"].GetFloat();
}

void LifestealBuff::onDealDamage(u16 damage, Entity* target)
{
	m_unit->beHealed(static_cast<float>(damage) * m_lifestealPerDamage, m_unit);
}
