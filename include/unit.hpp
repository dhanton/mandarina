#pragma once

#include <list>
#include "entity.hpp"
#include "caster_component.hpp"
#include "json_parser.hpp"
#include "unit_ui.hpp"
#include "buff_holder_component.hpp"
#include "caster_snapshot.hpp"

class _UnitBase
{
public:
    _UnitBase() = default;

    float getAimAngle() const;
    void setAimAngle(float aimAngle);

    u8 getMovementSpeed() const;
    void setMovementSpeed(u8 movementSpeed);

    Status& getStatus();
    const Status& getStatus() const;

    static Vector2 moveCollidingTilemap_impl(const Vector2& oldPos, Vector2 newPos, float collisionRadius, TileMap* map);

protected:
    void loadFromJson(const rapidjson::Document& doc);

protected:
    float m_aimAngle;
    u8 m_weaponId;
    u16 m_movementSpeed;

    Status m_status;

private:
    static bool m_abilitiesLoaded;
};

//IMPORTANT
//CasterComponent and BuffHolderComponents should only be inherited by Unit (only units can hold buffs and cast abilities)
//They're made into components so that internal unique_ptrs can be properly handled when units ared copied

class Unit : public Entity, public _UnitBase, public HealthComponent, 
             public InvisibleComponent, public TrueSightComponent,
             public CasterComponent, public BuffHolderComponent
{
private:
    TRUE_SIGHT_COMPONENT()
    INVISIBLE_COMPONENT()
    CASTER_COMPONENT()

public:
    virtual Unit* clone() const;
    virtual ~Unit() = default;
    
    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void update(sf::Time eTime, const ManagersContext& context);
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const;

    virtual void onTakeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId);
    virtual void onBeHealed(u16 amount, Entity* source);

    virtual void applyInput(const PlayerInput& input, const ManagersContext& context, u16 clientDelay);

    virtual bool shouldSendToTeam(u8 teamId) const;

    virtual void onQuadtreeInserted(const ManagersContext& context);

private:
    void moveColliding(Vector2 newPos, const ManagersContext& context, bool force = false);

    Vector2 m_prevPos;
    u8 m_prevCollisionRadius;
};

struct RenderNode;

class C_Unit : public C_Entity, public _UnitBase, public HealthComponent,
               public TrueSightComponent
{
private:
    TRUE_SIGHT_COMPONENT()
    BUFF_HOLDER_COMPONENT()

public:
    virtual C_Unit* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context);

    virtual void update(sf::Time eTime, const C_ManagersContext& context);
    virtual void loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot);
    virtual void interpolate(const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d, bool isControlled);

    virtual void copySnapshotData(const C_Entity* snapshotEntity, bool isControlled);

    virtual void updateControlledAngle(float aimAngle);
    
    virtual void applyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context, sf::Time dt);
    virtual void reapplyMovementInput(Vector2& pos, PlayerInput& input, const C_ManagersContext& context);

    virtual u16 getControlledMovementSpeed() const;

    virtual void updateLocallyVisible(const C_ManagersContext& context);
    virtual void localReveal(C_Entity* unit);
    virtual void insertRenderNode(const C_ManagersContext& managersContext, const Context& context);

    UnitUI* getUnitUI();
    const UnitUI* getUnitUI() const;

    bool isLocallyHidden() const;
    void setLocallyHidden(bool locallyHidden);
    bool isServerRevealed() const;
    void setServerRevealed(bool serverHidden);
    bool shouldBeHiddenFrom(const C_Unit& unit) const;

private:
    void predictMovementLocally(const Vector2& oldPos, Vector2& newPos, const C_ManagersContext& context) const;

    UnitUI m_ui;

    bool m_locallyHidden;
    bool m_serverRevealed;
    bool m_invisible;
};
