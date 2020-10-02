#pragma once

#include <string>
#include <random>

#include "defines.hpp"

double  Helper_lerp(double x0, double x1, double t, double d);
double  Helper_lerpAngle(double x0, double x1, double t, double d);
Vector2 Helper_lerpVec2(const Vector2& vec0, const Vector2& vec1, double t, double d);

//returns quadrant as int (1, 2, 3, 4)
int Helper_angleQuadrant(float angle);

float Helper_radToDeg(float rad);
float Helper_degToRad(float rad);

//efficient methods for packing and unpacking angles into 16 bits (with some error)
u16   Helper_angleTo16bit(float angle);
float Helper_angleFrom16bit(u16 angle);

u16   Helper_percentageTo16bit(float percentage);
float Helper_percentageFrom16bit(u16 percentage);

template<typename T>
float Helper_vec2length(const sf::Vector2<T>& vec);

template<typename T>
sf::Vector2<T> Helper_vec2unitary(const sf::Vector2<T>& vec);

template<typename T>
std::string Helper_vec2string(const sf::Vector2<T>& vec);

template<typename T>
bool Helper_pointsCollinear(const sf::Vector2<T>& a, const sf::Vector2<T>& b, const sf::Vector2<T>& c);

template<typename T>
T Helper_clamp(T value, T upper, T lower);

//angle in degrees
template <typename T> 
void Helper_rotatePoint(sf::Vector2<T>& point, T angle, const sf::Vector2<T>& center = {(T) 0, (T) 0});

class Helper_Random
{
public:
    Helper_Random() = delete;

    static std::mt19937& gen();
    
    static double rndAngleDegrees();
    static double rndAngleRadians();

    static int coinFlip();

private:
    static std::random_device m_rd;
    static std::mt19937 m_gen;

    static std::uniform_real_distribution<double> m_angleDistr;
    static std::uniform_int_distribution<int> m_coinFlipDistr;
};

#include "helper.inl"
