#include "static_bucket.hpp"

#include <iostream>

#include "bucket.hpp"

template<typename T, size_t N>
StaticBucket<T, N>::StaticBucket()
{
    m_firstInvalidIndex = 0;
}

template<typename T, size_t N>
int StaticBucket<T, N>::addElement(u32 uniqueId)
{
    if (m_hashTable.find(uniqueId) != m_hashTable.end()) {
        std::cout << "StaticBucket::addElement error - UniqueId already exists" << std::endl;
        return -1;
    }

    if (m_firstInvalidIndex >= N) {
        std::cout << "StaticBucket::addElement error - Maximum Size exceeded" << std::endl;
        return -1;
    }

    m_elements[m_firstInvalidIndex] = T();

    int index = m_firstInvalidIndex++;
    m_hashTable.insert(std::make_pair(uniqueId, index));

    return index;
}

template<typename T, size_t N>
void StaticBucket<T, N>::removeElement(u32 uniqueId)
{
    int index = getIndexByUniqueId(uniqueId);

    if (index != -1) {
        m_firstInvalidIndex--;
        m_hashTable[m_elements[m_firstInvalidIndex].uniqueId] = index;
        std::swap(m_elements[index], m_elements[m_firstInvalidIndex]);
        m_hashTable.erase(uniqueId);

    } else {
        std::cout << "StaticBucket::removeElement error - UniqueId doesn't exist" << std::endl;
    }
}

template<typename T, size_t N>
template<typename OtherT>
void StaticBucket<T, N>::copyValidDataTo(Bucket<OtherT>& otherBucket) const
{
    if (m_firstInvalidIndex == 0) return;

    //clear containers
    otherBucket.m_firstInvalidIndex = m_firstInvalidIndex;
    otherBucket.m_hashTable.clear();

    if (otherBucket.m_elements.size() < m_firstInvalidIndex) {
        otherBucket.resize(m_firstInvalidIndex);
    }

    for (int i = 0; i < m_firstInvalidIndex; ++i) {
        otherBucket.m_elements[i] = m_elements[i];
        otherBucket.m_hashTable.insert(std::make_pair(m_elements[i].uniqueId, i));
    }
}

template<typename T, size_t N>
bool StaticBucket<T, N>::isIndexInRange(int index) const
{
    return (index >= 0 && index < N);
}

template<typename T, size_t N>
bool StaticBucket<T, N>::isIndexValid(int index) const
{
    return (isIndexInRange(index) && index < m_firstInvalidIndex);
}

template<typename T, size_t N>
int StaticBucket<T, N>::firstInvalidIndex() const
{
    return m_firstInvalidIndex;
}

template<typename T, size_t N>
T& StaticBucket<T, N>::operator[](int index)
{
    return m_elements[index];
}

template<typename T, size_t N>
const T& StaticBucket<T, N>::operator[](int index) const
{
    return m_elements[index];
}

template<typename T, size_t N>
int StaticBucket<T, N>::getIndexByUniqueId(u32 uniqueId) const
{
    auto it = m_hashTable.find(uniqueId);
    if (it != m_hashTable.end()) {
        return it->second;
    }

    return -1;
}

template<typename T, size_t N>
T* StaticBucket<T, N>::atIndex(int index)
{
    if (!isIndexValid(index)) return nullptr;

    return &m_elements[index];
}

template<typename T, size_t N>    
const T* StaticBucket<T, N>::atIndex(int index) const
{
    if (!isIndexValid(index)) return nullptr;

    return &m_elements[index];
}

template<typename T, size_t N>
T* StaticBucket<T, N>::atUniqueId(u32 uniqueId)
{
    int index = getIndexByUniqueId(uniqueId);
    if (index != -1) {
        return &m_elements[index];
    }
    return nullptr;
}

template<typename T, size_t N>
const T* StaticBucket<T, N>::atUniqueId(u32 uniqueId) const
{
    int index = getIndexByUniqueId(uniqueId);
    if (index != -1) {
        return &m_elements[index];
    }
    return nullptr;
}
