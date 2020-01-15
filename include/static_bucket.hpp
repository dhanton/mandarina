#pragma once

#include <unordered_map>

#include "defines.hpp"

//@WIP: Maybe use this for entities
//and the other one for snapshots

template<typename T> 
class Bucket;

template<typename T, size_t N>
class StaticBucket
{
public:
    StaticBucket();

    int addElement(u32 uniqueId);
    void removeElement(u32 uniqueId);

    //@WIP: Allow to copy data to a dynamic bucket
    template<typename OtherT>
    void copyValidDataTo(Bucket<OtherT>& otherBucket) const;

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

private:
    T m_elements[N];
    std::unordered_map<u32, int> m_hashTable;

    int m_firstInvalidIndex;
};

#include "static_bucket.inl"
