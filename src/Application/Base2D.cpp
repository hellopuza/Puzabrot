#include "Application/Base2D.h"

Base2D::Borders::Borders(double left_, double right_, double bottom_, double top_) :
    left(left_), right(right_), bottom(bottom_), top(top_)
{}

Base2D::Base2D(const vec2i& size) : size_(size) {}

void Base2D::setBaseSize(const vec2i& size)
{
    size_ = size;

    borders_.right = borders_.left + (borders_.top - borders_.bottom) * size_.x / size_.y;
}

void Base2D::setBorders(double bottom_, double top_, double right_left_ratio)
{
    borders_.bottom = bottom_;
    borders_.top = top_;

    borders_.left = -(borders_.top - borders_.bottom) * size_.x / size_.y * 1.0 / (1.0 + right_left_ratio);
    borders_.right = (borders_.top - borders_.bottom) * size_.x / size_.y * right_left_ratio / (1.0 + right_left_ratio);
}

Base2D::Borders Base2D::getBorders() const
{
    return borders_;
}

vec2d Base2D::Screen2Base(const vec2i& pixel) const
{
    return {
        borders_.left + (borders_.right - borders_.left) * pixel.x / size_.x,
        borders_.top - (borders_.top - borders_.bottom) * pixel.y / size_.y
    };
}

vec2i Base2D::Base2Screen(const vec2d& point) const
{
    return {
        static_cast<int>((point.x - borders_.left) / (borders_.right - borders_.left) * size_.x),
        static_cast<int>((borders_.top - point.y) / (borders_.top - borders_.bottom) * size_.y)
    };
}