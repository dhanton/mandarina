#pragma once

#include "defines.hpp"

double  Helper_lerp(double x0, double x1, double t, double d);
double Helper_lerpAngle(double x0, double x1, double t, double d);
Vector2 Helper_lerpVec2(const Vector2& vec0, const Vector2& vec1, double t, double d);

//returns quadrant as int (1, 2, 3, 4)
int Helper_angleQuadrant(float angle);

float Helper_radToDeg(float rad);
float Helper_degToRad(float rad);

//efficient methods for packing and unpacking angles into 16 bits (with some error)
u16   Helper_angleTo16bit(float angle);
float Helper_angleFrom16bit(u16 angle);

template<typename T>
float Helper_vec2length(const sf::Vector2<T>& vec);

template<typename T>
sf::Vector2<T> Helper_vec2unitary(const sf::Vector2<T>& vec);

template<typename T>
T Helper_clamp(T value, T upper, T lower);

//angle in degrees
template <typename T> 
void Helper_rotatePoint(sf::Vector2<T>& point, T angle, const sf::Vector2<T>& center = {(T) 0, (T) 0});

#include "helper.inl"
