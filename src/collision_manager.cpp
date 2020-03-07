#include "collision_manager.hpp"

#include <iostream>

#include "quadtree.hpp"

CollisionManager::CollisionManager()
{
    m_quadtree = std::unique_ptr<QuadtreeType>(new QuadtreeType());
}

CollisionManager::~CollisionManager()
{

}

void CollisionManager::onInsertEntity(u32 uniqueId, const Vector2& pos, u8 radius)
{
    Circlef circle(pos, (float) radius);
    m_entities[uniqueId] = std::unique_ptr<QuadtreeEntityType>(new QuadtreeEntityType(uniqueId, circle));

    m_quadtree->Insert(m_entities[uniqueId].get());
}

void CollisionManager::onUpdateUnit(u32 uniqueId, const Vector2& newPos, float newRadius)
{
    auto it = m_entities.find(uniqueId);

    if (it != m_entities.end()) {
        it->second->body.circle.center = newPos;
        it->second->body.circle.radius = newRadius;

        m_quadtree->Update(it->second.get());

    } else {
        std::cout <<  "CollisionManager::onUpdateUnit error - UniqueId doesn't exist" << std::endl;
    }
}

void CollisionManager::onDeleteUnit(u32 uniqueId)
{
    auto it = m_entities.find(uniqueId);

    if (it != m_entities.end()) {
        m_quadtree->Remove(it->second.get());
        m_entities.erase(it);

    } else {
        std::cout <<  "CollisionManager::onDeleteUnit error - UniqueId doesn't exist" << std::endl;
    }
}

QuadtreeType* CollisionManager::getQuadtree()
{
    return m_quadtree.get();
}
