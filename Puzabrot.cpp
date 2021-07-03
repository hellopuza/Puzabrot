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

    InputBox input_box(sf::Vector2f(10, 10), sf::Color(128, 128, 128, 128), sf::Color::White, 20);
    input_box.setInput(sf::String("z^2+c"));

    std::string string = input_box.getInput().toAnsiString();
    char* str = (char*)string.c_str();
    char* expr = new char[MAX_STR_LEN] {};
    strcpy(expr, str);
    Expression expression = { expr, expr };
    int err = Expr2Tree(expression, calc_.trees_[0]);
    delete [] expr;

    DrawSet();
    window_->display();

    while (window_->isOpen())
    {
        sf::Event event;
        Screen newscreen = {};
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

            if (input_box.has_focus_ && (event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Enter))
            {
                string = input_box.getInput().toAnsiString();
                str = (char*)string.c_str();
                expr = new char[MAX_STR_LEN] {};
                strcpy(expr, str);
                expression = { expr, expr };
                err = Expr2Tree(expression, calc_.trees_[0]);
                delete [] expr;

                DrawSet();
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                if (GetNewScreen(newscreen))
                {
                    changeBorders(newscreen);

                    if (newscreen.zoom > 1)
                        itrn_max_ = (int)(itrn_max_*(1 + newscreen.zoom/delta_zoom_));
                    else
                        itrn_max_ = (int)(itrn_max_*(1 - 1/(newscreen.zoom*delta_zoom_ + 1)));

                    DrawSet();
                }
            }
        }

        window_->clear();
        window_->draw(*pointmap_);
        if (input_box.is_visible_)
            input_box.draw(window_);
        else
            window_->display();
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

    DrawSet();
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

void Puzabrot::DrawSet ()
{
    assert(itrn_max_);
    assert(lim_);

    int width  = winsizes_.x;
    int height = winsizes_.y;

    double re_step = (borders_.Re_right - borders_.Re_left) / width;
    double im_step = (borders_.Im_up    - borders_.Im_down) / height;

    double im0 = borders_.Im_down;

    int x_offset = 0;

    for (int y = 0; y < height; (++y, im0 += im_step, x_offset += width))
    {
        double re0 = borders_.Re_left;

        //#pragma omp parallel for
        for (int x = 0; x < width; ++x)
        {
            calc_.variables_.Push({ {re0, im0}, "c" });
            calc_.variables_.Push({ {re0, im0}, "z" });

            int i = 1;
            calc_.trees_[0].root_->setData({ {re0, im0}, calc_.trees_[0].root_->getData().word, calc_.trees_[0].root_->getData().op_code, calc_.trees_[0].root_->getData().node_type });
            for (; (i < itrn_max_) && (abs(calc_.trees_[0].root_->getData().number) < lim_); ++i)
            {
                calc_.Calculate(calc_.trees_[0].root_);
                calc_.variables_[calc_.variables_.getSize() - 1] = { calc_.trees_[0].root_->getData().number, "z" };
            }

            (*pointmap_)[x_offset + x].position = sf::Vector2f(x, y);
            if (i < itrn_max_)
                (*pointmap_)[x_offset + x].color = getColor(i);
            else
                (*pointmap_)[x_offset + x].color = sf::Color::Black;

            calc_.variables_.Clean();
            ADD_VAR(calc_.variables_);
            
            re0 += re_step;
        }
    }
}

//------------------------------------------------------------------------------

sf::Color Puzabrot::getColor (int32_t itrn)
{
    if (itrn < itrn_max_)
    {
        itrn = itrn*4 % 1530;

             if (itrn < 256 ) return sf::Color( 255,       itrn,      0         );
        else if (itrn < 511 ) return sf::Color( 510-itrn,  255,       0         );
        else if (itrn < 766 ) return sf::Color( 0,         255,       itrn-510  );
        else if (itrn < 1021) return sf::Color( 0,         1020-itrn, 255       );
        else if (itrn < 1276) return sf::Color( itrn-1020, 0,         255       );
        else if (itrn < 1530) return sf::Color( 255,       0,         1529-itrn );
    }

    return sf::Color( 0, 0, 0 );
}

//------------------------------------------------------------------------------

