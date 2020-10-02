#pragma once

#include "entity.hpp"
#include "food.hpp"
#include "health_ui.hpp"

class Crate : public Entity, public HealthComponent
{
public:
    virtual Crate* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void update(sf::Time eTime, const ManagersContext& context);
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const;

    virtual void onCreated();
    virtual void onDeath(bool& dead, const ManagersContext& context);

private:
    void checkDead(const ManagersContext& context);

    int m_foodAmount;

    int m_minPossibleFood;
    int m_maxPossibleFood;

    u16 m_minPossibleHealth;
    u16 m_maxPossibleHealth;
};

class C_Crate : public C_Entity, public HealthComponent
{
public:
    virtual C_Crate* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context);

    virtual void update(sf::Time eTime, const C_ManagersContext& context);
    virtual void loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot);
    virtual void interpolate(const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d, bool isControlled);
    virtual void copySnapshotData(const C_Entity* snapshotEntity, bool isControlled);

    virtual void insertRenderNode(sf::Time eTime, const C_ManagersContext& managersContext, const Context& context);

private:
    HealthUI m_ui;
};
