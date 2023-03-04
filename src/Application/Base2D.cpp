#include "Application/Base2D.h"

Base2D::Borders::Borders(float left_, float right_, float bottom_, float top_) :
    left(left_), right(right_), bottom(bottom_), top(top_)
{}

Base2D::Base2D(const vec2u& size) : size_(size) {}

void Base2D::setBaseSize(const vec2u& size)
{
    size_ = size;
    float size_ratio = static_cast<float>(size_.x) / static_cast<float>(size_.y);

    borders_.right = borders_.left + (borders_.top - borders_.bottom) * size_ratio;
}

void Base2D::setBorders(float bottom_, float top_, float right_left_ratio)
{
    float size_ratio = static_cast<float>(size_.x) / static_cast<float>(size_.y);

    borders_.bottom = bottom_;
    borders_.top = top_;

    borders_.left = -(borders_.top - borders_.bottom) * size_ratio * 1.0F / (1.0F + right_left_ratio);
    borders_.right = (borders_.top - borders_.bottom) * size_ratio * right_left_ratio / (1.0F + right_left_ratio);
}

Base2D::Borders Base2D::getBorders() const
{
    return borders_;
}

vec2f Base2D::Screen2Base(const vec2i& pixel) const
{
    return {
        borders_.left + (borders_.right - borders_.left) * static_cast<float>(pixel.x) / static_cast<float>(size_.x),
        borders_.top - (borders_.top - borders_.bottom) * static_cast<float>(pixel.y) / static_cast<float>(size_.y)
    };
}

vec2i Base2D::Base2Screen(const vec2f& point) const
{
    return {
        static_cast<int>((point.x - borders_.left) / (borders_.right - borders_.left) * static_cast<float>(size_.x)),
        static_cast<int>((borders_.top - point.y) / (borders_.top - borders_.bottom) * static_cast<float>(size_.y))
    };
}