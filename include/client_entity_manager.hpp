#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

#include "bucket.hpp"
#include "context.hpp"
#include "units.hpp"
#include "projectiles.hpp"

struct RenderNode {
    sf::Sprite sprite;

    //used to render debug collision shapes
    float collisionRadius;

    //all these parameters are used to sort the entities
    float flyingHeight;
    u32 uniqueId;
    int manualFilter;

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

    void performInterpolation(const C_EntityManager* prevSnapshot, const C_EntityManager* nextSnapshot, double elapsedTime, double totalTime);
    void copySnapshotData(const C_EntityManager* snapshot);
    void updateRevealedUnits();

    void loadFromData(C_EntityManager* prevSnapshot, CRCPacket& inPacket);

    void allocateAll();

    void setTileMap(TileMap* tileMap);
public:
    Bucket<C_Unit> units;
    Bucket<C_Projectile> projectiles;

    u32 controlledEntityUniqueId;
    u8 controlledEntityTeamId;
    
    TileMap* m_tileMap;

    //true if rendering collision shapes and other debug stuff
    bool renderingDebug;

    bool renderingLocallyHidden;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};