int Puzabrot::GetNewScreen (Screen& newscreen)
{
    int w = winsizes_.x;
    int h = winsizes_.y;

    char was_screenshot = 0;

    sf::Vector2i start(-1, -1);
    sf::Vector2i end  (-1, -1);

    sf::RectangleShape rectangle;
    rectangle.setOutlineThickness(1);
    rectangle.setFillColor(sf::Color::Transparent);

#ifdef __linux__

    sf::Texture screen;
    screen.create(w, h);
    screen.update(*window_);

    sf::Sprite sprite(screen);

#endif // __linux__

    while (1)
    {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            start = sf::Mouse::getPosition(*window_);
            rectangle.setPosition(start.x, start.y);

            while (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                end = sf::Mouse::getPosition(*window_) + sf::Vector2i(1, 1);

                if ((abs(end.x - start.x) > 8) && (abs(end.y - start.y) > 8))
                {
                    double sx = start.x;
                    double sy = start.y;
                    double ex = end.x;
                    double ey = end.y;


                    if ( ((end.y > start.y) && (end.x > start.x)) ||
                         ((end.y < start.y) && (end.x < start.x))   )
                    {
                        end.x = (int)((w*h*(ey - sy) + w*w*ex + h*h*sx)/(w*w + h*h));
                        end.y = (int)((w*h*(ex - sx) + w*w*sy + h*h*ey)/(w*w + h*h));
                    }
                    else
                    {
                        end.x = (int)((w*h*(sy - ey) + w*w*ex + h*h*sx)/(w*w + h*h));
                        end.y = (int)((w*h*(sx - ex) + w*w*sy + h*h*ey)/(w*w + h*h));
                    }


                    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                    {
                        rectangle.setOutlineColor(sf::Color::Blue);
                        newscreen.zoom = (double)w*h/abs(end.x - start.x)/abs(end.y - start.y);
                    }
                    else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
                    {
                        rectangle.setOutlineColor(sf::Color::Red);
                        newscreen.zoom = (double)abs(end.x - start.x)*abs(end.y - start.y)/w/h;
                    }

                    rectangle.setSize(sf::Vector2f(end - start));

                    #ifdef __linux__
                    window_->draw(sprite);
                    #else
                    window_->draw(*pointmap_);
                    #endif // __linux__

                    window_->draw(rectangle);
                    window_->display();
                }
                else end.x = -1;
            }
        }

        #ifndef __linux__
        window_->draw(*pointmap_);
        #endif // __linux__

        if (end.x != -1) break;
        else return 0;
    }


    if (start.x > end.x)
    {
        newscreen.x2 = start.x;
        newscreen.x1 = end.x;
    }
    else
    {
        newscreen.x1 = start.x;
        newscreen.x2 = end.x;
    }

    if (start.y > end.y)
    {
        newscreen.y2 = start.y;
        newscreen.y1 = end.y;
    }
    else
    {
        newscreen.y1 = start.y;
        newscreen.y2 = end.y;
    }

    return 1;
}

//------------------------------------------------------------------------------

void Puzabrot::changeBorders (Screen newscreen)
{
    double releft  = borders_.Re_left;
    double reright = borders_.Re_right;
    double imup    = borders_.Im_up;
    double imdown  = borders_.Im_down;

    if (newscreen.zoom > 1)
    {
        borders_.Re_left  = releft + (reright - releft) * newscreen.x1 / winsizes_.x;
        borders_.Re_right = releft + (reright - releft) * newscreen.x2 / winsizes_.x;
        borders_.Im_down  = imdown + (imup    - imdown) * newscreen.y1 / winsizes_.y;

        borders_.Im_up = borders_.Im_down + (borders_.Re_right - borders_.Re_left) * winsizes_.y / winsizes_.x;
    }
    else
    {
        borders_.Re_left  = releft  - (reright - releft) *                newscreen.x1  / (newscreen.x2 - newscreen.x1);
        borders_.Re_right = reright + (reright - releft) * (winsizes_.x - newscreen.x2) / (newscreen.x2 - newscreen.x1);
        borders_.Im_down  = imdown  - (imup    - imdown) *                newscreen.y1  / (newscreen.y2 - newscreen.y1);

        borders_.Im_up = borders_.Im_down + (borders_.Re_right - borders_.Re_left) * winsizes_.y / winsizes_.x;
    }
}

//------------------------------------------------------------------------------
