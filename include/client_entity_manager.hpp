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
    sf::Sprite sprite;

    //used to render debug collision shapes
    float collisionRadius;

    //all these parameters are used to sort the entities
    float flyingHeight;
    u32 uniqueId;
    int manualFilter;

#ifdef MANDARINA_DEBUG
    std::string debugDisplayData; 
#endif

    RenderNode(float flyingHeight, u32 uniqueId, float collisionRadius);

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

    int createProjectile(ProjectileType type, const Vector2& pos, float aimAngle, u8 teamId);
    C_Entity* createEntity(u8 entityType, u32 uniqueId);

    void loadFromData(C_EntityManager* prevSnapshot, CRCPacket& inPacket);

    void allocateAll();

    void setTileMap(TileMap* tileMap);

    std::vector<RenderNode>& getRenderNodes();

public:
    EntityTable<C_Entity> entities;
    Bucket<C_Projectile> projectiles;

    Bucket<C_Projectile> localProjectiles;
    u32 localLastUniqueId;

    u32 controlledEntityUniqueId;
    u8 controlledEntityTeamId;

    TileMap* m_tileMap;

    //true if rendering collision shapes and other debug stuff
    bool renderingDebug;

    bool renderingLocallyHidden;

    bool renderingEntityData;


private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    
    std::vector<RenderNode> m_renderNodes;

    static bool m_entitiesJsonLoaded;
    static std::unique_ptr<C_Entity> m_entityData[ENTITY_MAX_TYPES];

    static void loadEntityData(const JsonParser* jsonParser);
};
