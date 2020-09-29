#include "entity.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include "helper.hpp"
#include "client_entity_manager.hpp"

u32 BaseEntityComponent::getUniqueId() const
{
    return m_uniqueId;
}

void BaseEntityComponent::setUniqueId(u32 uniqueId)
{
    m_uniqueId = uniqueId;
}

u8 BaseEntityComponent::getEntityType() const
{
    return m_type;
}
    
void BaseEntityComponent::setEntityType(u8 entityType)
{
    m_type = entityType;
}

Vector2 BaseEntityComponent::getPosition() const
{
    return m_pos;
}

void BaseEntityComponent::setPosition(const Vector2& pos)
{
    m_pos = pos;
}

Vector2 BaseEntityComponent::getVelocity() const
{
    return m_vel;
}

void BaseEntityComponent::setVelocity(const Vector2& vel)
{
    m_vel = vel;
}


u8 BaseEntityComponent::getTeamId() const
{
    return m_teamId;
}

void BaseEntityComponent::setTeamId(u8 teamId)
{
    m_teamId = teamId;
}

u8 BaseEntityComponent::getCollisionRadius() const
{
    return m_collisionRadius;
}

void BaseEntityComponent::setCollisionRadius(u8 radius)
{
    m_collisionRadius = radius;
}

u8 BaseEntityComponent::getFlyingHeight() const
{
    return m_flyingHeight;
}

void BaseEntityComponent::setFlyingHeight(u8 flyingHeight)
{
    m_flyingHeight = flyingHeight;
}

bool BaseEntityComponent::isInBush() const
{
    return m_inBush;
}

bool BaseEntityComponent::isSolid() const
{
    return m_solid;
}

void BaseEntityComponent::setSolid(bool solid)
{
    m_solid = solid;
}

bool BaseEntityComponent::canCollide(const BaseEntityComponent& otherEntity) const
{
    //@TODO: should we check flyingheight as well?
    return m_uniqueId != otherEntity.m_uniqueId && m_solid && otherEntity.m_solid;
}

void BaseEntityComponent::loadFromJson(const rapidjson::Document& doc)
{
    m_teamId = 0;

    if (doc.HasMember("flying_height")) {
        m_flyingHeight = doc["flying_height"].GetInt();
    } else {
        m_flyingHeight = 0;
    }

    m_collisionRadius = doc["collision_radius"].GetUint();

    m_inBush = false;

    if (doc.HasMember("solid")) {
        m_solid = doc["solid"].GetBool();
    } else {
        m_solid = false;
    }
}

void Entity::loadFromJson(const rapidjson::Document& doc)
{
    BaseEntityComponent::loadFromJson(doc);

    m_dead = false;
}

void Entity::onCreated()
{

}

bool Entity::shouldSendToTeam(u8 teamId) const
{
    return true;
}

void Entity::applyInput(const PlayerInput& input, const ManagersContext& context, u16 clientDelay)
{

}

void Entity::onQuadtreeInserted(const ManagersContext& context)
{

}

bool Entity::isDead() const
{
    return m_dead;
}

void C_Entity::loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context)
{
    BaseEntityComponent::loadFromJson(doc);

    m_textureId = textureId;

    if (doc.HasMember("scale")) {
        m_scale = doc["scale"].GetFloat();
    } else {
        m_scale = 1.f;
    }

    if (doc.HasMember("sub_texture_rect")) {
        m_useSubTextureRect = true;

        m_subTextureRect.width = doc["sub_texture_rect"]["width"].GetInt();
        m_subTextureRect.height = doc["sub_texture_rect"]["height"].GetInt();
        
        if (doc["sub_texture_rect"].HasMember("left")) {
            m_subTextureRect.left = doc["sub_texture_rect"]["left"].GetInt();
        } else {
            m_subTextureRect.left = 0;
        }

        if (doc["sub_texture_rect"].HasMember("top")) {
            m_subTextureRect.top = doc["sub_texture_rect"]["top"].GetInt();
        } else {
            m_subTextureRect.top = 0;
        }

    } else {
        m_useSubTextureRect = false;
    }
}

void C_Entity::updateControlledAngle(float newAngle)
{

}

void C_Entity::applyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt)
{

}

void C_Entity::reapplyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context)
{

}

u16 C_Entity::getControlledMovementSpeed() const
{
    return 0;
}

void C_Entity::updateLocallyVisible(const C_ManagersContext& context)
{

}

void C_Entity::localReveal(C_Entity* entity)
{

}

