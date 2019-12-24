#include "entity_manager.hpp"

#include <iostream>

//template methods are implemented in .inl file

void EntityManager::destroyEntity(int entityId)
{
    if (!validEntityId(entityId)) {
        std::cout << "destroyEntity error - Invalid entityId" << std::endl;
        return;
    }

    m_entities[entityId] = -1;
    m_entityDataManager.itemIdRemoved(entityId);
}

bool EntityManager::validEntityId(int entityId) const
{
    return (entityId >= 0 && entityId < m_entities.size());
}
