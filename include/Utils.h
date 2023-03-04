#ifndef UTILS_H
#define UTILS_H

#include "vec2.h"
#include <SFML/Graphics.hpp>

template <typename TYPE>
vec2f vec(sf::Vector2<TYPE> v)
{
    return vec2f(static_cast<float>(v.x), static_cast<float>(v.y));
}

template <typename TYPE>
sf::Vector2f vec(vec2<TYPE> v)
{
    return sf::Vector2f(static_cast<float>(v.x), static_cast<float>(v.y));
}

#ifdef _WIN32
#include <Windows.h>

inline float DIP2Pixels(float dip)
{
    return GetDpiForSystem() / 96.0F * dip;
}

inline float Pixels2DIP(float pixels)
{
    return 96.0F / GetDpiForSystem() * pixels;
}

#else

inline float DIP2Pixels(float dip)
{
    return 115.0F / 96.0F * dip;
}

inline float Pixels2DIP(float pixels)
{
    return 96.0F / 115.0F * pixels;
}

#endif // _WIN32

#endif // UTILS_H