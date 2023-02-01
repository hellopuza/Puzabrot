#include "UI/Label.h"
#include "Utils.h"

constexpr double OFFSET_FACTOR = 1.15;

#define FMUL(a, b) static_cast<float>(a) * static_cast<float>(b)
#define EXPAND(a) ((a) + (offset_))

Label::Label(const sf::Font& font, double font_size, const vec2d& position) :
    Vidget(position), font_size_(DIP2Pixels(font_size), FMUL(DIP2Pixels(font_size), OFFSET_FACTOR))
{
    offset_ = FMUL(OFFSET_FACTOR, font_size_.y) - font_size_.y;

    text_.setFont(font);
    text_.setCharacterSize(DIP2Pixels(font_size));

    text_.setFillColor(sf::Color::White);
    text_.setOutlineThickness(1.0);
    text_.setOutlineColor(sf::Color::Black);
    update();
}

void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (is_visible)
    {
        if (text_.getString().getSize() != 0)
        {
            target.draw(text_, states);
        }
    }
}

bool Label::handleEvent(const sf::Event& event)
{
    return false;
}

void Label::setText(const std::string& text)
{
    text_.setString(text);
    update();
}

void Label::update()
{
    text_.setPosition(box_.getPosition());
    box_.setSize(vec((text_.getString().getSize() == 0) ? vec2f() : vec2f(text_.getLocalBounds().width, text_.getLocalBounds().height)));
}