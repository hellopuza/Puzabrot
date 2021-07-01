/*------------------------------------------------------------------------------
    * File:        InputBox.h                                                  *
    * Description: Functions and data types used for SFML input box            *
    * Created:     30 jun 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef INPUTBOX_H_INCLUDED
#define INPUTBOX_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS

#include <SFML/Graphics.hpp>

//------------------------------------------------------------------------------

class InputBox
{
public:

    InputBox () :
        box_pos_   ({0.0f, 0.0f}),
        box_color_ (sf::Color(128, 128, 128)),
        text_color_(sf::Color::White),
        font_size_ (20)
    {}

    InputBox (sf::Vector2f box_pos, sf::Color box_color, sf::Color text_color, size_t font_size) :
        box_pos_   (box_pos),
        box_color_ (box_color),
        text_color_(text_color),
        font_size_ (font_size)
    {}

    ~InputBox () {}

    void draw (sf::RenderWindow* window)
    {
        sf::RectangleShape input_box;
        if (input_text_.getString().isEmpty())
            input_box.setSize(sf::Vector2f(8.0f * font_size_, 1.3f * font_size_));
        else
            input_box.setSize(sf::Vector2f((8.0f * font_size_ > 0.56f * font_size_ * input_text_.getString().getSize() ? 8.0f * font_size_ : 0.56f * font_size_ * input_text_.getString().getSize()), 1.3f * font_size_));

        input_box.setFillColor(sf::Color(40, 40, 40));
        input_box.setOutlineThickness(2.0f);
        input_box.setOutlineColor(sf::Color(200, 200, 200));

        sf::RectangleShape box;
        if (output_text_.getString().isEmpty())
            box.setSize(sf::Vector2f(input_box.getSize().x + input_box.getSize().y, 1.5f * input_box.getSize().y));
        else
            box.setSize(sf::Vector2f(1.5f * input_box.getSize().y + (input_box.getSize().x > 0.56f * font_size_ * output_text_.getString().getSize() ? input_box.getSize().x : 0.56f * font_size_ * output_text_.getString().getSize()), 1.5f * input_box.getSize().y + font_size_));
        box.setFillColor(box_color_);

        box.setPosition(box_pos_);
        input_box.setPosition(sf::Vector2f(box_pos_.x + 0.5f * input_box.getSize().y, box_pos_.y + 0.25f * input_box.getSize().y));
        window->draw(box);
        window->draw(input_box);

        sf::Font font;
        font.loadFromFile("consola.ttf");

        if (not output_text_.getString().isEmpty())
        {
            output_text_.setFont(font);
            output_text_.setPosition(sf::Vector2f(input_box.getPosition().x, input_box.getPosition().y + input_box.getSize().y));
            output_text_.setCharacterSize(font_size_);
            output_text_.setFillColor(text_color_);
            window->draw(output_text_);
        }

        if (not input_text_.getString().isEmpty())
        {
            input_text_.setFont(font);
            input_text_.setPosition(input_box.getPosition());
            input_text_.setCharacterSize(font_size_);
            input_text_.setFillColor(sf::Color::White);
            window->draw(input_text_);
        }

        window->display();
    }

    void setText (const sf::String& text)
    {
        output_text_.setString(text);
    }

    const sf::String& getText () const
    {
        return input_text_.getString();
    }

private:

    sf::Text input_text_;
    sf::Text output_text_;

    sf::Vector2f box_pos_;
    sf::Color    box_color_;
    sf::Color    text_color_;
    size_t       font_size_;
};

//------------------------------------------------------------------------------

#endif // INPUTBOX_H_INCLUDED
