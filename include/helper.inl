#include "helper.hpp"

#include <cmath>
#include <numeric>
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

std::array<uint_fast32_t, 256> generate_crc_lookup_table() noexcept
{
    auto const reversed_polynomial = std::uint_fast32_t{0xEDB88320uL};

    struct byte_checksum
    {
        std::uint_fast32_t operator()() noexcept
        {
            auto checksum = static_cast<std::uint_fast32_t>(n++);

            for (auto i = 0; i < 8; ++i)
                checksum = (checksum >> 1) ^ ((checksum & 0x1u) ? reversed_polynomial : 0);

            return checksum;
        }

        unsigned n = 0;
    };

    auto table = std::array<std::uint_fast32_t, 256>{};
    std::generate(table.begin(), table.end(), byte_checksum{});

    return table;
}

template<typename InputIterator>
uint_fast32_t crc(InputIterator first, InputIterator last)
{
    //Generate table only the first time this is called
    static auto const table = generate_crc_lookup_table();

    return std::uint_fast32_t{0xFFFFFFFFuL} &
        ~std::accumulate(first, last,
          ~std::uint_fast32_t{0} & std::uint_fast32_t{0xFFFFFFFFuL},
            [](std::uint_fast32_t checksum, std::uint_fast8_t value)
              { return table[(checksum ^ value) & 0xFFu] ^ (checksum >> 8); });
}

}