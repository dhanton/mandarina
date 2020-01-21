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
