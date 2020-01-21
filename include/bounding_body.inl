#include "bounding_body.hpp"

#include <SFML/Graphics/Transform.hpp>
#include "helper.hpp"

template <typename T>
RotatingRect<T>::RotatingRect() :
    RotatingRect(0, 0, 0, 0, 0)
{

}

template <typename T>
RotatingRect<T>::RotatingRect(const sf::Rect<T>& rect, T _angle) :
    RotatingRect(rect.left, rect.top, rect.width, rect.height, _angle)
{

}

template<typename T>
RotatingRect<T>::RotatingRect(T _left, T _top, T _width, T _height, T _angle) :
    left(_left), top(_top), width(_width), height(_height), angle(_angle)
{

}

template<typename T>
RotatingRect<T>::RotatingRect(const sf::Vector2<T> &pos, const sf::Vector2<T> &size, T _angle) :
    RotatingRect(pos.x, pos.y, size.x, size.y, _angle)
{

}

template<typename T>
bool RotatingRect<T>::intersects(const RotatingRect<T> &other) const
{
    if (other.angle == 0 && angle == 0) {
        return getNonRotatingRect().intersects(other.getNonRotatingRect());
    }

    //Check against this axis
    sf::Vector2<T> point1 = {other.left, other.top};
    sf::Vector2<T> point2 = {other.left + other.width, other.top};
    sf::Vector2<T> point3 = {other.left + other.width, other.top + other.height};
    sf::Vector2<T> point4 = {other.left, other.top + other.height};

    sf::Transform transform;
    transform.rotate(-angle, getCenter());
    transform.rotate(other.angle, other.getCenter());

    point1 = transform.transformPoint(point1);

    sf::Vector2<T> otherMin = point1;
    sf::Vector2<T> otherMax = point1;

    point2 = transform.transformPoint(point2);
    if (point2.x < otherMin.x) otherMin.x = point2.x;
    if (point2.y < otherMin.y) otherMin.y = point2.y;
    if (point2.x > otherMax.x) otherMax.x = point2.x;
    if (point2.y > otherMax.y) otherMax.y = point2.y;

    point3 = transform.transformPoint(point3);
    if (point3.x < otherMin.x) otherMin.x = point3.x;
    if (point3.y < otherMin.y) otherMin.y = point3.y;
    if (point3.x > otherMax.x) otherMax.x = point3.x;
    if (point3.y > otherMax.y) otherMax.y = point3.y;

    point4 = transform.transformPoint(point4);
    if (point4.x < otherMin.x) otherMin.x = point4.x;
    if (point4.y < otherMin.y) otherMin.y = point4.y;
    if (point4.x > otherMax.x) otherMax.x = point4.x;
    if (point4.y > otherMax.y) otherMax.y = point4.y;

    if (otherMin.x > left + width || otherMax.x < left) return false;
    if (otherMin.y > top + height || otherMax.y < top) return false;

    //Check against the other axis
    point1 = {left, top};
    point2 = {left + width, top};
    point3 = {left + width, top + height};
    point4 = {left, top + height};

    transform = sf::Transform::Identity;
    transform.rotate(-other.angle, other.getCenter());
    transform.rotate(angle, getCenter());

    point1 = transform.transformPoint(point1);

    sf::Vector2<T> thisMin = point1;
    sf::Vector2<T> thisMax = point1;

    point2 = transform.transformPoint(point2);
    if (point2.x < thisMin.x) thisMin.x = point2.x;
    if (point2.y < thisMin.y) thisMin.y = point2.y;
    if (point2.x > thisMax.x) thisMax.x = point2.x;
    if (point2.y > thisMax.y) thisMax.y = point2.y;

    point3 = transform.transformPoint(point3);
    if (point3.x < thisMin.x) thisMin.x = point3.x;
    if (point3.y < thisMin.y) thisMin.y = point3.y;
    if (point3.x > thisMax.x) thisMax.x = point3.x;
    if (point3.y > thisMax.y) thisMax.y = point3.y;

    point4 = transform.transformPoint(point4);
    if (point4.x < thisMin.x) thisMin.x = point4.x;
    if (point4.y < thisMin.y) thisMin.y = point4.y;
    if (point4.x > thisMax.x) thisMax.x = point4.x;
    if (point4.y > thisMax.y) thisMax.y = point4.y;

    if (thisMin.x > other.left + other.width || thisMax.x < other.left) return false;
    if (thisMin.y > other.top + other.height || thisMax.y < other.top) return false;

    return true;
}

template<typename T>
bool RotatingRect<T>::intersects(const Circle<T> &circle) const
{
    Circle<T> inversedCircle = circle;
    Helper_rotatePoint(inversedCircle.center, -angle, getCenter());

    return inversedCircle.intersects(getNonRotatingRect());
}

