#include "bucket.hpp"

#include <iostream>

template<typename T>
Bucket<T>::Bucket(int initialSize)
{
    m_firstInvalidIndex = 0;

    if (initialSize != 0) {
        m_elements.resize(initialSize);
    }
}

template<typename T>
void Bucket<T>::resize(int size)
{
    m_elements.resize(size);
}

template<typename T>
int Bucket<T>::addElement(u32 uniqueId)
{
    if (m_hashTable.find(uniqueId) != m_hashTable.end()) {
        std::cout << "Bucket::addElement error - UniqueId already exists" << std::endl;
        return -1;
    }

    if (m_firstInvalidIndex >= m_elements.size()) {
        m_elements.emplace_back();
    } else {
        m_elements[m_firstInvalidIndex] = T();
    }

    int index = m_firstInvalidIndex++;
    m_hashTable.insert(std::make_pair(uniqueId, index));

    return index;
}

template<typename T>
void Bucket<T>::removeElement(u32 uniqueId)
{
    int index = getIndexByUniqueId(uniqueId);

    if (index != -1) {
        m_firstInvalidIndex--;

        if (index != m_firstInvalidIndex) {            
            m_hashTable[m_elements[m_firstInvalidIndex].uniqueId] = index;
            std::swap(m_elements[index], m_elements[m_firstInvalidIndex]);
        }

        m_hashTable.erase(uniqueId);

    } else {
        std::cout << "Bucket::removeElement error - UniqueId doesn't exist" << std::endl;
    }
}

template<typename T>
void Bucket<T>::removeInvalidData()
{
    while (m_elements.size() > m_firstInvalidIndex) {
        m_elements.pop_back();
    }
}

template<typename T>
void Bucket<T>::copyValidDataTo(Bucket<T>& otherBucket) const
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

    // otherBucket.m_elements = std::vector<T>(m_elements.begin(), m_elements.begin() + m_firstInvalidIndex);
    // otherBucket.m_hashTable = m_hashTable;
}

template<typename T>
bool Bucket<T>::isIndexInRange(int index) const
{
    return (index >= 0 && index < m_elements.size());
}

template<typename T>
bool Bucket<T>::isIndexValid(int index) const
{
    return (isIndexInRange(index) && index < m_firstInvalidIndex);
}

template<typename T>
int Bucket<T>::firstInvalidIndex() const
{
    return m_firstInvalidIndex;
}

template<typename T>
T& Bucket<T>::operator[](int index)
{
    return m_elements[index];
}

template<typename T>
const T& Bucket<T>::operator[](int index) const
{
    return m_elements[index];
}

template<typename T>
int Bucket<T>::getIndexByUniqueId(u32 uniqueId) const
{
    auto it = m_hashTable.find(uniqueId);
    if (it != m_hashTable.end()) {
        return it->second;
    }

    return -1;
}

template<typename T>
T* Bucket<T>::atIndex(int index)
{
    if (!isIndexValid(index)) return nullptr;

    return &m_elements[index];
}

template<typename T>    
const T* Bucket<T>::atIndex(int index) const
{
    if (!isIndexValid(index)) return nullptr;

    return &m_elements[index];
}

template<typename T>
T* Bucket<T>::atUniqueId(u32 uniqueId)
{
    int index = getIndexByUniqueId(uniqueId);
    if (index != -1) {
        return &m_elements[index];
    }
    return nullptr;
}

template<typename T>
const T* Bucket<T>::atUniqueId(u32 uniqueId) const
{
    int index = getIndexByUniqueId(uniqueId);
    if (index != -1) {
        return &m_elements[index];
    }
    return nullptr;
}

template<typename T>
void Bucket<T>::clear()
{
    m_firstInvalidIndex = 0;
    m_hashTable.clear();
}