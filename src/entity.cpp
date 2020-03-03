#include "entity.hpp"

#include "helper.hpp"

BaseEntityComponent::BaseEntityComponent(u32 uniqueId):
    m_uniqueId(uniqueId)
{

}

u32 Entity::getUniqueId() const
{
    return m_uniqueId;
}

Entity::Entity(u32 uniqueId):
    BaseEntityComponent(uniqueId)
{

}

bool Entity::isDead() const
{
    return m_dead;
}

C_Entity::C_Entity(u32 uniqueId):
    BaseEntityComponent(uniqueId)
{
    m_textureId = 0;
    m_scale = 1.f;
}

void C_Entity::interpolate(const C_ManagersContext& context, const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d)
{

}

void C_Entity::updateLocallyVisible(const C_ManagersContext& context)
{

}

void C_Entity::localReveal(const C_Entity* entity)
{

}

HealthComponent::HealthComponent()
{
    //default values
    m_health = 1;
    m_maxHealth = 1;
}

void HealthComponent::dealDamage(u16 damage, Entity* source)
{
    m_health = std::max((int) m_health - (int) damage, 0);
}

void HealthComponent::heal(u16 amount, Entity* source)
{
    m_health = std::min((int) m_health + (int) amount, m_maxHealth);
}

void HealthComponent::onDealDamage(u16 damage, Entity* source)
{

}

void HealthComponent::onHeal(u16 amount, Entity* source)
{

}

TrueSightComponent::TrueSightComponent(Vector2& pos, bool& inBush):
    m_pos(pos),
    m_inBush(inBush)
{
    //default value
    trueSightRadius = 0;
}

InvisibleComponent::InvisibleComponent(u8& teamId, Vector2& pos, bool& inBush):
    m_teamId(teamId),
    m_pos(pos),
    m_inBush(inBush)
{
    locallyHidden = false;
    m_visionFlags = 0;
    m_teamSentFlags = 0;
    m_invisible = false;
}

void InvisibleComponent::resetFlags()
{
    m_visionFlags = 0;
    m_teamSentFlags = 0;
}

void InvisibleComponent::markToSend(u8 teamId)
{
    m_teamSentFlags |= (1 << teamId);
}

void InvisibleComponent::reveal(u8 teamId)
{
    m_visionFlags |= (1 << teamId);
}

void InvisibleComponent::setInvisible(bool invisible)
{
    m_invisible = invisible;
}

bool InvisibleComponent::isInvisible() const
{
    return m_invisible;
}

bool InvisibleComponent::isRevealedForTeam(u8 teamId) const
{
    return (m_visionFlags & (1 << teamId));
}

bool InvisibleComponent::isMarkedToSendForTeam(u8 teamId) const
{
    return (m_teamSentFlags & (1 << teamId));
}

bool InvisibleComponent::isVisibleForTeam(u8 teamId) const
{
    if (ref_teamId == teamId || !isInvisible()) {
        return true;
    } else {
        return isRevealedForTeam(teamId);
    }
}

bool InvisibleComponent::shouldBeHiddenFrom(TrueSightComponent& otherEntity) const
{
    if (Helper_vec2length(ref_pos - otherEntity.ref_pos) >= otherEntity.trueSightRadius) {
        return true;
    } else {
        return otherEntity.ref_inBush ? !ref_inBush : false;
    }
}
