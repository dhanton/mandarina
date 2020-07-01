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

void CollisionManager::onUpdateEntity(u32 uniqueId, const Vector2& newPos, float newRadius)
{
    auto it = m_entities.find(uniqueId);

    if (it != m_entities.end()) {
        //minimizes the amount of calls to Quadtree::Update
        bool needsUpdate = false;

        if (it->second->body.circle.center != newPos) {
            it->second->body.circle.center = newPos;
            needsUpdate = true;
        }

        if (it->second->body.circle.radius != newRadius) {
            it->second->body.circle.radius = newRadius;
            needsUpdate = true;
        }

        if (needsUpdate) {
            m_quadtree->Update(it->second.get());
        }

    } else {
        std::cout <<  "CollisionManager::onUpdateUnit error - UniqueId doesn't exist" << std::endl;
    }
}

void CollisionManager::onDeleteEntity(u32 uniqueId)
{
    auto it = m_entities.find(uniqueId);

    if (it != m_entities.end()) {
        m_quadtree->Remove(it->second.get());
        m_entities.erase(it);

    } else {
        std::cout <<  "CollisionManager::onDeleteUnit error - UniqueId doesn't exist" << std::endl;
    }
}

void CollisionManager::clear()
{
    m_quadtree->Clear();
    m_entities.clear();
}

QuadtreeType* CollisionManager::getQuadtree()
{
    return m_quadtree.get();
}
