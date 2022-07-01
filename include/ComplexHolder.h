#ifndef COMPLEXHOLDER_H
#define COMPLEXHOLDER_H

#include <SFML/Graphics.hpp>

constexpr double LIMIT         = 100.0;
constexpr double UPPER_BORDER  = 1.3;
constexpr size_t MAX_ITERATION = 500;
constexpr double ZOOMING_RATIO = 0.33;

struct Frame final
{
    unsigned int x1   = 0;
    unsigned int x2   = 0;
    unsigned int y1   = 0;
    unsigned int y2   = 0;
    double       zoom = 0;
};

typedef sf::Vector2<double> point_t;

struct ComplexHolder final
{
    explicit ComplexHolder(sf::Vector2u winsizes_);

    void    updateWinSizes(size_t new_width, size_t new_height);
    void    zoom(double wheel_delta, point_t point);
    point_t Screen2Plane(sf::Vector2i pixel) const;
    point_t Plane2Screen(point_t point) const;
    void    changeBorders(Frame frame);
    void    reset();

    struct Borders final
    {
        double re_left   = -1;
        double re_right  = 1;
        double im_bottom = -1;
        double im_top    = 1;

        Borders() = default;
        Borders(double re_left_, double re_right_, double im_bottom_, double im_top_);
    } borders;

    size_t       itrn_max = MAX_ITERATION;
    double       limit    = LIMIT;
    point_t      julia_point;
    sf::Vector2u winsizes;
};

#endif // COMPLEXHOLDER_H