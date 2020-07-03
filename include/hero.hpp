#pragma once

#include "unit.hpp"

class _HeroBase
{
public:
    _HeroBase() = default;

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

class Hero : public Unit, public _HeroBase
{
public:
    virtual Hero* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc);
    virtual void packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const;

    virtual void onDeath(bool& dead, const ManagersContext& context);

    virtual float getPowerDamageMultiplier() const;
    void increasePower(u32 amount);
    void consumeFood(u8 foodType);

private:
    std::vector<u8> m_consumedFood;
};

class C_Hero : public C_Unit, public _HeroBase
{
public:
    virtual C_Hero* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context);
    virtual void loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot);
};
