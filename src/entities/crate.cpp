#include "entities/crate.hpp"

#include "bit_stream.hpp"
#include "client_entity_manager.hpp"

Crate* Crate::clone() const
{
    return new Crate(*this);
}

void Crate::loadFromJson(const rapidjson::Document& doc)
{
    Entity::loadFromJson(doc);

    //this values will be overwriten randomly onCreated
    m_maxHealth = 0;
    m_health = 0;
    m_foodAmount = 0;

    m_latestDamageDealer = 0;

    m_minPossibleFood = doc["min_possible_food"].GetUint();
    m_maxPossibleFood = doc["max_possible_food"].GetUint();

    m_minPossibleHealth = doc["min_possible_health"].GetUint();
    m_maxPossibleHealth = doc["max_possible_health"].GetUint();

    m_solid = true;
}

void Crate::update(sf::Time eTime, const ManagersContext& context)
{
    checkDead(context);

    if (m_dead) return;

    //rest of the update would go here
}

void Crate::preUpdate(sf::Time eTime, const ManagersContext& context)
{

}

void Crate::postUpdate(sf::Time eTime, const ManagersContext& context)
{
    checkDead(context);
}

void Crate::packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const
{
    const Crate* prevCrate = static_cast<const Crate*>(prevEntity);

    bool posXChanged = !prevCrate || m_pos.x != prevCrate->getPosition().x;
    outPacket << posXChanged;

    bool posYChanged = !prevCrate || m_pos.y != prevCrate->getPosition().y;
    outPacket << posYChanged;

    bool teamIdChanged = !prevCrate || teamId != prevCrate->getTeamId();
    outPacket << teamIdChanged;

    bool maxHealthChanged = !prevCrate || m_maxHealth != prevCrate->getMaxHealth();
    outPacket << maxHealthChanged;

    bool healthChanged = !prevCrate || m_health != prevCrate->getHealth();
    outPacket << healthChanged;

    if (posXChanged) {
        outPacket << m_pos.x;
    }

    if (posYChanged) {
        outPacket << m_pos.y;
    }

    if (teamIdChanged) {
        outPacket << m_teamId;
    }

    if (maxHealthChanged) {
        outPacket << m_maxHealth;
    }

    if (healthChanged) {
        outPacket << m_health;
    }
}

void Crate::onCreated()
{
    int possibleFoodDiff = m_maxPossibleFood - m_minPossibleFood;
    m_foodAmount = (rand() % (possibleFoodDiff + 1)) + m_minPossibleFood;
    
    float percentage = static_cast<float>(m_foodAmount - m_minPossibleFood)/static_cast<float>(possibleFoodDiff);

    m_maxHealth = percentage * static_cast<float>(m_maxPossibleHealth) + (1.f - percentage) * static_cast<float>(m_minPossibleHealth);
    m_health = m_maxHealth;
}

void Crate::onDeath(bool& dead, const ManagersContext& context)
{
    FoodBase::scatterRandomFood(getPosition(), m_foodAmount, context);
}

void Crate::checkDead(const ManagersContext& context)
{
    if (!m_dead && m_health == 0) {
        m_dead = true;

        onDeath(m_dead, context);
    }
}

C_Crate* C_Crate::clone() const
{
    return new C_Crate(*this);
}

void C_Crate::loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context)
{
    C_Entity::loadFromJson(doc, textureId, context);

    m_solid = true;
}

void C_Crate::update(sf::Time eTime, const C_ManagersContext& context)
{

}

void C_Crate::loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot)
{
    bool posXChanged;
    bool posYChanged;
    bool teamIdChanged;
    bool maxHealthChanged;
    bool healthChanged;
    
    inPacket >> posXChanged;
    inPacket >> posYChanged;
    inPacket >> teamIdChanged;
    inPacket >> maxHealthChanged;
    inPacket >> healthChanged;

    if (posXChanged) {
        inPacket >> m_pos.x;
    }

    if (posYChanged) {
        inPacket >> m_pos.y;
    }

    if (teamIdChanged) {
        inPacket >> m_teamId;
    }

    if (maxHealthChanged) {
        inPacket >> m_maxHealth;
    }

    if (healthChanged) {
        inPacket >> m_health;
    }
}

void C_Crate::interpolate(const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d, bool isControlled)
{

}

void C_Crate::copySnapshotData(const C_Entity* snapshotEntity, bool isControlled)
{
    HealthUI healthUI = m_ui;

    *this = *(static_cast<const C_Crate*>(snapshotEntity));

    m_ui = healthUI;
}

void C_Crate::insertRenderNode(const C_ManagersContext& managersContext, const Context& context)
{
    C_Entity::insertRenderNode(managersContext, context);

    std::vector<RenderNode>& uiRenderNodes = managersContext.entityManager->getUIRenderNodes();
    
    if (!m_ui.getEntity()) {
        m_ui.setEntity(this);
        m_ui.setHealthComponent(this);
        m_ui.setFonts(context.fonts);
    }

    bool isAlly = (m_teamId == managersContext.entityManager->getLocalTeamId());

    if (!m_ui.getEntity() || isAlly != m_ui.getIsAlly()) {
        m_ui.setIsAlly(m_teamId == managersContext.entityManager->getLocalTeamId());
    }

    uiRenderNodes.emplace_back(m_uniqueId);
    uiRenderNodes.back().usingSprite = false;
    uiRenderNodes.back().drawable = &m_ui;
    uiRenderNodes.back().height = getPosition().y;
}
