#pragma once

#include <vector>

#include "defines.hpp"
#include "data_oriented_manager.hpp"
#include "components.hpp"

class EntityManager
{
public:
    template<typename T>
    int createEntity(const Vector2& pos);

    void destroyEntity(int entityId);

    bool validEntityId(int entityId) const;

// private:

    DataOrientedManager m_entityDataManager;
    std::vector<bool> m_entities;
    std::vector<int> mIds_base;
    std::vector<int> mIds_collision;
    std::vector<int> mIds_render;

    DataOrientedManager m_baseDataManager;
    std::vector<Vector2> mBaseComponent_pos;
    std::vector<Vector2> mBaseComponent_vel;

    DataOrientedManager m_collisionDataManager;
    std::vector<CollisionComponent> m_collisionComponent;

    DataOrientedManager m_renderDataManager;
    std::vector<RenderComponent> m_renderComponent;
};

#include "entity_manager.inl"
