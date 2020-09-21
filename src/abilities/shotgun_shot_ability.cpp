#include "abilities/shotgun_shot_ability.hpp"

#include "unit.hpp"
#include "helper.hpp"
#include "game_mode.hpp"
#include "client_entity_manager.hpp"
#include "server_entity_manager.hpp"

ShotgunShotAbility* ShotgunShotAbility::clone() const
{
	return new ShotgunShotAbility(*this);
}
void ShotgunShotAbility::onCast(Unit* caster, const ManagersContext& context, u16 clientDelay)
{
	CooldownAbility::onCastUpdate();

	Projectile* projectile = nullptr;
	float multiplier = context.gameMode->getDamageMultiplier() * caster->getDamageMultiplier();
	
	for (int i = 0; i < m_projectileNumber; ++i) {
		ABILITY_CREATE_PROJECTILE(m_projectileType, caster->getPosition(), caster->getAimAngle(), caster->getTeamId())
		ABILITY_SET_PROJECTILE_SHOOTER(caster)

		ABILITY_SET_PROJECTILE_DAMAGE_MULTIPLIER(multiplier)
		ABILITY_BACKTRACK_PROJECTILE(clientDelay)
	}
}

void ShotgunShotAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
	if (repeating) return;

	CooldownAbility::onCastUpdate();

	C_Projectile* projectile = nullptr;

	for (int i = 0; i < m_projectileNumber; ++i) {
		//const float angle = caster->getAimAngle() + 
		ABILITY_CREATE_PROJECTILE(m_projectileType, unit->getPosition(), unit->getAimAngle(), unit->getTeamId())
		ABILITY_SET_PROJECTILE_INPUT_ID(inputId)
	}
}

void ShotgunShotAbility::loadFromJson(const rapidjson::Document& doc)
{
	CooldownAbility::loadFromJson(doc);

	m_projectileType = Projectile_stringToType(doc["projectile"].GetString());
	m_projectileNumber = doc["projectile_number"].GetUint();

	if (doc.HasMember("initial_distance")) {
		m_initialDistance = doc["initial_distance"].GetFloat();
	} else {
		m_initialDistance = 0.f;
	}

	m_spreadAngle = std::min(doc["spread_angle"].GetUint(), 360u);
}
