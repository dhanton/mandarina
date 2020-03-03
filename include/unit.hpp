#pragma once

#include "entity.hpp"


class Unit : public Entity, public HealthComponent, public InvisibleComponent,
             public TrueSightComponent
{
public:
    Unit(u32 uniqueId);
    virtual Unit* clone() const;
    
    virtual void update(sf::Time eTime, const ManagersContext& context);
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void packData(const Entity* prevUnit, u8 teamId, CRCPacket& outPacket);

public:
    float aimAngle;

private:
    //buffs
    //status
};

class C_Unit : public C_Entity, public HealthComponent, public InvisibleComponent,
               public TrueSightComponent
{
public:
    C_Unit(u32 uniqueId);
    virtual C_Unit* clone() const;

    virtual void update(sf::Time eTime, const C_ManagersContext& context);
    virtual void loadFromData(CRCPacket& inPacket);
    virtual void interpolate(const C_ManagersContext& context, const C_Unit* prevUnit, 
                             const C_Unit* nextUnit, double t, double d);

    virtual void updateLocallyVisible(const C_ManagersContext& context);
    virtual void localReveal(const C_Unit* unit);

    //render function that pushes RenderNodes to the list or something
    //the base C_Entity render function just pushes one sprite with the textureId and pos and everything


    //buffs and status
};


// class Hero : public Unit
// {
//     //abilities and ultimate
//     //some sort of xp system
// };