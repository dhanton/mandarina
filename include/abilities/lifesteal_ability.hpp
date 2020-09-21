#pragma once

#include "ability.hpp"

class LifestealAbility : public PassiveAbility
{
public:
	virtual LifestealAbility* clone() const;
};
