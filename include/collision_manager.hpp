#pragma once

#include <memory>
#include <unordered_map>

#include "quadtree_entity.hpp"
#include "quadtree.hpp"

using QuadtreeType = Quadtree<float, QuadtreeEntityType, SimpleExtractor<float>>;

class CollisionManager
{
public:
    CollisionManager();
    ~CollisionManager();

    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;

    void onInsertEntity(u32 uniqueId, const Vector2& pos, u8 radius);
    void onUpdateUnit(u32 uniqueId, const Vector2& newPos, float newRadius);
    void onDeleteUnit(u32 uniqueId);

    QuadtreeType* getQuadtree();

private:
    std::unique_ptr<QuadtreeType> m_quadtree;
    std::unordered_map<u32, std::unique_ptr<QuadtreeEntityType>> m_entities;
};
