#include "UI/Button.h"
#include "Utils.h"

constexpr double OFFSET_FACTOR = 1.15;

#define FMUL(a, b) static_cast<float>(a) * static_cast<float>(b)
#define EXPAND(a) ((a) + (2.0 * offset_))

Button::Button(const sf::Font& font, double font_size, const vec2d& position) :
    Vidget(position), font_size_(FMUL(DIP2Pixels(font_size), OFFSET_FACTOR))
{
    offset_ = FMUL(OFFSET_FACTOR, font_size_) - font_size_;

    text_.setFont(font);
    text_.setCharacterSize(DIP2Pixels(font_size));
    update();
    setColors();
}

Button::Button(const vec2d& position, const vec2d& size) :
    Vidget(position), offset_(0)
{
    box_.setSize(vec(vec2d(DIP2Pixels(size.x), DIP2Pixels(size.y))));
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (is_visible)
    {
        target.draw(box_, states);

        if (text_.getString().getSize() != 0)
        {
            target.draw(text_, states);
        }
    }
}

bool Button::handleEvent(const sf::Event& event)
{
    // Press button
    if (is_visible && (event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
    {
        sf::Vector2f mouse_button(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if ((box_.getPosition().x < mouse_button.x) && (mouse_button.x < box_.getPosition().x + box_.getSize().x) &&
            (box_.getPosition().y < mouse_button.y) && (mouse_button.y < box_.getPosition().y + box_.getSize().y))
        {
            pressed_ = !pressed_;
            setColors();

            return true;
        }
    }

    return false;
}

void Button::setText(const std::string& text)
{
    text_.setString(text);
    update();
}

void Button::setFont(const sf::Font& font, double font_size)
{
    font_size_ = FMUL(DIP2Pixels(font_size), OFFSET_FACTOR);
    offset_ = FMUL(OFFSET_FACTOR, font_size_) - font_size_;

    text_.setFont(font);
    text_.setCharacterSize(DIP2Pixels(font_size));
    update();
}

void Button::setSize(const vec2d& size)
{
    box_.setSize(vec(vec2d(DIP2Pixels(size.x), DIP2Pixels(size.y))));
}

void Button::setButtonColors(const sf::Color& off_color, const sf::Color& on_color)
{
    button_off_color_ = off_color;
    button_on_color_ = on_color;
    setColors();
}

void Button::setTextColors(const sf::Color& off_color, const sf::Color& on_color)
{
    text_off_color_ = off_color;
    text_on_color_ = on_color;
    setColors();
}

bool Button::pressed() const
{
    return pressed_;
}

void Button::update()
{
    if (text_.getString().getSize() != 0)
    {
        float text_size = text_.getLocalBounds().width;

        box_.setSize(sf::Vector2f(EXPAND(text_size), EXPAND(font_size_)));
        text_.setPosition(sf::Vector2f(box_.getPosition() + sf::Vector2f(offset_, offset_)));
    }
}

void Button::setColors()
{
    box_.setFillColor(pressed_ ? button_on_color_ : button_off_color_);
    text_.setFillColor(pressed_ ? text_on_color_ : text_off_color_);
}