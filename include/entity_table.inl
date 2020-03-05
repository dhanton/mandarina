#include "entity_table.hpp"

#include <iostream>

template<typename _Entity_Type>
_Entity_Type* EntityTable<_Entity_Type>::atUniqueId(u32 uniqueId)
{
    typename _Table::iterator it = m_table.find(uniqueId);

    if (it == m_table.end()) {
        return nullptr;
    } else {
        return it->second.get();
    }
}

template<typename _Entity_Type>
const _Entity_Type* EntityTable<_Entity_Type>::atUniqueId(u32 uniqueId) const
{
    typename _Table::const_iterator it = m_table.find(uniqueId);

    if (it == m_table.end()) {
        return nullptr;
    } else {
        return it->second.get();
    }
}

template<typename _Entity_Type>
_Entity_Type* EntityTable<_Entity_Type>::addEntity(_Entity_Type* entity)
{
    if (!entity) {
        std::cout << "EntityTable::addEntity error - Invalid entity pointer" << std::endl;
        return nullptr;
    }

    if (entity->getUniqueId() == 0 || m_table.find(entity->getUniqueId()) != m_table.end()) {
        std::cout << "EntityTable::addEntity error - Invalid or repeated uniqueId" << std::endl;
        return nullptr;
    }

    m_table.insert(std::make_pair(entity->getUniqueId(), std::move(std::unique_ptr<_Entity_Type>(entity))));

    return entity;
}

template<typename _Entity_Type>
typename EntityTable<_Entity_Type>::iterator EntityTable<_Entity_Type>::removeEntity(u32 uniqueId)
{
    typename _Table::iterator it = m_table.begin();

    if (it != m_table.end()) {
        return iterator(m_table.erase(it));
    } else {
        return iterator(m_table.end());
    }
}

template<typename _Entity_Type>
typename EntityTable<_Entity_Type>::iterator EntityTable<_Entity_Type>::removeEntity(iterator it)
{
    return iterator(m_table.erase(it._internal_it));
}

template<typename _Entity_Type>
typename EntityTable<_Entity_Type>::iterator EntityTable<_Entity_Type>::begin()
{
    return iterator(m_table.begin());
}

template<typename _Entity_Type>
typename EntityTable<_Entity_Type>::const_iterator EntityTable<_Entity_Type>::begin() const
{
    return const_iterator(m_table.begin());
}

template<typename _Entity_Type>
typename EntityTable<_Entity_Type>::iterator EntityTable<_Entity_Type>::end()
{
    return iterator(m_table.end());
}

template<typename _Entity_Type>
typename EntityTable<_Entity_Type>::const_iterator EntityTable<_Entity_Type>::end() const
{
    return const_iterator(m_table.end());
}

template<typename _Entity_Type>
size_t EntityTable<_Entity_Type>::size() const
{
    return m_table.size();
}