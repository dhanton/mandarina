#pragma once

#include "entity.hpp"
#include "ability.hpp"
#include "json_parser.hpp"

class _UnitBase
{
public:
    _UnitBase() = default;

    static void loadAbilityData(const JsonParser* jsonParser);
    
    //rule of 5 (methods needed because this class has unique_ptr and we want to copy it)
    ~_UnitBase() = default;
    _UnitBase(_UnitBase const& other);
    _UnitBase(_UnitBase && other) = default;
    _UnitBase& operator=(_UnitBase const& other);
    _UnitBase& operator=(_UnitBase && other) = default;

    float getAimAngle() const;
    void setAimAngle(float aimAngle);

    u8 getMovementSpeed() const;
    void setMovementSpeed(u8 movementSpeed);

protected:
    void loadFromJson(const rapidjson::Document& doc);

protected:
    static std::unique_ptr<Ability> m_abilityData[ABILITY_MAX_TYPES];

    float m_aimAngle;
    u8 m_weaponId;
    u16 m_movementSpeed;
    
    std::unique_ptr<Ability> m_primaryFire;
    std::unique_ptr<Ability> m_secondaryFire;
    std::unique_ptr<Ability> m_altAbility;
    std::unique_ptr<Ability> m_ultimate;

    //Buffs

private:
    static bool m_abilitiesLoaded;
};

class Unit : public Entity, public _UnitBase, public HealthComponent, 
             public InvisibleComponent, public TrueSightComponent
{
private:
    TRUE_SIGHT_COMPONENT()
    INVISIBLE_COMPONENT()

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
    virtual void applyAbilitiesInput(const PlayerInput& input, const C_ManagersContext& context);

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