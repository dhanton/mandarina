#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

#include "bucket.hpp"
#include "context.hpp"
#include "projectiles.hpp"

#include "entity_table.hpp"
#include "entity.hpp"
#include "unit.hpp"

struct RenderNode {
    //the RenderNode can have ownership of the drawabale
    sf::Sprite sprite;

    //or it can be delegated somewhere else
    const sf::Drawable* drawable;

    //flag that's true if the node has ownership of its drawable
    bool usingSprite;

    //all these parameters are used to sort the entities
    float height;
    u32 uniqueId;
    int manualFilter;

#ifdef MANDARINA_DEBUG
    std::string debugDisplayData;

    //Used to properly position text and debug collision shapes
    //(since sf::Drawable doesn't store position)
    Vector2 position;

    //used to render debug collision shapes
    float collisionRadius;
#endif

    RenderNode(float height, u32 uniqueId);

    inline bool operator<(const RenderNode& other);
};

class C_EntityManager : public InContext, public sf::Drawable
{
public:
    C_EntityManager(const Context& context, sf::Time worldTime = sf::Time::Zero);

    //used by snapshots
    C_EntityManager();
    
    void update(sf::Time eTime);
    void renderUpdate(sf::Time eTime);

    void performInterpolation(const C_EntityManager* prevSnapshot, const C_EntityManager* nextSnapshot, double elapsedTime, double totalTime);
    void copySnapshotData(const C_EntityManager* snapshot, u32 latestAppliedInputId);
    void updateRevealedUnits();

    C_Projectile* createProjectile(u8 projectileType, const Vector2& pos, float aimAngle, u8 teamId);
    C_Entity* createEntity(u8 entityType, u32 uniqueId);

    void loadFromData(C_EntityManager* prevSnapshot, CRCPacket& inPacket, CasterSnapshot& casterSnapshot);

    void allocateAll();

    void setTileMap(TileMap* tileMap);

    u8 getLocalTeamId() const;
    u32 getLocalUniqueId() const;

    u8 getControlledEntityTeamId() const;
    void setControlledEntityTeamId(u8 teamId);

    u32 getControlledEntityUniqueId() const;
    void setControlledEntityUniqueId(u32 uniqueId);

    void setSpectatingEntityTeamId(u8 teamId);
    void setSpectatingEntityUniqueId(u32 uniqueId);

    bool isHeroDead() const;
    void setHeroDead(bool heroDead);

    std::vector<RenderNode>& getRenderNodes();
    std::vector<RenderNode>& getUIRenderNodes();

public:
    EntityTable<C_Entity> entities;
    Bucket<C_Projectile> projectiles;

    Bucket<C_Projectile> localProjectiles;
    u32 localLastUniqueId;

    TileMap* m_tileMap;

#ifdef MANDARINA_DEBUG
    //true if rendering collision shapes and other debug stuff
    bool renderingDebug;
    bool renderingLocallyHidden;
    bool renderingEntityData;
#endif

    bool renderingEntitiesUI;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    
    std::vector<RenderNode> m_renderNodes;
    std::vector<RenderNode> m_uiRenderNodes;

    static bool m_entitiesJsonLoaded;
    static std::unique_ptr<C_Entity> m_entityData[ENTITY_MAX_TYPES];

    static void loadEntityData(const Context& context);
    
    u32 m_controlledEntityUniqueId;
    u8 m_controlledEntityTeamId;

    u32 m_spectatingEntityUniqueId;
    u8 m_spectatingEntityTeamId;

    bool m_heroDead;
};
