#include "InputBox.h"

#include <SFML/Graphics.hpp>

const sf::Color GREY_COLOR = sf::Color(128, 128, 128);
const sf::Color LIGHT_GREY_COLOR = sf::Color(200, 200, 200);
const sf::Color DARK_GREY_COLOR = sf::Color(200, 200, 200);

const float BOX_OUTLINE_THICKNESS = 2.0F;
const float DEFAULT_FONT_SIZE = 20.0F;
const float FONT_FACTOR = 0.56F;
const float HALF = 0.5F;
const float HORIZONTAL_FONT_FACTOR = 8.0F;
const float QUARTER = 0.25F;
const float VERTICAL_FONT_FACTOR = 1.3F;
const float VERTICAL_BOX_FACTOR = 1.5F;

InputBox::InputBox() :
    box_pos_({ 0.0F, 0.0F }), box_color_(GREY_COLOR), text_color_(sf::Color::White), font_size_(DEFAULT_FONT_SIZE)
{
    font_.loadFromFile("assets/consola.ttf");
}

InputBox::InputBox(sf::Vector2f box_pos, sf::Color box_color, sf::Color text_color, float font_size) :
    box_pos_(box_pos), box_color_(box_color), text_color_(text_color), font_size_(font_size)
{
    font_.loadFromFile("assets/consola.ttf");
}

void InputBox::draw(sf::RenderWindow& window)
{
    if (is_visible)
    {
        sf::RectangleShape input_box;
        sf::RectangleShape box;

        auto input_text_size = static_cast<float>(input_text_.getString().getSize());
        if (input_text_.getString().isEmpty())
        {
            input_box.setSize(sf::Vector2f(HORIZONTAL_FONT_FACTOR * font_size_, VERTICAL_FONT_FACTOR * font_size_));
        }
        else
        {
            input_box.setSize(sf::Vector2f((HORIZONTAL_FONT_FACTOR * font_size_ > FONT_FACTOR * font_size_ * input_text_size ?
                                            HORIZONTAL_FONT_FACTOR * font_size_ : FONT_FACTOR * font_size_ * input_text_size),
                                            VERTICAL_FONT_FACTOR * font_size_));
        }

        input_box.setFillColor(DARK_GREY_COLOR);
        input_box.setOutlineThickness(BOX_OUTLINE_THICKNESS);
        if (has_focus)
        {
            input_box.setOutlineColor(sf::Color::Yellow);
        }
        else
        {
            input_box.setOutlineColor(LIGHT_GREY_COLOR);
        }

        auto label_size       = static_cast<float>(label_.getString().getSize());
        auto output_text_size = static_cast<float>(output_text_.getString().getSize());
        if (output_text_.getString().isEmpty())
        {
            box_size_ = sf::Vector2f(input_box.getSize().x + input_box.getSize().y, VERTICAL_BOX_FACTOR * input_box.getSize().y);
        }
        else
        {
            box_size_ = sf::Vector2f(VERTICAL_BOX_FACTOR * input_box.getSize().y +
                                     (input_box.getSize().x > FONT_FACTOR * font_size_ * output_text_size ?
                                     input_box.getSize().x : FONT_FACTOR * font_size_ * output_text_size),
                                     VERTICAL_BOX_FACTOR * input_box.getSize().y + font_size_);
        }

        if (not label_.getString().isEmpty())
        {
            box_size_.x += FONT_FACTOR * font_size_ * label_size + QUARTER * font_size_;
        }

        box.setSize(box_size_);
        box.setFillColor(box_color_);

        box.setPosition(box_pos_);
        if (label_.getString().isEmpty())
        {
            input_box.setPosition(sf::Vector2f(box_pos_.x + HALF * input_box.getSize().y, box_pos_.y + QUARTER * input_box.getSize().y));
        }
        else
        {
            input_box.setPosition(sf::Vector2f(box_pos_.x + HALF * input_box.getSize().y + FONT_FACTOR * font_size_ * label_size +
                                               QUARTER * font_size_, box_pos_.y + QUARTER * input_box.getSize().y));
        }

        window.draw(box);
        window.draw(input_box);

        if (not label_.getString().isEmpty())
        {
            label_.setFont(font_);
            label_.setPosition(sf::Vector2f(input_box.getPosition().x - FONT_FACTOR * font_size_ * label_size - QUARTER * font_size_,
                                            input_box.getPosition().y));

            label_.setCharacterSize(static_cast<unsigned int>(font_size_));
            label_.setFillColor(text_color_);
            window.draw(label_);
        }

        if (not output_text_.getString().isEmpty())
        {
            output_text_.setFont(font_);
            output_text_.setPosition(sf::Vector2f(input_box.getPosition().x, input_box.getPosition().y + input_box.getSize().y));

            output_text_.setCharacterSize(static_cast<unsigned int>(font_size_));
            output_text_.setFillColor(text_color_);
            window.draw(output_text_);
        }

        if (not input_text_.getString().isEmpty())
        {
            input_text_.setFont(font_);
            input_text_.setPosition(input_box.getPosition());
            input_text_.setCharacterSize(static_cast<unsigned int>(font_size_));
            input_text_.setFillColor(sf::Color::White);
            window.draw(input_text_);
        }
    }
}

void InputBox::setInput(const sf::String& text)
{
    sf::String input = input_text_.getString();
    if (text == sf::String("\b"))
    {
        if (input.getSize() != 0)
        {
            input.erase(input.getSize() - 1, 1);
        }
    }
    else
    {
        input += text;
    }
    input_text_.setString(input);
}

void InputBox::setLabel(const sf::String& text)
{
    label_.setString(text);
}

void InputBox::setOutput(const sf::String& text)
{
    output_text_.setString(text);
}

const sf::String& InputBox::getInput() const
{
    return input_text_.getString();
}

const sf::Vector2f& InputBox::getPosition() const
{
    return box_pos_;
}

const sf::Vector2f& InputBox::getSize() const
{
    return box_size_;
}

void InputBox::setPosition(const sf::Vector2f& pos)
{
    box_pos_ = pos;
}

void InputBox::setSize(const sf::Vector2f& size)
{
    box_size_ = size;
}