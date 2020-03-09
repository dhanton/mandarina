#pragma once

#include "entity.hpp"
#include "ability.hpp"
#include "caster_component.hpp"
#include "json_parser.hpp"

class _UnitBase
{
public:
    _UnitBase() = default;

    float getAimAngle() const;
    void setAimAngle(float aimAngle);

    u8 getMovementSpeed() const;
    void setMovementSpeed(u8 movementSpeed);

protected:
    void loadFromJson(const rapidjson::Document& doc);

protected:
    float m_aimAngle;
    u8 m_weaponId;
    u16 m_movementSpeed;

    //Buffs (they probably should be in BuffHolderComponent)

private:
    static bool m_abilitiesLoaded;
};

class Unit : public Entity, public _UnitBase, public HealthComponent, 
             public InvisibleComponent, public TrueSightComponent,
             public CasterComponent
{
private:
    TRUE_SIGHT_COMPONENT()
    INVISIBLE_COMPONENT()
    CASTER_COMPONENT()

public:
    virtual Unit* clone() const;
    
    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void update(sf::Time eTime, const ManagersContext& context);
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void packData(const Entity* prevEntity, u8 teamId, CRCPacket& outPacket) const;

    virtual void applyInput(const PlayerInput& input, const ManagersContext& context, u16 clientDelay);

    virtual bool shouldSendToTeam(u8 teamId) const;

    virtual void onQuadtreeInserted(const ManagersContext& context);

private:
    void moveColliding(Vector2 newPos, const ManagersContext& context, bool force = false);
};

struct RenderNode;

class C_Unit : public C_Entity, public _UnitBase, public HealthComponent, 
               public InvisibleComponent, public TrueSightComponent
{
private:
    TRUE_SIGHT_COMPONENT()
    INVISIBLE_COMPONENT()

public:
    virtual C_Unit* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc, u16 textureId);

    virtual void update(sf::Time eTime, const C_ManagersContext& context);
    virtual void loadFromData(CRCPacket& inPacket);
    virtual void interpolate(const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d, bool isControlled);

    virtual void copySnapshotData(const C_Entity* snapshotEntity, bool isControlled);

    virtual void updateControlledAngle(float aimAngle);
    
    virtual void applyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt);

    virtual u16 getControlledMovementSpeed() const;

    virtual void updateLocallyVisible(const C_ManagersContext& context);
    virtual void localReveal(C_Entity* unit);
    virtual void insertRenderNode(const C_ManagersContext& managersContext, const Context& context) const;

private:
    void predictMovementLocally(const Vector2& oldPos, Vector2& newPos, const C_ManagersContext& context) const;
};

// class Hero : public Unit
// {
//     //abilities and ultimate
//     //some sort of xp system/power system
// };