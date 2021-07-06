/*------------------------------------------------------------------------------
    * File:        ProgressBar.h                                               *
    * Description: Functions and data types used for SFML progress bar         *
    * Created:     6  jul 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef PROGRESSBAR_H_INCLUDED
#define PROGRESSBAR_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS

#include <SFML/Graphics.hpp>

//------------------------------------------------------------------------------

class ProgressBar
{
public:

    ProgressBar (sf::Vector2f pos = { 0, 0 }, sf::Vector2f size = { 0, 0 }, sf::Color col = sf::Color::Black) :
        pos_  (pos),
        size_ (size),
        col_  (col)
    {
        outer_rect_.setPosition(pos_);
        outer_rect_.setSize(size_);

        outer_rect_.setFillColor(sf::Color(40, 40, 40));
        outer_rect_.setOutlineThickness(2.0f);
        outer_rect_.setOutlineColor(sf::Color(40, 40, 40));

        inter_rect_.setPosition(pos_);
        inter_rect_.setSize(sf::Vector2f(size_.x * progress_, size_.y));

        inter_rect_.setFillColor(col_);
        inter_rect_.setOutlineThickness(0.0f);

        font_.loadFromFile("consola.ttf");

        prog_text_.setFont(font_);
        prog_text_.setPosition(pos_.x + size_.x/2 - size_.y * 1.6, pos_.y);
        prog_text_.setCharacterSize(size_.y * 0.8);
    }

    ~ProgressBar () {}

    void draw (sf::RenderWindow* window)
    {
        window->draw(outer_rect_);

        inter_rect_.setSize(sf::Vector2f(size_.x * progress_, size_.y));
        window->draw(inter_rect_);

        char str[8] = "";
        sprintf(str, "%.1f%%", progress_ * 100);

        prog_text_.setString(str);
        window->draw(prog_text_);

        window->display();
    }

    void setProgress (float progress)
    {
        progress_ = progress;
    }

private:

    sf::Vector2f pos_  = { 0, 0 };
    sf::Vector2f size_ = { 0, 0 };
    sf::Color    col_  = sf::Color::Black;
    float        progress_ = 0;

    sf::RectangleShape outer_rect_;
    sf::RectangleShape inter_rect_;
    sf::Text           prog_text_;
    sf::Font           font_;
};

//------------------------------------------------------------------------------

#endif // PROGRESSBAR_H_INCLUDED
