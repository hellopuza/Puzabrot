#include "ComplexHolder.h"

const float INITIAL_LEFT_RATIO = 3.0F / 5.0F;
const float INITIAL_RIGHT_RATIO = 2.0F / 5.0F;

ComplexHolder::Borders::Borders(double re_left_, double re_right_, double im_bottom_, double im_top_) :
    re_left(re_left_), re_right(re_right_), im_bottom(im_bottom_), im_top(im_top_)
{}

ComplexHolder::ComplexHolder(sf::Vector2u winsizes_) : winsizes(winsizes_)
{
    borders.im_top    = UPPER_BORDER;
    borders.im_bottom = -UPPER_BORDER;

    borders.re_left  = -(borders.im_top - borders.im_bottom) * winsizes.x / winsizes.y * INITIAL_LEFT_RATIO;
    borders.re_right =  (borders.im_top - borders.im_bottom) * winsizes.x / winsizes.y * INITIAL_RIGHT_RATIO;
}

point_t ComplexHolder::Screen2Plane(sf::Vector2i pixel) const
{
    return {borders.re_left + (borders.re_right - borders.re_left) * pixel.x / winsizes.x,
            borders.im_top  - (borders.im_top - borders.im_bottom) * pixel.y / winsizes.y};
}

point_t ComplexHolder::Plane2Screen(point_t point) const
{
    return {(point.x - borders.re_left) / (borders.re_right - borders.re_left) * winsizes.x,
            (borders.im_top - point.y)  / (borders.im_top - borders.im_bottom) * winsizes.y};
}

void ComplexHolder::zoom(double wheel_delta, point_t point)
{
    double width  = borders.re_right - borders.re_left;
    double height = borders.im_top - borders.im_bottom;

    double x_ratio = (point.x - borders.re_left)   / width;
    double y_ratio = (point.y - borders.im_bottom) / height;

    Borders new_frame = {
        borders.re_left   +      x_ratio  * ZOOMING_RATIO * width * wheel_delta,
        borders.re_right  - (1 - x_ratio) * ZOOMING_RATIO * width * wheel_delta,
        borders.im_bottom +      y_ratio  * ZOOMING_RATIO * height * wheel_delta,
        borders.im_top    - (1 - y_ratio) * ZOOMING_RATIO * height * wheel_delta,
    };

    borders = new_frame;
}

void ComplexHolder::reset()
{
    borders.im_top    = UPPER_BORDER;
    borders.im_bottom = -UPPER_BORDER;

    borders.re_left  = -(borders.im_top - borders.im_bottom) * winsizes.x / winsizes.y * INITIAL_LEFT_RATIO;
    borders.re_right =  (borders.im_top - borders.im_bottom) * winsizes.x / winsizes.y * INITIAL_RIGHT_RATIO;

    itrn_max = MAX_ITERATION;
    limit    = LIMIT;
}

void ComplexHolder::updateWinSizes(size_t new_width, size_t new_height)
{
    winsizes.x = static_cast<unsigned int>(new_width);
    winsizes.y = static_cast<unsigned int>(new_height);

    borders.re_right = borders.re_left + (borders.im_top - borders.im_bottom) * static_cast<float>(winsizes.x) / static_cast<float>(winsizes.y);
}

void ComplexHolder::changeBorders(Frame frame)
{
    double releft  = borders.re_left;
    double reright = borders.re_right;
    double imup    = borders.im_top;
    double imdown  = borders.im_bottom;

    if (frame.zoom > 1)
    {
        borders.re_left   = releft + (reright - releft) * frame.x1 / winsizes.x;
        borders.re_right  = releft + (reright - releft) * frame.x2 / winsizes.x;
        borders.im_bottom = imup - (imup - imdown) * frame.y2 / winsizes.y;

        borders.im_top = borders.im_bottom + (borders.re_right - borders.re_left) * winsizes.y / winsizes.x;
    }
    else
    {
        borders.re_left   = releft - (reright - releft) * frame.x1 / (frame.x2 - frame.x1);
        borders.re_right  = reright + (reright - releft) * (winsizes.x - frame.x2) / (frame.x2 - frame.x1);
        borders.im_bottom = imdown - (imup - imdown) * (winsizes.y - frame.y2) / (frame.y2 - frame.y1);

        borders.im_top = borders.im_bottom + (borders.re_right - borders.re_left) * winsizes.y / winsizes.x;
    }
}