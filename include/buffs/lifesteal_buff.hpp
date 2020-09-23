#pragma once

#include "buff.hpp"

class LifestealBuff : public Buff
{
public:
	virtual LifestealBuff* clone() const;

	virtual void loadFromJson(const rapidjson::Document& doc);

	virtual void onDealDamage(u16 damage, Entity* target);
private:
	float m_lifestealPerDamage;
};
