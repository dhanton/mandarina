#include "entity_manager.hpp"

//the rest of the methods are implemented in .cpp file

template<typename T>
int EntityManager::createEntity(const Vector2& pos)
{
    const ComponentFlags& flags = T::defaultComponentFlags;

    int entityId = m_entityDataManager.getNewItemId();

    if (entityId == m_entities.size()) {
        m_entities.push_back(true);

        mIds_base.push_back(-1);
        mIds_collision.push_back(-1);
        mIds_render.push_back(-1);
    } else {
        mIds_base[entityId] = -1;
        mIds_collision[entityId] = -1;
        mIds_render[entityId] = -1;
    }

    {
        int baseId = m_baseDataManager.getNewItemId();

        if (baseId == mBaseComponent_pos.size()) {
            mBaseComponent_pos.push_back(pos);
            mBaseComponent_vel.push_back(Vector2());
        } else {
            mBaseComponent_pos[baseId] = pos;
        }

        mIds_base[entityId] = baseId;
    }

    if (flags.collision) {
        int collisionId = m_collisionDataManager.getNewItemId();

        if (collisionId == m_collisionComponent.size()) {
            m_collisionComponent.push_back(CollisionComponent());
        }

        mIds_collision[entityId] = collisionId;
    }

    if (flags.render) {
        int renderId = m_renderDataManager.getNewItemId();

        if (renderId == m_renderComponent.size()) {
            m_renderComponent.push_back(RenderComponent());
        }

        mIds_render[entityId] = renderId;
    }

    T::defaultValues(this, entityId);

    return entityId;
}
