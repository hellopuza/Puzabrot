#include "UI/SwitchButton.h"
#include "Utils.h"

constexpr float OFFSET_FACTOR = 1.15F;

#define FMUL(a, b) (static_cast<float>(a) * static_cast<float>(b))
#define EXPAND(a) ((a) + (2.0F * offset_))

SwitchButton::SwitchButton(const sf::Font& font, float font_size, const vec2f& position) :
    Vidget(position), font_size_(FMUL(DIP2Pixels(font_size), OFFSET_FACTOR))
{
    offset_ = FMUL(OFFSET_FACTOR, font_size_) - font_size_;

    text_.setFont(font);
    text_.setCharacterSize(static_cast<unsigned>(DIP2Pixels(font_size)));
    text_.setFillColor({ 255, 255, 255, 255 });
    box_.setFillColor({ 128, 128, 128, 128 });
    update();
}

void SwitchButton::draw(sf::RenderTarget& target, sf::RenderStates states) const
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

bool SwitchButton::handleEvent(const sf::Event& event)
{
    // Switch button
    if (is_visible && (event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
    {
        sf::Vector2f mouse_button(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if ((box_.getPosition().x < mouse_button.x) && (mouse_button.x < box_.getPosition().x + box_.getSize().x) &&
            (box_.getPosition().y < mouse_button.y) && (mouse_button.y < box_.getPosition().y + box_.getSize().y))
        {
            value_ = (value_ + 1) % texts_.size();
            text_.setString(texts_[value_]);
            update();
            return true;
        }
    }

    return false;
}

void SwitchButton::addText(const std::string& text)
{
    texts_.emplace_back(text);
    text_.setString(texts_[value_]);
    update();
}

void SwitchButton::setButtonColor(const sf::Color& color)
{
    box_.setFillColor(color);
}

void SwitchButton::setTextColor(const sf::Color& color)
{
    text_.setFillColor(color);
}

size_t SwitchButton::value() const
{
    return value_;
}

void SwitchButton::update()
{
    float text_size = text_.getLocalBounds().width;

    box_.setSize(sf::Vector2f(EXPAND(text_size), EXPAND(font_size_)));
    text_.setPosition(sf::Vector2f(box_.getPosition() + sf::Vector2f(offset_, offset_)));
}