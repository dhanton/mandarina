#pragma once

#include <vector>
#include <unordered_map>

#include "defines.hpp"

//This is called bucket for lack of a better term

//Container that keeps data tight in memory
//Uses a hash table to access data by a unique identifier
//This uniqueId has to be handled somewhere else

template<typename T>
class Bucket
{
public:
    Bucket(int initialSize = 0);

    void resize(int size);

    int addElement(u32 uniqueId);
    void removeElement(u32 uniqueId);

    void removeInvalidData();
    void copyValidDataTo(Bucket<T>& otherBucket) const;

    bool isIndexInRange(int index) const;
    bool isIndexValid(int index) const;
    int firstInvalidIndex() const;

    T& operator[](int index);
    const T& operator[](int index) const;

    int getIndexByUniqueId(u32 uniqueId) const;

    //returns nullptr if element is not found
    T* atIndex(int index);
    const T* atIndex(int index) const;
    
    //returns nullptr if element is not found
    T* atUniqueId(u32 uniqueId);
    const T* atUniqueId(u32 uniqueId) const;

    void clear();

private:
    std::vector<T> m_elements;
    std::unordered_map<u32, int> m_hashTable;

    int m_firstInvalidIndex;
};

#include "bucket.inl"