template<typename T>
bool RotatingRect<T>::contains(const RotatingRect<T> &other) const
{
    if (other.angle == 0 && angle == 0) {
        sf::Rect<T> rect1 = getNonRotatingRect();
        sf::Rect<T> rect2 = other.getNonRotatingRect();

        return rect2.left >= rect1.left && rect2.top >= rect1.top &&
                           (rect2.left + rect2.width) <= (rect1.left + rect1.width) && (rect2.top + rect2.height) <= (rect1.top + rect1.height);
    }

    //Check against this axis
    sf::Vector2<T> point1 = {other.left, other.top};
    sf::Vector2<T> point2 = {other.left + other.width, other.top};
    sf::Vector2<T> point3 = {other.left + other.width, other.top + other.height};
    sf::Vector2<T> point4 = {other.left, other.top + other.height};

    sf::Transform transform;
    transform.rotate(-angle, getCenter());
    transform.rotate(other.angle, other.getCenter());

    point1 = transform.transformPoint(point1);

    sf::Vector2<T> otherMin = point1;
    sf::Vector2<T> otherMax = point1;

    point2 = transform.transformPoint(point2);
    if (point2.x < otherMin.x) otherMin.x = point2.x;
    if (point2.y < otherMin.y) otherMin.y = point2.y;
    if (point2.x > otherMax.x) otherMax.x = point2.x;
    if (point2.y > otherMax.y) otherMax.y = point2.y;

    point3 = transform.transformPoint(point3);
    if (point3.x < otherMin.x) otherMin.x = point3.x;
    if (point3.y < otherMin.y) otherMin.y = point3.y;
    if (point3.x > otherMax.x) otherMax.x = point3.x;
    if (point3.y > otherMax.y) otherMax.y = point3.y;

    point4 = transform.transformPoint(point4);
    if (point4.x < otherMin.x) otherMin.x = point4.x;
    if (point4.y < otherMin.y) otherMin.y = point4.y;
    if (point4.x > otherMax.x) otherMax.x = point4.x;
    if (point4.y > otherMax.y) otherMax.y = point4.y;

    if (otherMin.x > left + width || otherMax.x < left) return false;
    if (otherMin.y > top + height || otherMax.y < top) return false;

    if (otherMin.x < left || otherMax.x > left + width) return false;
    if (otherMin.y < top || otherMax.y > top + height) return false;

    //Check against the other axis
    point1 = {left, top};
    point2 = {left + width, top};
    point3 = {left + width, top + height};
    point4 = {left, top + height};

    transform = sf::Transform::Identity;
    transform.rotate(-other.angle, other.getCenter());
    transform.rotate(angle, getCenter());

    point1 = transform.transformPoint(point1);

    sf::Vector2<T> thisMin = point1;
    sf::Vector2<T> thisMax = point1;

    point2 = transform.transformPoint(point2);
    if (point2.x < thisMin.x) thisMin.x = point2.x;
    if (point2.y < thisMin.y) thisMin.y = point2.y;
    if (point2.x > thisMax.x) thisMax.x = point2.x;
    if (point2.y > thisMax.y) thisMax.y = point2.y;

    point3 = transform.transformPoint(point3);
    if (point3.x < thisMin.x) thisMin.x = point3.x;
    if (point3.y < thisMin.y) thisMin.y = point3.y;
    if (point3.x > thisMax.x) thisMax.x = point3.x;
    if (point3.y > thisMax.y) thisMax.y = point3.y;

    point4 = transform.transformPoint(point4);
    if (point4.x < thisMin.x) thisMin.x = point4.x;
    if (point4.y < thisMin.y) thisMin.y = point4.y;
    if (point4.x > thisMax.x) thisMax.x = point4.x;
    if (point4.y > thisMax.y) thisMax.y = point4.y;

    if (thisMin.x > other.left + other.width || thisMax.x < other.left) return false;
    if (thisMin.y > other.top + other.height || thisMax.y < other.top) return false;

    if (other.left < thisMin.x || other.left + other.width > thisMax.x) return false;
    if (other.top < thisMin.y || other.top + other.height > thisMax.y) return false;

    return true;
}

template<typename T>
bool RotatingRect<T>::contains(const Circle<T> &circle) const
{
    Circle<T> inversedCircle = circle;
    Helper_rotatePoint(inversedCircle.center, -angle, getCenter());

    return inversedCircle.inside(getNonRotatingRect());
}

template<typename T>
bool RotatingRect<T>::inside(const Circle<T> &circle) const
{
    Circle<T> inversedCircle = circle;
    Helper_rotatePoint(inversedCircle.center, -angle, getCenter());

    return inversedCircle.contains(getNonRotatingRect());
}

template<typename T>
bool RotatingRect<T>::contains(const sf::Vector2<T> &point) const
{
    return contains(point.x, point.y);
}

template<typename T>
bool RotatingRect<T>::contains(T x, T y) const
{
    sf::Vector2<T> inversedPoint(x, y);
    Helper_rotatePoint(inversedPoint, -angle, getCenter());

    return getNonRotatingRect().contains(inversedPoint);
}

