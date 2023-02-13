#ifndef UTILS_H
#define UTILS_H

#include "vec2.h"
#include <SFML/Graphics.hpp>

template <typename TYPE>
vec2d vec(sf::Vector2<TYPE> v)
{
    return vec2d(v.x, v.y);
}

template <typename TYPE>
sf::Vector2f vec(vec2<TYPE> v)
{
    return sf::Vector2f(v.x, v.y);
}

#ifdef _WIN32
#include <Windows.h>

static double DIP2Pixels(double dip)
{
    return GetDpiForSystem() / 96.0 * dip;
}

static double Pixels2DIP(double pixels)
{
    return 96.0 / GetDpiForSystem() * pixels;
}

#endif // _WIN32

#endif // UTILS_H