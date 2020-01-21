#include "quadtree_entity.hpp"

template<typename T>
void SimpleExtractor<T>::ExtractBoundingBody(const QuadtreeEntity<T> *in, BoundingBody<T> *out) {
    out->isCircle = in->body.isCircle;
    
    if (in->body.isCircle) {
        out->circle.center = in->body.circle.center;
        out->circle.radius = in->body.circle.radius;
    }

    out->rect.left = in->body.rect.left;
    out->rect.top = in->body.rect.top;
    out->rect.width = in->body.rect.width;
    out->rect.height = in->body.rect.height;
    out->rect.angle = in->body.rect.angle;
}

template<typename T>
QuadtreeEntity<T>::QuadtreeEntity(u32 id, const RotatingRect<T> &rect) :
    uniqueId(id), body(rect)
{

}

template<typename T>
QuadtreeEntity<T>::QuadtreeEntity(u32 id, const Circle<T> &circle) :
    uniqueId(id), body(circle)
{

}
