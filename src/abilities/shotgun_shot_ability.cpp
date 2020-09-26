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

	const float multiplier = context.gameMode->getDamageMultiplier() * caster->getDamageMultiplier();

	float angle = caster->getAimAngle() - static_cast<float>(m_spreadAngle)/2;
	
	for (int i = 0; i < m_projectileNumber; ++i) {
		Projectile* projectile = context.entityManager->createProjectile(m_projectileType, caster->getPosition(), angle, caster->getTeamId());
		if (!projectile) continue;

		projectile->shooterUniqueId = caster->getUniqueId();
		projectile->damage *= multiplier;
		Projectile_backtrackCollisions(*projectile, context, clientDelay);
		
		angle += m_angleStep;
	}
}

void ShotgunShotAbility::C_onCast(C_Unit* unit, CasterComponent* caster, Vector2& casterPos, const C_ManagersContext& context, u32 inputId, bool repeating)
{
	if (repeating) return;

	CooldownAbility::onCastUpdate();

	float angle = unit->getAimAngle() - static_cast<float>(m_spreadAngle)/2;

	for (int i = 0; i < m_projectileNumber; ++i) {
		C_Projectile* projectile = context.entityManager->createProjectile(m_projectileType, unit->getPosition(), angle, unit->getTeamId());
		if (!projectile) continue;

		projectile->createdInputId = inputId;

		angle += m_angleStep;
	}
}

void ShotgunShotAbility::loadFromJson(const rapidjson::Document& doc)
{
	CooldownAbility::loadFromJson(doc);

	m_projectileType = Projectile_stringToType(doc["projectile"].GetString());
	m_projectileNumber = doc["projectile_number"].GetUint();

	if (m_projectileNumber < 2) {
		m_spreadAngle = 0;
		m_angleStep = 0.f;
	} else {
		m_spreadAngle = std::min(doc["spread_angle"].GetUint(), 360u);
		m_angleStep = static_cast<float>(m_spreadAngle)/static_cast<float>(m_projectileNumber - 1);
	}
}
