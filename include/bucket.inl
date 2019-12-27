#include "bucket.hpp"

#include <iostream>

template<typename T>
Bucket<T>::Bucket(int initialSize)
{
    m_firstInvalidIndex = 0;

    m_elements.resize(initialSize);
}

template<typename T>
int Bucket<T>::addElement()
{
    if (m_firstInvalidIndex >= m_elements.size()) {
        m_elements.push_back(T());
    } else {
        m_elements[m_firstInvalidIndex] = T();
    }

    return m_firstInvalidIndex++;
}

template<typename T>
void Bucket<T>::removeElement(int index)
{
    m_firstInvalidIndex--;
    std::swap(m_elements[index], m_elements[m_firstInvalidIndex]);
}

template<typename T>
void Bucket<T>::copyValidDataTo(Bucket<T>& otherBucket) const
{
    otherBucket.m_elements = std::vector<T>(m_elements.begin(), m_elements.begin() + m_firstInvalidIndex);
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
