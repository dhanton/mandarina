#pragma once

#include "defines.hpp"

double  Helper_lerp(double x0, double x1, double t, double d);
Vector2 Helper_lerpVec2(const Vector2& vec0, const Vector2& vec1, double t, double d);

//efficient methods for packing and unpacking angles into 16 bits (with some error)
u16   Helper_angleTo16bit(float angle);
float Helper_angleFrom16bit(u16 angle);

template<typename T>
float Helper_vec2length(const sf::Vector2<T>& vec);
