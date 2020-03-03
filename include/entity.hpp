#pragma once

#include "managers_context.hpp"
#include "defines.hpp"
#include "crcpacket.hpp"

//This is not an ECS
//An ECS would probably be better but I have no idea how to make one
//and I don't want to learn right now, I want to make game :) yes

//@BRANCH_WIP
//We're testing OOP since the previous model was not as cache-friendly
//for units as we thought (~200 bytes in size vs 64 bytes of cache lines)

//So we're gonna implemente entities as classes now, with functionality and data
//stored in different classes (we called components)

//In the entity manager we're going to have multiple data structures to store data
//std::list<std::unique_ptr<Entity>>
//Bucket<TouchEntity>
//TouchEntity are lightweight objects that are created and destroyed constantly
//like projectiles or pickup items

class BaseEntityComponent
{
public:
    BaseEntityComponent(u32 uniqueId);

    u32 getUniqueId() const;

public:
    Vector2 pos;
    Vector2 vel;
    u8 teamId;
    u8 collisionRadius;
    u8 flyingHeight;
    bool inBush;

private:
    const u32 m_uniqueId;
};

class Entity : public BaseEntityComponent
{
public:
    Entity(u32 uniqueId);
    virtual Entity* clone() const = 0;

    virtual void update(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void packData(const Entity* prevEntity, u8 teamId, CRCPacket& outPacket) = 0;

    bool isDead() const;

private:
    bool m_dead;
};

class C_Entity : public BaseEntityComponent
{
public:
    C_Entity(u32 uniqueId);
    virtual C_Entity* clone() const = 0;

    virtual void update(sf::Time eTime, const C_ManagersContext& context) = 0;
    virtual void loadFromData(CRCPacket& inPacket) = 0;
    virtual void interpolate(const C_ManagersContext& context, const C_Entity* prevEntity, 
                             const C_Entity* nextEntity, double t, double d);

    virtual void updateLocallyVisible(const C_ManagersContext& context);
    virtual void localReveal(const C_Entity* entity);

    //@BRANCH_WIP: render methods

private:
    u16 m_textureId;
    float m_scale;
};

class HealthComponent
{
public:
    void dealDamage(u16 damage, Entity* source);
    void heal(u16 amount, Entity* source);

    virtual void onDealDamage(u16 damage, Entity* source);
    virtual void onHeal(u16 amount, Entity* source);
private:
    u16 m_health;
    u16 m_maxHealth;
};

class TrueSightComponent
{
public:
    friend class InvisibleComponent;

public:
    TrueSightComponent(Vector2& pos, bool& inBush);

    u8 trueSightRadius;

private:
    //these must be stored in another component
    Vector2& ref_pos;
    bool& ref_inBush;
};

class InvisibleComponent
{
public:
    friend class TrueSightComponent;

public:
    InvisibleComponent(u8& teamId, Vector2& pos, bool& inBush);

    void resetFlags();
    void markToSend(u8 teamId);
    void reveal(u8 teamId);

    void setInvisible(bool invisible);
    bool isInvisible() const;
    bool isRevealedForTeam(u8 teamId) const;
    bool isMarkedToSendForTeam(u8 teamId) const;
    bool isVisibleForTeam(u8 teamId) const;
    bool shouldBeHiddenFrom(TrueSightComponent* otherEntity) const;

public:
    bool locallyHidden;

private:
    //stores if the entity is visible for each team (max 64 teams)
    u64 m_visionFlags;

    //all entities close enough to a team will be forced to be sent
    //(even if they're hidden for that team)
    //this is to make reveals smooth in client
    u64 m_teamSentFlags;

    bool m_invisible;

    //these must be stored in another component
    u8& ref_teamId;
    Vector2& ref_pos;
    bool& ref_inBush;
};
