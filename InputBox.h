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
        box_size_  ({200.0f, 50.0f}),
        box_color_ (sf::Color(128, 128, 128)),
        text_color_(sf::Color::White),
        font_size_ (20)
    {}

    InputBox (sf::Vector2f box_pos, sf::Vector2f box_size, sf::Color box_color, sf::Color text_color, size_t font_size) :
        box_pos_   (box_pos),
        box_size_  (box_size),
        box_color_ (box_color),
        text_color_(text_color),
        font_size_ (font_size)
    {}

    ~InputBox () {}

    void draw (sf::RenderWindow* window)
    {
        sf::RectangleShape box;
        box.setPosition(box_pos_);
        if (output_text.getString().isEmpty())
            box.setSize(box_size_);
        else
            box.setSize(sf::Vector2f(box_size_.x, box_size_.y + 0.4f * font_size_));
        box.setFillColor(box_color_);
        window->draw(box);

        sf::RectangleShape input_box;
        input_box.setPosition(sf::Vector2f(box_pos_.x + 0.1f * box_size_.x, box_pos_.y + 0.1f * box_size_.y));
        input_box.setSize(sf::Vector2f(0.8f * box_size_.x, 1.2f * font_size_));
        input_box.setFillColor(sf::Color(40, 40, 40));
        input_box.setOutlineThickness(2.0f);
        input_box.setOutlineColor(sf::Color(200, 200, 200));
        window->draw(input_box);

        sf::Font font;
        font.loadFromFile("consola.ttf");

        if (not output_text.getString().isEmpty())
        {
            output_text.setFont(font);
            output_text.setPosition(sf::Vector2f(box_pos_.x + 0.1f * box_size_.x, box_pos_.y + 0.1f * box_size_.y + 1.2f * font_size_));
            output_text.setCharacterSize(font_size_);
            output_text.setFillColor(text_color_);
            window->draw(output_text);
        }

        if (not input_text.getString().isEmpty())
        {
            input_text.setFont(font);
            input_text.setPosition(sf::Vector2f(box_pos_.x + 0.1f * box_size_.x, box_pos_.y + 0.1f * box_size_.y + 0.1f * font_size_));
            input_text.setCharacterSize(font_size_);
            input_text.setFillColor(sf::Color(200, 200, 200));
            window->draw(input_text);
        }

        window->display();
    }

    void setText (const sf::String& text)
    {
        output_text.setString(text);
    }

    const sf::String& getText () const
    {
        return input_text.getString();
    }

private:

    sf::Text input_text;
    sf::Text output_text;

    sf::Vector2f box_pos_;
    sf::Vector2f box_size_;
    sf::Color    box_color_;
    sf::Color    text_color_;
    size_t       font_size_;
};

//------------------------------------------------------------------------------

#endif // INPUTBOX_H_INCLUDED
