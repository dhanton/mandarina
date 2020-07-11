#pragma once

#include <list>
#include "component.hpp"
#include "buff.hpp"

//this component must only be inherited by Unit class

class BuffHolderComponent
{
public:
    static void loadBuffData(const JsonParser* JsonParser);

    BuffHolderComponent() = default;

    //rule of 5 (to copy classes that contain unique_ptr)
    virtual ~BuffHolderComponent() = default;
    BuffHolderComponent(BuffHolderComponent const& other);
    BuffHolderComponent(BuffHolderComponent && other) = default;
    BuffHolderComponent& operator=(BuffHolderComponent const& other);
    BuffHolderComponent& operator=(BuffHolderComponent && other) = default;

    Buff* addBuff(u8 buffType);

    //@TODO: proper purge and better creation flags
    Buff* addUniqueBuff(u8 buffType);
    void removeUniqueBuff(u8 buffType);

protected:
    std::list<std::unique_ptr<Buff>> m_buffs;
    
public:
    //there are more callbacks that don't get called at the same time for all the buffs
    //onEnd() onStart() onPurged()

    void onTakeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId);
    void onDealDamage(u16 damage, Entity* target);
    void onBeHealed(u16 amount, Entity* source);
    void onHeal(u16 amount, Entity* target);
    void onEntityKill(Entity* target);
    // void onAbilityCasted(Ability* ability);

protected:
    void onPreUpdate(sf::Time eTime);
    void onUpdate(sf::Time eTime);
    void onDeath(bool& dead);
    void onGetDamageMultiplier(float& multiplier) const;
    void onMovement();
    void onPrimaryFireCasted();
    void onSecondaryFireCasted();
    void onAltAbilityCasted();
    void onUltimateCasted();

private:
    static std::unique_ptr<Buff> m_buffData[BUFF_MAX_TYPES];
    static bool m_buffsLoaded;
};

#define BUFF_HOLDER_COMPONENT()
