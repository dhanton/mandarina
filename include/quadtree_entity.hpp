#pragma once

#include "bounding_body.hpp"

template <typename T>
struct QuadtreeEntity
{
    QuadtreeEntity(u32 id, const RotatingRect<T>& rect);
    QuadtreeEntity(u32 id, const Circle<T> &circle);

    u32 uniqueId;
    BoundingBody<T> body;
};

template <typename T>
class SimpleExtractor {
public:
    static void ExtractBoundingBody(const QuadtreeEntity<T> *in, BoundingBody<T> *out);
};

using QuadtreeEntityType = QuadtreeEntity<float>;

#include "quadtree_entity.inl"
