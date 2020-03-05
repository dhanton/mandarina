#pragma once

#include <SFML/System/Time.hpp>
#include "context.hpp"
#include "managers_context.hpp"
#include "defines.hpp"
#include "crcpacket.hpp"
#include "player_input.hpp"

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

    Vector2 getPosition() const;
    void setPosition(const Vector2& pos);

    Vector2 getVelocity() const;
    void setVelocity(const Vector2& vel);
    
    u8 getTeamId() const;
    void setTeamId(u8 teamId);

    u8 getCollisionRadius() const;
    void setCollisionRadius(u8 radius);

    u8 getFlyingHeight() const;
    void setFlyingHeight(u8 flyingHeight);

    bool isInBush() const;

protected:
    const u32 m_uniqueId;

    Vector2 m_pos;
    Vector2 m_vel;
    u8 m_teamId;
    u8 m_collisionRadius;
    u8 m_flyingHeight;
    bool m_inBush;
};

class Entity : public BaseEntityComponent
{
public:
    Entity(u32 uniqueId);
    virtual Entity* clone() const = 0;

    virtual void update(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void packData(const Entity* prevEntity, u8 teamId, CRCPacket& outPacket) const = 0;

    virtual bool shouldSendToTeam(u8 teamId) const;

    virtual void applyInput(const PlayerInput& input, const ManagersContext& context, u16 clientDelay);

    //Callback when the entity is inserted on the quadtree
    virtual void onQuadtreeInserted(const ManagersContext& context);

    virtual bool inQuadtree() const;

    bool isDead() const;

protected:
    bool m_dead;
};

//Used by ClientEntityManager
struct RenderNode;

class C_Entity : public BaseEntityComponent
{
public:
    C_Entity(u32 uniqueId);
    virtual C_Entity* clone() const = 0;

    virtual void update(sf::Time eTime, const C_ManagersContext& context) = 0;
    virtual void loadFromData(CRCPacket& inPacket) = 0;
    virtual void interpolate(const C_ManagersContext& context, const C_Entity* prevEntity, 
                             const C_Entity* nextEntity, double t, double d);

    //these 2 methods are called only for the controlled entity in client
    //update the angle with respect to the mouse
    virtual void updateControlledAngle(float newAngle);

    virtual void applyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt);
    virtual void applyAbilitiesInput(const PlayerInput& input, const C_ManagersContext& context);

    //returns the movement speed of the entity
    virtual u16 getControlledMovementSpeed() const;

    virtual void updateLocallyVisible(const C_ManagersContext& context);
    virtual void localReveal(const C_Entity* entity);

    virtual void insertRenderNode(const C_ManagersContext& managersContext, const Context& context) const;

    //@BRANCH_WIP: render methods

protected:
    u16 m_textureId;
    float m_scale;
};

class HealthComponent
{
public:
    HealthComponent();

    void dealDamage(u16 damage, Entity* source);
    void heal(u16 amount, Entity* source);

    virtual void onDealDamage(u16 damage, Entity* source);
    virtual void onHeal(u16 amount, Entity* source);

    u16 getHealth() const;
    u16 getMaxHealth() const;

protected:
    u16 m_health;
    u16 m_maxHealth;
};

class TrueSightComponent
{
public:
    friend class InvisibleComponent;

public:
    TrueSightComponent(Vector2& pos, bool& inBush);

    void setTrueSightRadius(u8 trueSightRadius);
    u8 getTrueSightRadius() const;

protected:
    u8 m_trueSightRadius;

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
    bool shouldBeHiddenFrom(TrueSightComponent& otherEntity) const;

    bool isLocallyHidden() const;
    bool isForceSent() const;

protected:
    bool m_locallyHidden;
    bool m_forceSent;

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
