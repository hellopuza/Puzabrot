#include "UI/InputBox.h"
#include "Utils.h"

const sf::Color WHITE_COLOR = { 255, 255, 255, 255 };
const sf::Color GREY_COLOR = { 128, 128, 128, 128 };
const sf::Color LIGHT_GREY_COLOR = { 200, 200, 200, 255 };
const sf::Color DARK_GREY_COLOR = { 50, 50, 50, 255 };

constexpr int INPUT_UNICODE_MAX = 127;
constexpr int INPUT_UNICODE_MIN = 32;
constexpr double OFFSET_FACTOR = 1.15;
constexpr double MIN_FACTOR = 4.0;

#define FMUL(a, b) static_cast<float>(a) * static_cast<float>(b)
#define EXPAND(a) ((a) + (offset_))

InputBox::InputBox(const sf::Font& font, double font_size, const vec2d& position) :
    Vidget(position), font_size_(DIP2Pixels(font_size), FMUL(DIP2Pixels(font_size), OFFSET_FACTOR))
{
    offset_ = FMUL(OFFSET_FACTOR, font_size_.y) - font_size_.y;

    box_.setFillColor(GREY_COLOR);

    label_text_.setFont(font);
    input_text_.setFont(font);
    output_text_.setFont(font);

    label_text_.setCharacterSize(DIP2Pixels(font_size));
    input_text_.setCharacterSize(DIP2Pixels(font_size));
    output_text_.setCharacterSize(DIP2Pixels(font_size));

    label_text_.setFillColor(WHITE_COLOR);
    output_text_.setFillColor(WHITE_COLOR);
    setColors();
    update();
}

void InputBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (is_visible)
    {
        target.draw(box_, states);
        target.draw(input_box_, states);

        if (!label_.empty())
        {
            target.draw(label_text_, states);
        }

        if (!input_.empty())
        {
            target.draw(input_text_, states);
        }

        if (!output_.empty())
        {
            target.draw(output_text_, states);
        }
    }
}

bool InputBox::handleEvent(const sf::Event& event)
{
    if (is_visible)
    {
        text_entered_ = false;

        // Toggle input box focus
        if ((event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
        {
            sf::Vector2f mouse_button(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
            bool in_box = ((box_.getPosition().x < mouse_button.x) && (mouse_button.x < box_.getPosition().x + box_.getSize().x) &&
                           (box_.getPosition().y < mouse_button.y) && (mouse_button.y < box_.getPosition().y + box_.getSize().y));

            bool old_focus = has_focus_;
            has_focus_ = in_box;
            setColors();

            if (old_focus ^ in_box)
            {
                return true;
            }
        }

        // Enter expression from input box
        if (has_focus_ && (event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Enter))
        {
            text_entered_ = true;
            return true;
        }

        // Input text expression to input box
        if (has_focus_ && (event.type == sf::Event::TextEntered) && (event.text.unicode <= INPUT_UNICODE_MAX))
        {
            if ((event.text.unicode == '\b') && (!input_.empty()))
            {
                input_.pop_back();
                setInput(input_);
            }
            else if (event.text.unicode >= INPUT_UNICODE_MIN)
            {
                setInput(input_ + static_cast<char>(event.text.unicode));
            }
            return true;
        }
    }

    return false;
}

void InputBox::setLabel(const std::string& text)
{
    label_ = text;
    label_text_.setString(label_);
    update();
}

void InputBox::setInput(const std::string& text)
{
    input_ = text;
    input_text_.setString(input_);
    update();
}

void InputBox::setOutput(const std::string& text)
{
    output_ = text;
    output_text_.setString(output_);
    update();
}

std::string InputBox::getInput() const
{
    return input_;
}

bool InputBox::TextEntered() const
{
    return text_entered_;
}

bool InputBox::hasFocus() const
{
    return has_focus_;
}

void InputBox::update()
{
    vec2f label_size = label_.empty() ? vec2f() : vec2f(label_text_.getLocalBounds().width, label_text_.getLocalBounds().height);
    vec2f input_size = input_.empty() ? vec2f() : vec2f(input_text_.getLocalBounds().width, input_text_.getLocalBounds().height);
    vec2f output_size = output_.empty() ? vec2f() : vec2f(output_text_.getLocalBounds().width, output_text_.getLocalBounds().height);

    input_box_.setSize(sf::Vector2f(EXPAND(std::max(FMUL(MIN_FACTOR, font_size_.x), input_size.x)), EXPAND(std::max(font_size_.y, input_size.y))));
    box_.setSize(sf::Vector2f(EXPAND(std::max(input_box_.getSize().x, EXPAND(output_size.x)) + EXPAND(label_size.x)) + offset_, EXPAND(input_box_.getSize().y + EXPAND(output_size.y))));
    input_box_.setPosition(sf::Vector2f(EXPAND(box_.getPosition().x + EXPAND(label_size.x)), EXPAND(box_.getPosition().y)));

    label_text_.setPosition(sf::Vector2f(box_.getPosition().x + offset_, input_box_.getPosition().y));
    input_text_.setPosition(sf::Vector2f(input_box_.getPosition().x, input_box_.getPosition().y));
    output_text_.setPosition(sf::Vector2f(input_box_.getPosition().x, input_box_.getPosition().y + input_box_.getSize().y));
}

void InputBox::setColors()
{
    input_text_.setFillColor(has_focus_ ? DARK_GREY_COLOR : WHITE_COLOR);
    input_box_.setFillColor(has_focus_ ? LIGHT_GREY_COLOR : DARK_GREY_COLOR);
}