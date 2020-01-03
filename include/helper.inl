#include "helper.hpp"

#include <cmath>
#include <algorithm>

template<typename T>
float Helper_vec2length(const sf::Vector2<T>& vec)
{
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

template<typename T>
T Helper_clamp(T value, T upper, T lower)
{
    if (upper < lower) return std::min(lower, std::max(value, upper));

    return std::min(upper, std::max(value, lower));
}
