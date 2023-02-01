#include "UI/Vidget.h"
#include "Utils.h"

Vidget::Vidget(const vec2d& position)
{
    box_.setPosition(vec(vec2d(DIP2Pixels(position.x), DIP2Pixels(position.y))));
}

vec2d Vidget::getPosition() const
{
    return vec2d(Pixels2DIP(box_.getPosition().x), Pixels2DIP(box_.getPosition().y));
}

vec2d Vidget::getSize() const
{
    return vec2d(Pixels2DIP(box_.getSize().x), Pixels2DIP(box_.getSize().y));
}

void Vidget::setPosition(const vec2d& position)
{
    box_.setPosition(vec(vec2d(DIP2Pixels(position.x), DIP2Pixels(position.y))));
    update();
}

void Vidget::hide()
{
    is_visible = false;
}

void Vidget::show()
{
    is_visible = true;
}