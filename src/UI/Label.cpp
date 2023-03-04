#include "UI/Label.h"
#include "Utils.h"

Label::Label(const sf::Font& font, float font_size, const vec2f& position) : Vidget(position)
{
    text_.setFont(font);
    text_.setCharacterSize(static_cast<unsigned>(DIP2Pixels(font_size)));

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

bool Label::handleEvent(const sf::Event&)
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