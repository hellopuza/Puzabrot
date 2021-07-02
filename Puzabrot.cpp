/*------------------------------------------------------------------------------
    * File:        Puzabrot.cpp                                                *
    * Description: Functions for application                                   *
    * Created:     27 mar 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#include "Puzabrot.h"

//------------------------------------------------------------------------------

Puzabrot::Puzabrot () :
    winsizes_ ({DEFAULT_WIDTH, DEFAULT_HEIGHT})
{
    pointmap_ = new sf::VertexArray(sf::Points, winsizes_.x * winsizes_.y);
    window_   = new sf::RenderWindow(sf::VideoMode(winsizes_.x, winsizes_.y), title_string);
    
    borders_.Im_up   =  1.3;
    borders_.Im_down = -1.3;

    borders_.Re_left  = -(borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *3;
    borders_.Re_right =  (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *2;
}

//------------------------------------------------------------------------------

Puzabrot::~Puzabrot ()
{
    if (window_->isOpen()) window_->close();
    delete pointmap_;
    delete window_;
}

//------------------------------------------------------------------------------

void Puzabrot::run ()
{
    window_->setVerticalSyncEnabled(true);

    // DrawPuzabrot();

    InputBox input_box(sf::Vector2f(10, 10), sf::Color(128, 128, 128, 128), sf::Color::White, 20);
    input_box.setInput(sf::String("z^2+c"));

    while (window_->isOpen())
    {
        sf::Event event;
        while (window_->pollEvent(event))
        {
            if ( ( event.type == sf::Event::Closed) ||
                 ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)) )
            {
                window_->close();
                return;
            }

            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F11))
            {
                toggleFullScreen();
            }

            if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window_->setView(sf::View(visibleArea));
                updateWinSizes(window_->getSize().x, window_->getSize().y);
            }

            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Backslash))
            {
                input_box.is_visible_ = 1 - input_box.is_visible_;
                if (not input_box.is_visible_)
                    input_box.has_focus_ = false;
            }

            if (input_box.is_visible_ && (event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
            {
                if ((input_box.getPos().x < event.mouseButton.x) && (event.mouseButton.x < input_box.getPos().x + input_box.getSize().x) &&
                    (input_box.getPos().y < event.mouseButton.y) && (event.mouseButton.y < input_box.getPos().y + input_box.getSize().y))
                    input_box.has_focus_ = true;
                else
                    input_box.has_focus_ = false;
            }

            if (input_box.has_focus_ && (event.type == sf::Event::TextEntered) && (event.text.unicode < 128))
            {
                input_box.setInput(event.text.unicode);
            }
        }

        window_->clear();
        if (input_box.is_visible_)
        {
            input_box.draw(window_);
        }
        else
        {
            window_->display();
            // DrawPuzabrot();
        }
    }
}

//------------------------------------------------------------------------------

void Puzabrot::updateWinSizes (size_t width, size_t height)
{
    winsizes_.x = width;
    winsizes_.y = height;

    delete pointmap_;
    pointmap_ = new sf::VertexArray(sf::Points, winsizes_.x * winsizes_.y);

    borders_.Re_right = borders_.Re_left + (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y;

    // DrawPuzabrot();
}

//------------------------------------------------------------------------------

void Puzabrot::toggleFullScreen ()
{
    if (window_->isOpen()) window_->close();
    delete window_;

    if ((winsizes_.x == sf::VideoMode::getDesktopMode().width) && (winsizes_.y == sf::VideoMode::getDesktopMode().height))
    {
        window_ = new sf::RenderWindow(sf::VideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT), title_string);
        updateWinSizes(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
    else
    {
        window_ = new sf::RenderWindow(sf::VideoMode::getDesktopMode(), title_string, sf::Style::Fullscreen);
        updateWinSizes(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
    }
}

//------------------------------------------------------------------------------
