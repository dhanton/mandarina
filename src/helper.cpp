#include "helper.hpp"

#include <cmath>

double Helper_lerp(double x0, double x1, double t, double d)
{
    t /= d;
    return (1 - t) * x0 + t * x1;
}

double Helper_lerpAngle(double x0, double x1, double t, double d)
{
    int quadrant0 = Helper_angleQuadrant(x0);
    int quadrant1 = Helper_angleQuadrant(x1);

    if (quadrant0 == 4 && quadrant1 == 1) {
        x1 += 360.f;
    }

    if (quadrant0 == 1 && quadrant1 == 4) {
        x0 += 360.f;
    }

    return Helper_lerp(x0, x1, t, d);
}

Vector2 Helper_lerpVec2(const Vector2& vec0, const Vector2& vec1, double t, double d)
{
    Vector2 result;
    result.x = Helper_lerp(vec0.x, vec1.x, t, d);
    result.y = Helper_lerp(vec0.y, vec1.y, t, d);
    return result;
}

int Helper_angleQuadrant(float angle)
{
    angle = std::fmod(angle, 360.f);

    if (angle < 0.f) {
        angle += 360.f;
    }

    if (angle >= 0.f && angle < 90.f) {
        return 1;
    }

    if (angle >= 90.f && angle < 180.f) {
        return 2;
    }

    if (angle >= 180.f && angle < 270.f) {
        return 3;
    }

    return 4;
}

float Helper_degToRad(float angle)
{
    return angle/180.0f * PI;
}

float Helper_radToDeg(float angle)
{
    return angle * 180.0f/PI;
}

//[0, 360] => [0, 0xffff]
u16 Helper_angleTo16bit(float angle)
{
    return std::floor((std::fmod(angle, 360.f))/360.f * 0xffff);
}

//[0, 0xffff] => [0, 360]
float Helper_angleFrom16bit(u16 angle)
{
    return (static_cast<float>(angle)/static_cast<float>(0xffff)) * 360.f;
}

u16 Helper_percentageTo16bit(float percentage)
{
    return std::floor(percentage * static_cast<float>(0xffff));
}

float Helper_percentageFrom16bit(u16 percentage)
{
    return (static_cast<float>(percentage)/static_cast<float>(0xffff));
}
