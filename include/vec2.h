#ifndef VEC2_H
#define VEC2_H

#include <cmath>
#include <functional>
#include <cstdint>

template <typename T>
struct vec2 final
{
    T x = T{};
    T y = T{};

    vec2() {}

    vec2(T x_, T y_) : x(x_), y(y_) {}

    vec2 xx() const
    {
        return vec2(x, x);
    }

    vec2 xy() const
    {
        return vec2(x, y);
    }

    vec2 yx() const
    {
        return vec2(y, x);
    }

    vec2 yy() const
    {
        return vec2(y, y);
    }

    vec2 operator + (const vec2& v) const
    {
        return vec2(x + v.x,
                    y + v.y);
    }

    vec2 operator - (const vec2& v) const
    {
        return vec2(x - v.x,
                    y - v.y);
    }

    vec2 operator * (const vec2& v) const
    {
        return vec2(x * v.x,
                    y * v.y);
    }

    vec2 operator / (const vec2& v) const
    {
        return vec2(x / v.x,
                    y / v.y);
    }

    vec2 operator * (T k) const
    {
        return vec2(k * x,
                    k * y);
    }

    vec2 operator / (T k) const
    {
        return vec2(x / k,
                    y / k);
    }

    vec2& operator += (const vec2& v)
    {
        return *this = *this + v;
    }

    vec2& operator -= (const vec2& v)
    {
        return *this = *this - v;
    }

    vec2& operator *= (const vec2& v)
    {
        return *this = *this * v;
    }

    vec2& operator /= (const vec2& v)
    {
        return *this = *this / v;
    }

    vec2& operator *= (T k)
    {
        return *this = *this * k;
    }

    vec2& operator /= (T k)
    {
        return *this = *this / k;
    }

    vec2 operator - () const
    {
        return vec2(-x, -y);
    }

    bool operator == (const vec2& v) const
    {
        return (x == v.x) && (y == v.y);
    }

    bool operator != (const vec2& v) const
    {
        return (x != v.x) || (y != v.y);
    }

    template <typename TYPE>
    operator vec2<TYPE>() const
    {
        return vec2<TYPE>(static_cast<TYPE>(x), static_cast<TYPE>(y));
    }

    T magnitude2() const
    {
        return x * x + y * y;
    }

    T magnitude() const
    {
        return std::sqrt(magnitude2());
    }

    vec2& normalize()
    {
        if ((x == 0) && (y == 0))
        {
            return *this;
        }
        return *this /= magnitude();
    }

    vec2 normalized() const
    {
        if ((x == 0) && (y == 0))
        {
            return *this;
        }
        return *this / magnitude();
    }
};

template <typename T>
inline vec2<T> operator * (T k, const vec2<T>& v)
{
    return vec2(k * v.x,
                k * v.y);
}

template <typename T>
inline vec2<T> operator / (T k, const vec2<T>& v)
{
    return vec2(k / v.x,
                k / v.y);
}

template <typename T>
inline T dot (const vec2<T>& u, const vec2<T>& v)
{
    return u.x * v.x + u.y * v.y;
}

template <typename T>
inline vec2<T> sign(const vec2<T>& v)
{
    return vec2<T>((v.x > 0) ? +1 : (v.x < 0) ? -1 : 0,
                   (v.y > 0) ? +1 : (v.y < 0) ? -1 : 0);
}

template <typename T>
inline vec2<T> round(const vec2<T>& v)
{
    return vec2<T>(round(v.x), round(v.y));
}

template <typename T>
inline vec2<T> abs(const vec2<T>& v)
{
    return vec2<T>(std::abs(v.x), std::abs(v.y));
}

template <typename T>
inline vec2<T> floor(const vec2<T>& v)
{
    return vec2<T>(std::floor(v.x), std::floor(v.y));
}

template <typename T>
inline vec2<T> ceil(const vec2<T>& v)
{
    return vec2<T>(std::ceil(v.x), std::ceil(v.y));
}

template <typename T>
inline vec2<T> clamp(const vec2<T>& v, const vec2<T>& min, const vec2<T>& max)
{
    return vec2<T>((v.x < min.x) ? min.x : (v.x > max.x) ? max.x : v.x,
                   (v.y < min.y) ? min.y : (v.y > max.y) ? max.y : v.y);
}

template <typename T>
inline vec2<T> step(const vec2<T>& edge, const vec2<T>& v)
{
    return vec2<T>(v.x < edge.x ? 0 : 1,
                   v.y < edge.y ? 0 : 1);
}

template <typename T>
inline vec2<T> min(const vec2<T>& v1, const vec2<T>& v2)
{
    return vec2<T>(v1.x < v2.x ? v1.x : v2.x,
                   v1.y < v2.y ? v1.y : v2.y);
}

template <typename T>
inline vec2<T> max(const vec2<T>& v1, const vec2<T>& v2)
{
    return vec2<T>(v1.x > v2.x ? v1.x : v2.x,
                   v1.y > v2.y ? v1.y : v2.y);
}

typedef vec2<int32_t>  vec2i;
typedef vec2<uint32_t> vec2u;
typedef vec2<int64_t>  vec2li;
typedef vec2<uint64_t> vec2lu;
typedef vec2<float>    vec2f;
typedef vec2<double>   vec2d;

namespace std {

template <typename T>
struct hash<vec2<T>>
{
    size_t operator()(const vec2<T>& key) const
    {
        size_t x = hash<T>()(key.x);
        size_t y = hash<T>()(key.y);
        return (x * y) + (x ^ y);
    }
};

} // namespace std

#endif // VEC2_H