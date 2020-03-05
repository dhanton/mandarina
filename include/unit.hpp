#pragma once

#include "entity.hpp"


class _UnitBase
{
public:
    _UnitBase();

    float getAimAngle() const;
    void setAimAngle(float aimAngle);

    u8 getMovementSpeed() const;
    void setMovementSpeed(u8 movementSpeed);

protected:
    float m_aimAngle;
    u8 m_weaponId;
    u16 m_movementSpeed;
    //Abilities
    //Buffs
};

class Unit : public Entity, public _UnitBase, public HealthComponent, 
             public InvisibleComponent, public TrueSightComponent
{
public:
    Unit(u32 uniqueId);
    virtual Unit* clone() const;
    
    virtual void update(sf::Time eTime, const ManagersContext& context);
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void packData(const Entity* prevEntity, u8 teamId, CRCPacket& outPacket) const;

    virtual void applyInput(const PlayerInput& input, const ManagersContext& context, u16 clientDelay);

    virtual bool shouldSendForTeam(u8 teamId) const;

    virtual void onQuadtreeInserted(const ManagersContext& context);

    bool inQuadtree() const;
};

struct RenderNode;

class C_Unit : public C_Entity, public _UnitBase, public HealthComponent, 
               public InvisibleComponent, public TrueSightComponent
{
public:
    C_Unit(u32 uniqueId);
    virtual C_Unit* clone() const;

    virtual void update(sf::Time eTime, const C_ManagersContext& context);
    virtual void loadFromData(CRCPacket& inPacket);
    virtual void interpolate(const C_ManagersContext& context, const C_Unit* prevUnit, 
                             const C_Unit* nextUnit, double t, double d);

    virtual void updateControlledAngle(float aimAngle);
    
    virtual void applyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt);
    virtual void applyAbilitiesInput(const PlayerInput& input, const C_ManagersContext& context);

    virtual u16 getControlledMovementSpeed() const;

    virtual void updateLocallyVisible(const C_ManagersContext& context);
    virtual void localReveal(C_Unit* unit);
    virtual void insertRenderNode(const C_ManagersContext& managersContext, const Context& context) const;
};


// class Hero : public Unit
// {
//     //abilities and ultimate
//     //some sort of xp system/power system
// };