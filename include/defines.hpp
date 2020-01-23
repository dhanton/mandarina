#pragma once

#include <cstdint>

#include <SFML/System/Vector2.hpp>

using Vector2 = sf::Vector2f;
using Vector2i = sf::Vector2i;
using Vector2u = sf::Vector2u;

using u8 = uint8_t;
using s8 = int8_t;

using u16 = uint16_t;
using s16 = int16_t;

using u32 = uint32_t;
using s32 = int32_t;

// using u64 = uint64_t;
// using s64 = int64_t;

//@TODO: Modify CRCPacket to use the type above
using u64 = unsigned long long;
using s64 = signed long long;

#define PI 3.14159265358979323846
#define SQRT2 1.41421356237309504880
#define SQRT2_INV 0.7071067811865475

//Possible constants that might be useful in the future
//#define InvPi   0.31830988618379067154
//#define Inv2Pi  0.15915494309189533577
//#define Inv4Pi  0.07957747154594766788
//#define PiOver2 1.57079632679489661923
//#define PiOver4 0.78539816339744830961
