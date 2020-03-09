#pragma once

#include "ability.hpp"

class SingleShotAbility : public CooldownAbility
{
public:
    virtual SingleShotAbility* clone();

    virtual void onCast(Unit* caster, const ManagersContext& context, u16 clientDelay);
    virtual void C_onCast(C_Unit* caster, const C_ManagersContext& context, u32 inputId);

    virtual void loadFromJson(const rapidjson::Document& doc);
private:
    u8 m_projectileType;

    template<typename _EManager_Type, typename _Unit_Type>
    void onCast(_EManager_Type* eManager, _Unit_Type* caster, bool isServer);
};
