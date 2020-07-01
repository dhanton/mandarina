#pragma once

#include <SFML/System/Time.hpp>
#include "defines.hpp"
#include "json_parser.hpp"
#include "crcpacket.hpp"
#include "managers_context.hpp"

enum BuffType {
    BUFF_NONE, 

    #define DoBuff(class_name, type, json_id) \
        BUFF_##type,
    #include "buffs.inc"
    #undef DoBuff

    BUFF_MAX_TYPES
};

//Status is like a buffer that buffs can write to
//it's used to quickly check if a unit is being affected in a certain way
//and is the only buff information a client receives
struct Status
{
    //@TODO: This should probably be done using an array and a type enum
    //so that adding new status variables is easier

    bool stunned = false;
    bool silenced = false;
    bool disarmed = false;
    bool rooted = false;
    bool slowed = false;

    void preUpdate();

    bool canMove() const;
    bool canAttack() const;
    bool canCast() const;
};

class Entity;
class Unit;
class Ability;

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
    virtual void onTakeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId);

    virtual void onDealDamage(u16 damage, Entity* target);
    virtual void onBeHealed(u16 amount, Entity* source);
    virtual void onHeal(u16 amount, Entity* target);
    virtual void onEntityKill(Entity* target);
    // virtual void onAbilityCasted(Ability* ability);
    //onGoingHidden onGoingInvis onBreakingInvis

    void setUnit(Unit* unit);

    bool isDead() const;
    
    u8 getType() const;
    void setBuffType(u8 type);

    //it can be useful for some buffs to hold a pointer to their creator
    //like for example some buffs asociated with abilities
    virtual void setCreator(void* creator, const ManagersContext& context);

    float getCurrentTime() const;

protected:
    Unit* m_unit;

    bool m_dead;
    u8 m_type;

    float m_duration;
    float m_currentTime;
};
