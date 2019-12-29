#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>

#include "defines.hpp"

namespace dh {

template <typename T> inline int sign(T number);
template <typename T> inline T clamp(T value, T upper, T lower);

inline double lerp(double x0, double x1, double t, double d);
inline Vector2 lerpVec2(const Vector2& vec0, const Vector2 vec1, double t, double d);
inline float hermite(float v0, float v1, float x0, float x1, float t, float d);

inline float degToRad(float angle);
inline float radToDeg(float angle);

//angle in degrees
template <typename T> inline void rotatePoint(sf::Vector2<T>& point, T angle, const sf::Vector2<T>& center = {(T) 0, (T) 0});

template <typename T> inline Vector2 rectCenter(const sf::Rect<T>& rect);
template <typename T> inline float length(const sf::Vector2<T>& pos);
template <typename T> inline sf::Vector2<T> unitary(const sf::Vector2<T>& pos);

template <typename T> inline sf::Vector2<T> add(const sf::Vector2<T>& a, const sf::Vector2<T>& b);
template <typename T> inline sf::Vector2<T> multiply(const sf::Vector2<T>& a, const sf::Vector2<T> & b);
template <typename T> inline T dotProduct(const sf::Vector2<T>& a, const sf::Vector2<T>& b);
template<typename T> inline sf::Vector2<T> absolute(const sf::Vector2<T>& a);

//It doesn't compile if non-template functions are not inline
inline sf::FloatRect getViewRect(const sf::View& view);
}

#include "helper.inl"