void C_Entity::insertRenderNode(const C_ManagersContext& managersContext, const Context& context)
{
    std::vector<RenderNode>& renderNodes = managersContext.entityManager->getRenderNodes();
    renderNodes.emplace_back(m_uniqueId);
    renderNodes.back().usingSprite = true;

    sf::Sprite& sprite = renderNodes.back().sprite;
    
    sprite.setTexture(context.textures->getResource(m_textureId));

    if (m_useSubTextureRect) {
        sprite.setTextureRect(m_subTextureRect);
    }

    sprite.setScale(m_scale, m_scale);
    sprite.setOrigin(sprite.getLocalBounds().width/2.f, sprite.getLocalBounds().height/2.f);
    sprite.setPosition(m_pos);

    //getGlobalBounds() is required to take scale into account
    renderNodes.back().height = getPosition().y + sprite.getGlobalBounds().height/2.f + m_flyingHeight;

#ifdef MANDARINA_DEBUG
    renderNodes.back().debugDisplayData = std::to_string(m_uniqueId);
    renderNodes.back().collisionRadius = (float) m_collisionRadius;
    renderNodes.back().position = getPosition();
#endif
}

u16 C_Entity::getTextureId() const
{
	return m_textureId;
}

float C_Entity::getScale() const
{
	return m_scale;
}

bool C_Entity::getUseSubTextureRect() const
{
	return m_useSubTextureRect;
}

sf::IntRect C_Entity::getSubTextureRect() const
{
	return m_subTextureRect;
}

void HealthComponent::takeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId)
{
    m_health = std::max((int) m_health - (int) damage, 0);
    onTakeDamage(damage, source, uniqueId, teamId);
}

void HealthComponent::beHealed(u16 amount, Entity* source)
{
    m_health = std::min((int) m_health + (int) amount, (int) m_maxHealth);
    onBeHealed(amount, source);
}

void HealthComponent::onTakeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId)
{
    m_latestDamageDealer = uniqueId;
    m_latestDamageDealerTeamId = teamId;
}

void HealthComponent::onBeHealed(u16 amount, Entity* source)
{

}

void HealthComponent::increaseMaxHealth(u16 amount)
{
    float percentage = static_cast<float>(m_health)/static_cast<float>(m_maxHealth);

    m_maxHealth = std::min(0xffff, m_maxHealth + amount);
    m_health = percentage * m_maxHealth;
}

u16 HealthComponent::getHealth() const
{
    return m_health;
}

u16 HealthComponent::getMaxHealth() const
{
    return m_maxHealth;
}

u8 HealthComponent::getLatestDamageDealerTeamId() const
{
    return m_latestDamageDealerTeamId;
}

u32 HealthComponent::getLatestDamageDealer() const
{
    return m_latestDamageDealer;
}

void HealthComponent::loadFromJson(const rapidjson::Document& doc)
{
    m_latestDamageDealer = 0;

    m_maxHealth = doc["health"].GetUint();

    if (doc.HasMember("starting_health")) {
        m_health = doc["starting_health"].GetUint();
    } else {
        m_health = m_maxHealth;
    }
}

const u8 TrueSightComponent::defaultTrueSightRadius = 170;

void TrueSightComponent::setTrueSightRadius(u8 trueSightRadius)
{
    m_trueSightRadius = trueSightRadius;
}

u8 TrueSightComponent::getTrueSightRadius() const
{
    return m_trueSightRadius;
}

void TrueSightComponent::loadFromJson(const rapidjson::Document& doc)
{
    if (doc.HasMember("true_sight_radius")) {
        m_trueSightRadius = doc["true_sight_radius"].GetUint();
    } else {
        m_trueSightRadius = defaultTrueSightRadius;
    }
}

InvisibleComponent::InvisibleComponent()
{
    resetInvisibleFlags();
}

void InvisibleComponent::resetInvisibleFlags()
{
    m_visionFlags = 0;
    m_teamSentFlags = 0;
    m_teamSentCloserFlags = 0;
    m_invisible = false;
}

void InvisibleComponent::markToSend(u8 teamId)
{
    m_teamSentFlags |= (1 << teamId);
}

void InvisibleComponent::markToSendCloser(u8 teamId)
{
    m_teamSentCloserFlags |= (1 << teamId);
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

bool InvisibleComponent::isInvisibleOrBush() const
{
    return m_invisible || _invisible_inBush();
}

bool InvisibleComponent::isRevealedForTeam(u8 teamId) const
{
    return (m_visionFlags & (1 << teamId));
}

bool InvisibleComponent::isMarkedToSendForTeam(u8 teamId) const
{
    return (m_teamSentFlags & (1 << teamId));
}

bool InvisibleComponent::isMarkedToSendCloserForTeam(u8 teamId) const
{
    return (m_teamSentCloserFlags & (1 << teamId));
}

bool InvisibleComponent::isVisibleForTeam(u8 teamId) const
{
    if (_invisible_teamId() == teamId || !isInvisibleOrBush()) {
        return true;
    } else {
        return isRevealedForTeam(teamId);
    }
}

bool InvisibleComponent::shouldBeHiddenFrom(TrueSightComponent& otherEntity) const
{
    if (_invisible_teamId() == otherEntity._trueSight_teamId()) {
        return false;
    } else if (Helper_vec2length(_invisible_pos() - otherEntity._trueSight_pos()) >= otherEntity.getTrueSightRadius()) {
        return true;
    } else {
        return _invisible_inBush() ? !otherEntity._trueSight_inBush() : false;
    }
}
