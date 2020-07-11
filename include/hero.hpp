#pragma once

#include "unit.hpp"
#include "hero_ui.hpp"

class HeroBase
{
public:
    static const size_t maxDisplayNameSize;

    HeroBase() = default;

    std::string getDisplayName() const;
    void setDisplayName(std::string displayName);

    u32 getPower() const;

    //number sent and displayed in the UI
    u8 getPowerLevel() const;

protected:
    void loadFromJson(const rapidjson::Document& doc);

protected:
    std::string m_displayName;
    u32 m_power;
};

class Hero : public Unit, public HeroBase
{
public:
    virtual Hero* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc);
    virtual void packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const;

    virtual void onDeath(bool& dead, const ManagersContext& context);

    virtual float getDamageMultiplier() const;
    void increasePower(u32 amount);
    void consumeFood(u8 foodType);

private:
    std::vector<u8> m_consumedFood;

    float m_powerHealthMultiplier;
    float m_powerDamageMultiplier;
};

class C_Hero : public C_Unit, public HeroBase
{
public:
    virtual C_Hero* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context);
    virtual void loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot);

    virtual void insertRenderNode(const C_ManagersContext& managersContext, const Context& context);

protected:
    virtual void _doSnapshotCopy(const C_Entity* snapshotEntity);

private:
    HeroUI m_ui;
};