template<typename T>
sf::Vector2<T> RotatingRect<T>::getCenter() const
{
    return sf::Vector2<T>(left + width/((T) 2), top + height/((T) 2));
}

template<typename T>
sf::Vector2<T> RotatingRect<T>::getLocalOrigin() const
{
    return sf::Vector2<T>(width/((T) 2), height/((T) 2));
}

template<typename T>
sf::Rect<T> RotatingRect<T>::getNonRotatingRect() const
{
    return sf::Rect<T>(left, top, width, height);
}

template<typename T>
sf::Rect<T> RotatingRect<T>::getGlobalBounds() const
{
    sf::Transform transform;
    transform.rotate(angle, getCenter());

    return transform.transformRect(getNonRotatingRect());
}

template<typename T>
Circle<T>::Circle() :
    Circle((T) 0, (T) 0, (T) 0)
{

}

template <typename T>
Circle<T>::Circle(T _x, T _y, T _radius) :
    center(_x, _y), radius(_radius)
{

}

template <typename T>
Circle<T>::Circle(const sf::Vector2<T>& _center, T _radius) :
    center(_center), radius(_radius)
{

}

template<typename T>
bool Circle<T>::intersects(const Circle<T> &other) const
{
    return Helper_vec2length(center - other.center) < (radius + other.radius);
}

template<typename T>
bool Circle<T>::intersects(const sf::Rect<T> &rect) const
{
    T closestX = Helper_clamp(center.x, rect.left, rect.left + rect.width);
    T closestY = Helper_clamp(center.y, rect.top, rect.top + rect.height);
    auto closest = sf::Vector2<T>(closestX, closestY);
    T distance = Helper_vec2length(center - closest);

    return distance == 0 || distance < radius;
}

template<typename T>
bool Circle<T>::contains(const Circle<T> &other) const
{
    return (Helper_vec2length(center - other.center) + other.radius) <= radius;
}

template<typename T>
bool Circle<T>::contains(const sf::Rect<T> &rect) const
{
    T furthestX = std::max(std::abs(rect.left - center.x), std::abs(rect.left + rect.width - center.x));
    T furthestY = std::max(std::abs(rect.top - center.y), std::abs(rect.top + rect.height - center.y));

    return Helper_vec2length(sf::Vector2<T>(furthestX, furthestY)) <= radius;
}

template<typename T>
bool Circle<T>::inside(const sf::Rect<T> &rect) const
{
    T distanceX = std::min(std::abs(rect.left - center.x), std::abs(rect.left - center.x + rect.width));
    T distanceY = std::min(std::abs(rect.top - center.y), std::abs(rect.top - center.y + rect.height));

    return rect.contains(center) && distanceX >= radius && distanceY >= radius;
}

template<typename T>
bool Circle<T>::contains(const sf::Vector2<T> &point) const
{
    return Helper_vec2length(point - center) <= radius;
}

template<typename T>
bool Circle<T>::contains(T x, T y) const
{
    return contains(sf::Vector2<T>(x, y));
}

template <typename T>
BoundingBody<T>::BoundingBody() :
    BoundingBody(RotatingRect<T>())
{
    isCircle = false;
}

template <typename T>
BoundingBody<T>::BoundingBody(const Circle<T> &_circle) :
    circle(_circle),
    rect(circle.center.x - _circle.radius, circle.center.y - _circle.radius, _circle.radius * (T) 2, _circle.radius * (T) 2)
{
    isCircle = true;
}

template <typename T>
BoundingBody<T>::BoundingBody(const RotatingRect<T> &_rect) :
    rect(_rect)
{
    isCircle = false;
}

template <typename T>
bool BoundingBody<T>::Intersects(const BoundingBody<T> &other) const
{
    if (isCircle) {
        //CIRCLE-CIRCLE
        if (other.isCircle) {
            return circle.intersects(other.circle);
        } else {
            //CIRCLE-RECT
            return other.rect.intersects(circle);
        }
    } else {
        if (other.isCircle) {
            //RECT-CIRCLE
            return rect.intersects(other.circle);
        } else {
            //RECT-RECT
            return rect.intersects(other.rect);
        }
    }
}

template <typename T>
bool BoundingBody<T>::Contains(const BoundingBody<T> &other) const
{
    if (isCircle) {
        //CIRCLE-CIRCLE
        if (other.isCircle) {
            return circle.contains(other.circle);
        } else {
            //CIRCLE-RECT
            return other.rect.inside(circle);
        }
    } else {
        if (other.isCircle) {
            //RECT-CIRCLE
            return rect.contains(other.circle);
        } else {
            //RECT-RECT
            return rect.contains(other.rect);
        }
    }
}

template <typename T>
bool BoundingBody<T>::Contains(T x, T y) const
{
    return Contains(sf::Vector2<T>(x, y));
}

template <typename T>
bool BoundingBody<T>::Contains(const sf::Vector2<T> &point) const
{
    if (isCircle) {
        return circle.contains(point);
    } else {
        return rect.contains(point);
    }
}