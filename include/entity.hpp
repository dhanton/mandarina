#pragma once

#include <SFML/System/Time.hpp>
#include "context.hpp"
#include "managers_context.hpp"
#include "defines.hpp"
#include "crcpacket.hpp"
#include "player_input.hpp"
#include "json_parser.hpp"

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

    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void update(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context) = 0;
    virtual void packData(const Entity* prevEntity, u8 teamId, CRCPacket& outPacket) const = 0;

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

    virtual void loadFromJson(const rapidjson::Document& doc, u16 textureId);

    virtual void update(sf::Time eTime, const C_ManagersContext& context) = 0;
    virtual void loadFromData(CRCPacket& inPacket) = 0;
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

    virtual void insertRenderNode(const C_ManagersContext& managersContext, const Context& context) const;

protected:
    u16 m_textureId;
    float m_scale;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// COMPONENTS //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//These macros are used to generate getters for required fields that the component needs
//and that are stored in another class (usually the child or another component)
#define COMP_CROSS_VIRTUAL(comp_name, var_type, var_name) \
    virtual var_type _##comp_name##_##var_name() const = 0;

#define COMP_CROSS_VARIABLE(comp_name, var_type, var_name) \
    virtual var_type _##comp_name##_##var_name() const {return m_##var_name;}

#define COMP_CROSS_VARIABLE_PUBLIC(comp_name, var_type, var_name) \
    virtual var_type _##comp_name##_##var_name() const {return ##var_name;}

//@BRANCH_WIP: Maybe we can create a MACRO to set some flags in child classes to 
//check which components an entity has so we don't have to use dynamic_cast
//Is it needed? Is dynamic cast that slow??

class HealthComponent
{
public:
    void dealDamage(u16 damage, Entity* source);
    void heal(u16 amount, Entity* source);

    virtual void onDealDamage(u16 damage, Entity* source);
    virtual void onHeal(u16 amount, Entity* source);

    u16 getHealth() const;
    u16 getMaxHealth() const;

protected:
    void loadFromJson(const rapidjson::Document& doc);

protected:
    u16 m_health;
    u16 m_maxHealth;
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
    void reveal(u8 teamId);

    void setInvisible(bool invisible);
    bool isInvisible() const;
    bool isInvisibleOrBush() const;

    bool isRevealedForTeam(u8 teamId) const;
    bool isMarkedToSendForTeam(u8 teamId) const;
    bool isVisibleForTeam(u8 teamId) const;
    bool shouldBeHiddenFrom(TrueSightComponent& otherEntity) const;

    bool isLocallyHidden() const;
    void setLocallyHidden(bool locallyHidden);
    bool isForceSent() const;
    void setForceSent(bool forceSent);

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
