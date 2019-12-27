#pragma once

#include <vector>

//This is called bucket for lack of a better term

template<typename T>
class Bucket
{
public:
    Bucket(int initialSize = 0);

    int addElement();
    void removeElement(int index);

    void copyValidDataTo(Bucket<T>& otherBucket) const;

    bool isIndexInRange(int index) const;
    bool isIndexValid(int index) const;
    int firstInvalidIndex() const;

    T& operator[](int index);
    const T& operator[](int index) const;

private:
    std::vector<T> m_elements;
    int m_firstInvalidIndex;
};

#include "bucket.inl"
