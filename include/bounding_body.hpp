#pragma once

#include <SFML/Graphics/Rect.hpp>

#include "defines.hpp"

template <typename T>
struct Circle {
    Circle();
    Circle(T _x, T _y, T _radius);
    Circle(const sf::Vector2<T>& _center, T _radius);

    bool intersects(const Circle<T>& other) const;
    bool intersects(const sf::Rect<T>& rect) const;
    bool contains(const Circle<T>& other) const;
    bool contains(const sf::Rect<T>& rect) const;
    bool inside(const sf::Rect<T>& rect) const;

    bool contains(const sf::Vector2<T>& point) const;
    bool contains(T x, T y) const;

    sf::Vector2<T> center;
    T radius;
};

//Rotating with respect to getLocalOrigin, which is (width/2, height/2)
template <typename T>
struct RotatingRect {
    RotatingRect();
    RotatingRect(const sf::Rect<T>& rect, T _angle = 0);
    RotatingRect(T _left, T _top, T _width, T _height, T _angle = 0);
    RotatingRect(const sf::Vector2<T>& pos, const sf::Vector2<T>& size, T _angle = 0);

    bool intersects(const RotatingRect<T>& other) const;
    bool intersects(const Circle<T>& circle) const;
    bool contains(const RotatingRect<T>& other) const;
    bool contains(const Circle<T>& circle) const;
    bool inside(const Circle<T>& circle) const;

    bool contains(const sf::Vector2<T>& point) const;
    bool contains(T x, T y) const;

    sf::Vector2<T> getCenter() const;
    sf::Vector2<T> getLocalOrigin() const;
    sf::Rect<T> getNonRotatingRect() const;

    //Non rotating global bounds
    sf::Rect<T> getGlobalBounds() const;

    T left;
    T top;
    T width;
    T height;
    T angle;
};

template <typename T>
struct BoundingBody {
    BoundingBody();
    BoundingBody(const Circle<T>& circle);
    BoundingBody(const RotatingRect<T>& rect);

    bool Intersects(const BoundingBody<T>& other) const;
    bool Contains(const BoundingBody<T>& other) const;
    bool Contains(T x, T y) const;
    bool Contains(const sf::Vector2<T>& point) const;

    bool isCircle;

    Circle<T> circle;
    RotatingRect<T> rect; //Used internally by the quadtree even if it's a circle
};

using Circlef = Circle<float>;
using RotatingRectf = RotatingRect<float>;
using BoundingBodyf = BoundingBody<float>;

#include "bounding_body.inl"
