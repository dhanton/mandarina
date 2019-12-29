#include "helper.hpp"

#include <cmath>
#include <string>
#include <cstdio>

namespace dh {

template <typename T>
int sign(T number)
{
    if (number == static_cast<T>(0)) return 0;

    return std::abs(number)/number;
}

template <typename T>
T clamp(T value, T upper, T lower)
{
    if (upper < lower) return std::min(lower, std::max(value, upper));

    return std::min(upper, std::max(value, lower));
}

double lerp(double x0, double x1, double t, double d)
{
    t /= d;
    return (1 - t) * x0 + t * x1;
}

Vector2 lerpVec2(const Vector2& vec0, const Vector2 vec1, double t, double d)
{
    Vector2 result;
    result.x = lerp(vec0.x, vec1.x, t, d);
    result.y = lerp(vec0.y, vec1.y, t, d);
    return result;
}

float hermite(float v0, float v1, float x0, float x1, float t, float d)
{
    t /= d;

    float h1 = 2*(t*t*t) - 3*(t*t) + 1;
    float h2 = -2*(t*t*t) + 3*(t*t);
    float h3 = t*t*t - 2*t*t + t;
    float h4 = t*t*t - t*t;

    return h1*x0 + h2*x1 + h3*v0 + h4*v1;
}

float degToRad(float angle)
{
    return angle/180.0f * PI;
}

float radToDeg(float angle)
{
    return angle * 180.0f/PI;
}

template <typename T>
void rotatePoint(sf::Vector2<T>& point, T angle, const sf::Vector2<T>& center)
{
    sf::Transform transform;
    transform.rotate(angle, center);

    point = transform.transformPoint(point);
}

template <typename T>
Vector2 rectCenter(const sf::Rect<T> &rect)
{
    return {rect.left - rect.width/2, rect.top - rect.height/2};
}

template <typename T>
float length(const sf::Vector2<T> &pos)
{
    return std::sqrt(pos.x * pos.x + pos.y * pos.y);
}

template <typename T>
sf::Vector2<T> unitary(const sf::Vector2<T> &pos)
{
    float length = dh::length(pos);

    if (length == 0) return sf::Vector2<T>();
    return pos/length;
}

template <typename T>
sf::Vector2<T> add(const sf::Vector2<T> &a, const sf::Vector2<T> &b)
{
    return {a.x + b.x, a.y + b.y};
}

template<typename T>
sf::Vector2<T> multiply(const sf::Vector2<T> &a, const sf::Vector2<T> &b)
{
    return {a.x * b.x, a.y * b.y};
}

template <typename T>
T dotProduct(const sf::Vector2<T>& a, const sf::Vector2<T>& b)
{
    return a.x * b.x + a.y * b.y;
}

template<typename T>
sf::Vector2<T> absolute(const sf::Vector2<T> &a)
{
   return {std::abs(a.x), std::abs(a.y)};
}

sf::FloatRect getViewRect(const sf::View &view)
{
    //TODO: Apply zoom and other transformations
    sf::FloatRect rect;

    rect.left = view.getCenter().x - view.getSize().x/2.f;
    rect.top  = view.getCenter().y - view.getSize().y/2.f;

    rect.width  = view.getSize().x;
    rect.height = view.getSize().y;

    return rect;
}

}