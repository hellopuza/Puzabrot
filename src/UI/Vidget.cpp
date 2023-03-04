#include "UI/Vidget.h"
#include "Utils.h"

Vidget::Vidget(const vec2f& position)
{
    box_.setPosition(vec(vec2f(DIP2Pixels(position.x), DIP2Pixels(position.y))));
}

vec2f Vidget::getPosition() const
{
    return vec2f(Pixels2DIP(box_.getPosition().x), Pixels2DIP(box_.getPosition().y));
}

vec2f Vidget::getSize() const
{
    return vec2f(Pixels2DIP(box_.getSize().x), Pixels2DIP(box_.getSize().y));
}

void Vidget::setPosition(const vec2f& position)
{
    box_.setPosition(vec(vec2f(DIP2Pixels(position.x), DIP2Pixels(position.y))));
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