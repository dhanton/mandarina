#pragma once

#include <SFML/System/Time.hpp>
#include "context.hpp"
#include "managers_context.hpp"
#include "defines.hpp"
#include "crcpacket.hpp"
#include "player_input.hpp"
#include "json_parser.hpp"
#include "component.hpp"
#include "caster_snapshot.hpp"

class BaseEntityComponent
{
public:
    u32 getUniqueId() const;
    void setUniqueId(u32 uniqueId);

    u8 getEntityType() const;
    void setEntityType(u8 entityType);

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

    bool isSolid() const;
    void setSolid(bool solid);

    bool canCollide(const BaseEntityComponent& otherEntity) const;

protected:
    void loadFromJson(const rapidjson::Document& doc);

protected:
    u32 m_uniqueId;
    u8 m_type;

    Vector2 m_pos;
    Vector2 m_vel;
    u8 m_teamId;
    u8 m_collisionRadius;
    u8 m_flyingHeight;
    bool m_inBush;
    bool m_solid;
};

enum EntityType {
    #define DoEntity(class_name, type, json_id) \
        ENTITY_##type,
    #include "entities.inc"
    #undef DoEntity

    ENTITY_MAX_TYPES
};

class Entity : public BaseEntityComponent
{
public:
    virtual Entity* clone() const = 0;

    //virtual destructor is required to delete an instance of a derived class through a pointer to this class
    //(even if delete is handled by unique_ptr)
    virtual ~Entity() = default;

    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void update(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const = 0;

    virtual void onCreated();

    virtual bool shouldSendToTeam(u8 teamId) const;

    virtual void applyInput(const PlayerInput& input, const ManagersContext& context, u16 clientDelay);

    //Callback when the entity is inserted on the quadtree
    virtual void onQuadtreeInserted(const ManagersContext& context);

    bool isDead() const;

protected:
    bool m_dead;
};

//Used by ClientEntityManager
struct RenderNode;

class C_Entity : public BaseEntityComponent
{
public:
    virtual C_Entity* clone() const = 0;

    //virtual destructor is required to delete an instance of a derived class through a pointer to this class
    //(even if delete is handled by unique_ptr)
    virtual ~C_Entity() = default;

    virtual void loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context);

    virtual void update(sf::Time eTime, const C_ManagersContext& context) = 0;
    virtual void loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot) = 0;
    virtual void interpolate(const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d, bool isControlled) = 0;
    
    //Setup for the next interpolation
    virtual void copySnapshotData(const C_Entity* snapshotEntity, bool isControlled) = 0;

    //these 2 methods are called only for the controlled entity in client
    //update the angle with respect to the mouse
    virtual void updateControlledAngle(float newAngle);

    virtual void applyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt);
    virtual void reapplyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context);

    //returns the movement speed of the entity
    virtual u16 getControlledMovementSpeed() const;

    virtual void updateLocallyVisible(const C_ManagersContext& context);
    virtual void localReveal(C_Entity* entity);

    virtual void insertRenderNode(const C_ManagersContext& managersContext, const Context& context);

    u16 getTextureId() const;
    float getScale() const;
    bool getUseSubTextureRect() const;
    sf::IntRect getSubTextureRect() const;

protected:
    u16 m_textureId;
    float m_scale;

    sf::IntRect m_subTextureRect;
    bool m_useSubTextureRect;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// COMPONENTS //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

class HealthComponent
{
public:
    void takeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId);
    void beHealed(u16 amount, Entity* source);

    virtual void onTakeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId);
    virtual void onBeHealed(u16 amount, Entity* source);

    //increase max health but keep health/max the same
    void increaseMaxHealth(u16 amount);

    u16 getHealth() const;
    u16 getMaxHealth() const;

    u32 getLatestDamageDealer() const;

    //the previous uniqueId might not be valid, but we still need the teamId
    u8 getLatestDamageDealerTeamId() const;

protected:
    void loadFromJson(const rapidjson::Document& doc);

protected:
    u16 m_health;
    u16 m_maxHealth;

    //latest entity that dealt damage to this component
    u32 m_latestDamageDealer;
    u8 m_latestDamageDealerTeamId;
};

class TrueSightComponent
{
public:
    friend class InvisibleComponent;

public:
    void setTrueSightRadius(u8 trueSightRadius);
    u8 getTrueSightRadius() const;

    static const u8 defaultTrueSightRadius;

protected:
    void loadFromJson(const rapidjson::Document& doc);

protected:
    u8 m_trueSightRadius;

private:
    COMP_CROSS_VIRTUAL(trueSight, u8, teamId)
    COMP_CROSS_VIRTUAL(trueSight, bool, inBush)
    COMP_CROSS_VIRTUAL(trueSight, Vector2, pos)
};

#define TRUE_SIGHT_COMPONENT() \
    COMP_CROSS_VARIABLE(trueSight, u8, teamId) \
    COMP_CROSS_VARIABLE(trueSight, bool, inBush) \
    COMP_CROSS_VARIABLE(trueSight, Vector2, pos)

class InvisibleComponent
{
public:
    friend class TrueSightComponent;

public:
    InvisibleComponent();

    void resetInvisibleFlags();
    void markToSend(u8 teamId);
    void markToSendCloser(u8 teamId);
    void reveal(u8 teamId);

    void setInvisible(bool invisible);
    bool isInvisible() const;
    bool isInvisibleOrBush() const;

    bool isRevealedForTeam(u8 teamId) const;
    bool isMarkedToSendForTeam(u8 teamId) const;
    bool isMarkedToSendCloserForTeam(u8 teamId) const;
    bool isVisibleForTeam(u8 teamId) const;
    bool shouldBeHiddenFrom(TrueSightComponent& otherEntity) const;

private:
    //stores if the entity is visible for each team (max 64 teams)
    u64 m_visionFlags;

    //all entities close enough to a team will be forced to be sent
    //(even if they're hidden for that team)
    //this is to make reveals smooth in client
    u64 m_teamSentFlags;

    //used to tell the client if the entity is being revealed by any other method
    //that is not close proximity
    u64 m_teamSentCloserFlags;

    bool m_invisible;

    //variables that have to be stored in another component or child class
    COMP_CROSS_VIRTUAL(invisible, u8, teamId)
    COMP_CROSS_VIRTUAL(invisible, bool, inBush)
    COMP_CROSS_VIRTUAL(invisible, Vector2, pos)
};

//This is used by child classes to properly generate cross variable methods
#define INVISIBLE_COMPONENT() \
    COMP_CROSS_VARIABLE(invisible, u8, teamId) \
    COMP_CROSS_VARIABLE(invisible, bool, inBush) \
    COMP_CROSS_VARIABLE(invisible, Vector2, pos)

//@TODO: Move client invisible behaviour here
// class C_InvisibleComponent
// {

// };
