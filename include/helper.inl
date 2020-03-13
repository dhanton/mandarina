#include "helper.hpp"

#include <cmath>
#include <algorithm>
#include <SFML/Graphics/Transform.hpp>

template<typename T>
float Helper_vec2length(const sf::Vector2<T>& vec)
{
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

template<typename T>
sf::Vector2<T> Helper_vec2unitary(const sf::Vector2<T>& vec)
{
    float length = Helper_vec2length(vec);

    if (length == 0.f) return sf::Vector2<T>();

    return vec/length;
}

template<typename T>
std::string Helper_vec2string(const sf::Vector2<T>& vec)
{
    return std::to_string(vec.x) + ' ' + std::to_string(vec.y);
}

template<typename T>
bool Helper_pointsCollinear(const sf::Vector2<T>& a, const sf::Vector2<T>& b, const sf::Vector2<T>& c)
{
    //true if the area of the triangle the points form is 0
    return (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y) == static_cast<T>(0));
}

template<typename T>
T Helper_clamp(T value, T upper, T lower)
{
    if (upper < lower) return std::min(lower, std::max(value, upper));

    return std::min(upper, std::max(value, lower));
}

template <typename T> 
void Helper_rotatePoint(sf::Vector2<T>& point, T angle, const sf::Vector2<T>& center)
{
    sf::Transform transform;
    transform.rotate(angle, center);

    point = transform.transformPoint(point);
}
