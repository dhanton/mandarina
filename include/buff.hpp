#pragma once

#include <SFML/System/Time.hpp>
#include "defines.hpp"
#include "json_parser.hpp"
#include "crcpacket.hpp"
#include "managers_context.hpp"
#include "status.hpp"

enum BuffType {
    BUFF_NONE, 

    #define DoBuff(class_name, type, json_id) \
        BUFF_##type,
    #include "buffs.inc"
    #undef DoBuff

    BUFF_MAX_TYPES
};

class Entity;
class Unit;
class Ability;
struct Projectile;

class Buff
{
public:
    static u8 stringToType(const std::string& typeStr);

    virtual Buff* clone() const = 0;

    virtual void loadFromJson(const rapidjson::Document& doc);
    
    //update the timer
    void update(sf::Time eTime);

    //callbacks go here onUnitSpanwed, onUnitUpdated, onUnitDamaged, etc 
    //(look at platano for names)

    virtual void onStart();
    virtual void onEnd();
    virtual void onPreUpdate(sf::Time eTime);
    virtual void onUpdate(sf::Time eTime);
    virtual void onDeath(bool& dead);
    
    //uniqueId and teamId are passed since the source entity might be invalid when the damage is dealt
    virtual void onTakeDamage(u16& damage, Entity* source, u32 uniqueId, u8 teamId);

    virtual void onProjectileHit(Projectile& projectile, Entity* target);
    virtual void onDealDamage(u16 damage, Entity* target);
    virtual void onBeHealed(u16 amount, Entity* source);
    virtual void onHeal(u16 amount, Entity* target);
    virtual void onEntityKill(Entity* target);

    virtual void onGetDamageMultiplier(float& multiplier);

    //we might need to pass some arguments to these (endPos, which ability was casted, etc)
    virtual void onMovement();
    virtual void onPrimaryFireCasted();
    virtual void onSecondaryFireCasted();
    virtual void onAltAbilityCasted();
    virtual void onUltimateCasted();

    // virtual void onAbilityCasted(Ability* ability);
    //onGoingHidden onGoingInvis onBreakingInvis

    void setUnit(Unit* unit);

    void kill();
    bool isDead() const;
    
    u8 getType() const;
    void setBuffType(u8 type);

    float getCurrentTime() const;

protected:
    Unit* m_unit;

    bool m_dead;
    u8 m_type;

    float m_duration;
    float m_currentTime;
};
