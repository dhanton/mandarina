#include "helper.hpp"

#include <cmath>

double Helper_lerp(double x0, double x1, double t, double d)
{
    t /= d;
    return (1 - t) * x0 + t * x1;
}

Vector2 Helper_lerpVec2(const Vector2& vec0, const Vector2& vec1, double t, double d)
{
    Vector2 result;
    result.x = Helper_lerp(vec0.x, vec1.x, t, d);
    result.y = Helper_lerp(vec0.y, vec1.y, t, d);
    return result;
}

//[0, 360] => [0, 0xffff]
u16 Helper_angleTo16bit(float angle)
{
    return std::floor((std::fmod(angle, 360))/360.f) * 0xffff;
}

//[0, 0xffff] => [0, 360]
float Helper_angleFrom16bit(u16 angle)
{
    return (static_cast<float>(angle)/static_cast<float>(0xffff)) * 360.f;
}

