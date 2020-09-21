#include "abilities/lifesteal_ability.hpp"


LifestealAbility* LifestealAbility::clone() const
{
	return new LifestealAbility(*this);
}
