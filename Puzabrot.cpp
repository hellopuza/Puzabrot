/*------------------------------------------------------------------------------
    * File:        Puzabrot.cpp                                                *
    * Description: Functions for application                                   *
    * Created:     30 jun 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#include "Puzabrot.h"

//------------------------------------------------------------------------------

Puzabrot::Puzabrot () :
    winsizes_    ({ DEFAULT_WIDTH, DEFAULT_HEIGHT }),
    input_box_x_ (sf::Vector2f(10, 10), sf::Color(128, 128, 128, 128), sf::Color::White, 20),
    input_box_y_ (sf::Vector2f(10, 50), sf::Color(128, 128, 128, 128), sf::Color::White, 20),
    input_box_z_ (sf::Vector2f(10, 10), sf::Color(128, 128, 128, 128), sf::Color::White, 20)
{
    window_ = new sf::RenderWindow(sf::VideoMode(winsizes_.x, winsizes_.y), title_string);
    
    borders_.Im_up   =  UPPER_BORDER;
    borders_.Im_down = -UPPER_BORDER;

    borders_.Re_left  = -(borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *3;
    borders_.Re_right =  (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *2;

    expr_trees_[0] = Tree<CalcNodeData>((char*)"Expression tree 1");
    expr_trees_[1] = Tree<CalcNodeData>((char*)"Expression tree 2");

    input_box_x_.setLabel(sf::String("x:"));
    input_box_y_.setLabel(sf::String("y:"));
    input_box_z_.setLabel(sf::String("z:"));

    input_box_x_.setInput(sf::String("x*x-y*y+cx"));
    input_box_y_.setInput(sf::String("2*x*y+cy"));

    input_box_z_.setInput(sf::String("z^2+c"));

    render_texture_.create(winsizes_.x, winsizes_.y);
    sprite_ = sf::Sprite(render_texture_.getTexture());
}

//------------------------------------------------------------------------------

Puzabrot::~Puzabrot ()
{
    if (window_->isOpen()) window_->close();
    delete window_;
}

//------------------------------------------------------------------------------

void Puzabrot::run ()
{
    window_->setVerticalSyncEnabled(true);

    int action_mode  = POINT_TRACING;

    makeShader();
    DrawSet();

    bool showing_menu   = false;
    bool showing_trace  = false;
    bool julia_dragging = false;
    bool change_iter    = false;
    bool change_limit   = false;

    sf::Vector2f orbit   = sf::Vector2f(0, 0);
    sf::Vector2f c_point = sf::Vector2f(0, 0);

    Synth synth(this);
    synth.play();

    while (window_->isOpen())
    {
        sf::Event event;
        Screen newscreen = {};
        bool was_screenshot = 0;
        while (window_->pollEvent(event))
        {
            //Close window
            if ( ( event.type == sf::Event::Closed) ||
                 ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)) )
            {
                window_->close();
                return;
            }

            //Toggle fullscreen
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F11))
            {
                toggleFullScreen();

                DrawSet();
            }

            //Resize window
            else if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window_->setView(sf::View(visibleArea));
                updateWinSizes(window_->getSize().x, window_->getSize().y);

                DrawSet();
            }

            //Reset set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R) && (not InputBoxesHasFocus()))
            {
                borders_.Im_up   =  UPPER_BORDER;
                borders_.Im_down = -UPPER_BORDER;

                borders_.Re_left  = -(borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *3;
                borders_.Re_right =  (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *2;

                itrn_max_ = MAX_ITERATION;
                lim_      = LIMIT;

                draw_mode_ = MAIN;
                DrawSet();
            }

            //Take a screenshot
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space) && (!was_screenshot) && (not InputBoxesHasFocus()))
            {
                savePicture();
                was_screenshot = 1;
            }

            //Toggle audio dampening
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::D) && (not InputBoxesHasFocus()))
            {
                synth.sustain_ = not synth.sustain_;
            }

            //Toggle sound coloring
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::C) && (not InputBoxesHasFocus()))
            {
                coloring_ = not coloring_;

                DrawSet();
            }

            //Toggle help menu showing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::H) && (not InputBoxesHasFocus()))
            {
                showing_menu = not showing_menu;
                if (showing_menu)
                {
                    input_box_x_.is_visible_ = false;
                    input_box_y_.is_visible_ = false;
                    input_box_z_.is_visible_ = false;
                }
            }

            //Toggle input mode
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tab) && (not InputBoxesHasFocus()))
            {
                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    input_box_x_.has_focus_ = false;
                    if (input_box_z_.is_visible_)
                        input_box_x_.is_visible_ = true;
                    else
                        input_box_x_.is_visible_ = false;

                    input_box_y_.has_focus_ = false;
                    if (input_box_z_.is_visible_)
                        input_box_y_.is_visible_ = true;
                    else
                        input_box_y_.is_visible_ = false;

                    input_box_z_.has_focus_  = false;
                    input_box_z_.is_visible_ = false;

                    input_mode_ = XY_INPUT;
                    break;
                }
                case XY_INPUT:
                {
                    input_box_z_.has_focus_  = false;
                    if (input_box_x_.is_visible_)
                        input_box_z_.is_visible_ = true;
                    else
                        input_box_z_.is_visible_ = false;

                    input_box_x_.has_focus_  = false;
                    input_box_x_.is_visible_ = false;

                    input_box_y_.has_focus_  = false;
                    input_box_y_.is_visible_ = false;

                    input_mode_ = Z_INPUT;
                    break;
                }
                }
            }

            //Toggle input box visibility
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tilde) && (not InputBoxesHasFocus()))
            {
                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    input_box_z_.is_visible_ = not input_box_z_.is_visible_;
                    if (not input_box_z_.is_visible_)
                        input_box_z_.has_focus_ = false;
                    break;
                }
                case XY_INPUT:
                {
                    input_box_x_.is_visible_ = not input_box_x_.is_visible_;
                    input_box_y_.is_visible_ = not input_box_y_.is_visible_;

                    if (not input_box_x_.is_visible_)
                    {
                        input_box_x_.has_focus_ = false;
                        input_box_y_.has_focus_ = false;
                    }
                    break;
                }
                }
            }

            //Toggle input box focus
            else if (InputBoxesIsVisible() && (event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
            {
                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    if ((input_box_z_.getPosition().x < event.mouseButton.x) && (event.mouseButton.x < input_box_z_.getPosition().x + input_box_z_.getSize().x) &&
                        (input_box_z_.getPosition().y < event.mouseButton.y) && (event.mouseButton.y < input_box_z_.getPosition().y + input_box_z_.getSize().y))
                        input_box_z_.has_focus_ = true;
                    else
                        input_box_z_.has_focus_ = false;
                    break;
                }
                case XY_INPUT:
                {
                    if ((input_box_x_.getPosition().x < event.mouseButton.x) && (event.mouseButton.x < input_box_x_.getPosition().x + input_box_x_.getSize().x) &&
                        (input_box_x_.getPosition().y < event.mouseButton.y) && (event.mouseButton.y < input_box_x_.getPosition().y + input_box_x_.getSize().y))
                        input_box_x_.has_focus_ = true;
                    else
                        input_box_x_.has_focus_ = false;

                    if ((input_box_y_.getPosition().x < event.mouseButton.x) && (event.mouseButton.x < input_box_y_.getPosition().x + input_box_y_.getSize().x) &&
                        (input_box_y_.getPosition().y < event.mouseButton.y) && (event.mouseButton.y < input_box_y_.getPosition().y + input_box_y_.getSize().y))
                        input_box_y_.has_focus_ = true;
                    else
                        input_box_y_.has_focus_ = false;
                    break;
                }
                }
            }

            //Input text expression to input box
            else if (InputBoxesHasFocus() && (event.type == sf::Event::TextEntered) && (event.text.unicode < 128))
            {
                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    input_box_z_.setInput(event.text.unicode);
                    break;
                }
                case XY_INPUT:
                {
                    if (input_box_x_.has_focus_)
                        input_box_x_.setInput(event.text.unicode);
                    else
                        input_box_y_.setInput(event.text.unicode);
                    break;
                }
                }
            }

            //Enter expression from input box
            else if (InputBoxesHasFocus() && (event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Enter))
            {
                int err = makeShader();

                if (!err)
                {
                    DrawSet();
                    draw_mode_ = MAIN;
                }

                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    if (err)
                        input_box_z_.setOutput(sf::String(calc_errstr[err + 1]));
                    else
                        input_box_z_.setOutput(sf::String());
                    break;
                }
                case XY_INPUT:
                {
                    if (input_box_x_.has_focus_)
                        if (err)
                        {
                            input_box_x_.setOutput(sf::String(calc_errstr[err + 1]));
                            input_box_y_.setPosition(sf::Vector2f(input_box_y_.getPosition().x, input_box_y_.getPosition().y + 0.5f * input_box_x_.getSize().y));
                        }
                        else
                        {
                            input_box_x_.setOutput(sf::String());
                            input_box_y_.setPosition(sf::Vector2f(10, 50));
                        }
                    else
                        if (err)
                            input_box_y_.setOutput(sf::String(calc_errstr[err + 1]));
                        else
                            input_box_y_.setOutput(sf::String());
                    break;
                }
                }

            }

            //Julia set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::J) && (not InputBoxesHasFocus()))
            {
                if (draw_mode_ != JULIA)
                {
                    while (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
                    {
                        julia_point_ = Screen2Plane(sf::Mouse::getPosition(*window_));

                        draw_mode_ = JULIA;
                        DrawSet();
                        draw_mode_ = MAIN;

                        window_->draw(sprite_);
                        window_->display();

                        julia_dragging = true;
                    }
                }
                else
                {
                    DrawSet();
                    julia_dragging = false;
                }
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::J) && (not InputBoxesHasFocus()))
            {
                if (not julia_dragging)
                    draw_mode_ = MAIN;
                else
                    draw_mode_ = JULIA;

                DrawSet();
            }

            //Change max iterations
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::I) && (not InputBoxesHasFocus()))
            {
                change_iter = true;
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::I) && (not InputBoxesHasFocus()))
            {
                change_iter = false;
            }
            else if ((event.type == sf::Event::MouseWheelMoved) && change_iter)
            {
                itrn_max_ += event.mouseWheel.delta * 50;

                DrawSet();
            }

            //Change limit
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::L) && (not InputBoxesHasFocus()))
            {
                change_limit = true;
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::L) && (not InputBoxesHasFocus()))
            {
                change_limit = false;
            }
            else if ((event.type == sf::Event::MouseWheelMoved) && change_limit)
            {
                lim_ *= pow(2, event.mouseWheel.delta);

                DrawSet();
            }

            //Toggle action modes
            else if ((event.type == sf::Event::KeyPressed) && (not InputBoxesHasFocus()))
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Z: action_mode = ZOOMING;       break;
                case sf::Keyboard::P: action_mode = POINT_TRACING; break;
                case sf::Keyboard::S: action_mode = SOUNDING;      break;
                }
            }

            //Zooming
            else if (event.type == sf::Event::MouseWheelMoved)
            {
                Zooming(event.mouseWheel.delta, Screen2Plane(sf::Mouse::getPosition(*window_)));

                DrawSet();
            }
            else if ((action_mode == ZOOMING) && (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right)))
            {
                if (GetNewScreen(newscreen))
                {
                    changeBorders(newscreen);

                    DrawSet();
                }
            }

            //Point tracing and sounding
            else if (((action_mode == POINT_TRACING) || (action_mode == SOUNDING)) && (sf::Mouse::isButtonPressed(sf::Mouse::Left)))
            {
                input_box_x_.is_visible_ = false;
                input_box_x_.has_focus_  = false;

                input_box_y_.is_visible_ = false;
                input_box_y_.has_focus_  = false;

                input_box_z_.is_visible_ = false;
                input_box_z_.has_focus_  = false;

                c_point = Screen2Plane(sf::Mouse::getPosition(*window_));
                orbit   = c_point;
                showing_trace = true;
                    
                if (action_mode == SOUNDING)
                {
                    synth.SetPoint(Screen2Plane(sf::Mouse::getPosition(*window_)));
                    synth.audio_pause_ = false;
                    synth.play();
                }
            }
            else if (((action_mode == POINT_TRACING) || (action_mode == SOUNDING)) && (sf::Mouse::isButtonPressed(sf::Mouse::Right)))
            {
                showing_trace = false;
                synth.audio_pause_ = true;
                synth.pause();
            }
        }

        window_->clear();
        window_->draw(sprite_);

        if ((input_mode_ == Z_INPUT) && input_box_z_.is_visible_)
            input_box_z_.draw(window_);
        else
        if ((input_mode_ == XY_INPUT) && input_box_x_.is_visible_)
        {
            input_box_x_.draw(window_);
            input_box_y_.draw(window_);
        }

        if (showing_trace)
            orbit = PointTrace(orbit, c_point);

        if (showing_menu)
            drawHelpMenu();

        window_->display();
    }

    synth.stop();
}

//------------------------------------------------------------------------------

sf::Vector2f Puzabrot::Screen2Plane (sf::Vector2i point)
{
    return sf::Vector2f(borders_.Re_left + (borders_.Re_right - borders_.Re_left) * point.x / winsizes_.x,
                        borders_.Im_up   - (borders_.Im_up    - borders_.Im_down) * point.y / winsizes_.y);
}

//------------------------------------------------------------------------------

void Puzabrot::updateWinSizes (size_t new_width, size_t new_height)
{
    size_t old_width  = winsizes_.x;
    size_t old_height = winsizes_.y;

    winsizes_.x = new_width;
    winsizes_.y = new_height;

    render_texture_.create(winsizes_.x, winsizes_.y);
    sprite_ = sf::Sprite(render_texture_.getTexture());

    borders_.Re_right = borders_.Re_left + (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y;
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

bool Puzabrot::InputBoxesHasFocus ()
{
    return (input_box_x_.has_focus_) || (input_box_y_.has_focus_) || (input_box_z_.has_focus_);
}

//------------------------------------------------------------------------------

bool Puzabrot::InputBoxesIsVisible ()
{
    return (input_box_x_.is_visible_) || (input_box_y_.is_visible_) || (input_box_z_.is_visible_);
}

//------------------------------------------------------------------------------

void Puzabrot::DrawSet ()
{
    shader_.setUniform("borders", sf::Glsl::Vec4(borders_.Re_left, borders_.Re_right, borders_.Im_down, borders_.Im_up));
    shader_.setUniform("winsizes", sf::Glsl::Ivec2(winsizes_.x, winsizes_.y));

    shader_.setUniform("itrn_max", (int)itrn_max_);
    shader_.setUniform("limit",    lim_);

    shader_.setUniform("drawing_mode", draw_mode_);
    shader_.setUniform("coloring",     coloring_);

    shader_.setUniform("julia_point", sf::Glsl::Vec2(julia_point_.x, julia_point_.y));

    render_texture_.draw(sprite_, &shader_);
}

//------------------------------------------------------------------------------

void Puzabrot::Zooming (int wheel_delta, sf::Vector2f point)
{
    float width  = (borders_.Re_right - borders_.Re_left);
    float height = (borders_.Im_up    - borders_.Im_down);

    float x_ratio = (point.x - borders_.Re_left) / width;
    float y_ratio = (point.y - borders_.Im_down) / height;

    ComplexFrame new_frame =
    {
        borders_.Re_left  +      x_ratio  * ZOOMING_RATIO * width  * wheel_delta,
        borders_.Re_right - (1 - x_ratio) * ZOOMING_RATIO * width  * wheel_delta,
        borders_.Im_down  +      y_ratio  * ZOOMING_RATIO * height * wheel_delta,
        borders_.Im_up    - (1 - y_ratio) * ZOOMING_RATIO * height * wheel_delta,
    };

    borders_ = new_frame;
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

                    window_->draw(sprite_);
                    window_->draw(rectangle);
                    window_->display();
                }
                else end.x = -1;
            }
        }

        window_->draw(sprite_);

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
    float releft  = borders_.Re_left;
    float reright = borders_.Re_right;
    float imup    = borders_.Im_up;
    float imdown  = borders_.Im_down;

    if (newscreen.zoom > 1)
    {
        borders_.Re_left  = releft + (reright - releft) * newscreen.x1 / winsizes_.x;
        borders_.Re_right = releft + (reright - releft) * newscreen.x2 / winsizes_.x;
        borders_.Im_down  = imup   - (imup    - imdown) * newscreen.y2 / winsizes_.y;

        borders_.Im_up = borders_.Im_down + (borders_.Re_right - borders_.Re_left) * winsizes_.y / winsizes_.x;
    }
    else
    {
        borders_.Re_left  = releft  - (reright - releft) *                newscreen.x1  / (newscreen.x2 - newscreen.x1);
        borders_.Re_right = reright + (reright - releft) * (winsizes_.x - newscreen.x2) / (newscreen.x2 - newscreen.x1);
        borders_.Im_down  = imdown  - (imup    - imdown) * (winsizes_.y - newscreen.y2) / (newscreen.y2 - newscreen.y1);

        borders_.Im_up = borders_.Im_down + (borders_.Re_right - borders_.Re_left) * winsizes_.y / winsizes_.x;
    }
}

//------------------------------------------------------------------------------

void Puzabrot::initCalculator (Calculator& calc, float x, float y, float cx, float cy)
{
    calc.trees_[0] = expr_trees_[0];
    calc.trees_[1] = expr_trees_[1];

    switch (input_mode_)
    {
    case Z_INPUT:
    {
        calc.variables_.Push({ { cx, cy }, "c" });
        calc.variables_.Push({ {  x,  y }, "z" });
        break;
    }
    case XY_INPUT:
    {
        calc.variables_.Push({ { cx, 0 }, "cx" });
        calc.variables_.Push({ { cy, 0 }, "cy" });

        calc.variables_.Push({ {  x, 0 }, "x" });
        calc.variables_.Push({ {  y, 0 }, "y" });
        break;
    }
    }
}

//------------------------------------------------------------------------------

void Puzabrot::Mapping (Calculator& calc, float& mapped_x, float& mapped_y)
{
    switch (input_mode_)
    {
    case Z_INPUT:
    {
        calc.Calculate(calc.trees_[0].root_, false);
        calc.variables_[calc.variables_.getSize() - 1] = { calc.trees_[0].root_->getData().number, "z" };

        mapped_x = real(calc.trees_[0].root_->getData().number);
        mapped_y = imag(calc.trees_[0].root_->getData().number);
        break;
    }
    case XY_INPUT:
    {
        calc.Calculate(calc.trees_[0].root_, false);
        calc.Calculate(calc.trees_[1].root_, false);

        calc.variables_[calc.variables_.getSize() - 1] = { calc.trees_[0].root_->getData().number, "x" };
        calc.variables_[calc.variables_.getSize() - 2] = { calc.trees_[1].root_->getData().number, "y" };

        mapped_x = real(calc.trees_[0].root_->getData().number);
        mapped_y = real(calc.trees_[1].root_->getData().number);
        break;
    }
    }
}

//------------------------------------------------------------------------------

sf::Vector2f Puzabrot::PointTrace (sf::Vector2f point, sf::Vector2f c_point)
{
    float re0 = point.x;
    float im0 = point.y;

    float x1 = 0;
    float y1 = 0;
    
    switch (draw_mode_)
    {
    case MAIN:
    {
        x1 = c_point.x;
        y1 = c_point.y;
        break;
    }
    case JULIA:
    {
        x1 = julia_point_.x;
        y1 = julia_point_.y;
        break;
    }
    }

    float x2 = 0;
    float y2 = 0;

    static Calculator calc;
    initCalculator(calc, re0, im0, x1, y1);

    for (int i = 0; (i < itrn_max_) && (sqrt(x2 * x2 + y2 * y2) < lim_); ++i)
    {
        Mapping(calc, x2, y2);

        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f((x1 - borders_.Re_left) / (borders_.Re_right - borders_.Re_left) * winsizes_.x,
                                    (borders_.Im_up - y1)   / (borders_.Im_up    - borders_.Im_down) * winsizes_.y), sf::Color::White),

            sf::Vertex(sf::Vector2f((x2 - borders_.Re_left) / (borders_.Re_right - borders_.Re_left) * winsizes_.x,
                                    (borders_.Im_up - y2)   / (borders_.Im_up    - borders_.Im_down) * winsizes_.y), sf::Color::Black)
        };

        x1 = x2;
        y1 = y2;

        window_->draw(line, 2, sf::Lines);
        ++i;

        switch (input_mode_)
        {
        case Z_INPUT:
        {
            if (abs(calc.trees_[0].root_->getData().number) < lim_)
                break;
            break;
        }
        case XY_INPUT:
        {
            if (abs(std::complex<double>(real(calc.trees_[0].root_->getData().number), real(calc.trees_[1].root_->getData().number))) < lim_)
                break;
            break;
        }
        }
    }

    calc.variables_.Clean();
    ADD_VAR(calc.variables_);

    return sf::Vector2f(x1, y1);
}

//------------------------------------------------------------------------------

void Puzabrot::savePicture ()
{
    static int shot_num = 0;
    char filename[256] = "screenshot";
    char shot_num_str[13] = "";
    sprintf(shot_num_str, "%d", shot_num++);

    strcat(filename, "(");
    strcat(filename, shot_num_str);
    strcat(filename, ")");
    strcat(filename, ".png");

    window_->draw(sprite_);

    sf::RectangleShape rectangle;
    rectangle.setPosition(0, 0);
    rectangle.setSize(sf::Vector2f(winsizes_));
    rectangle.setFillColor(sf::Color(10, 10, 10, 150));

    window_->draw(rectangle);
    window_->display();

    sf::Vector2u screenshot_sizes(SCREENSHOT_WIDTH, (float)SCREENSHOT_WIDTH / winsizes_.x * winsizes_.y);

    shader_.setUniform("borders",  sf::Glsl::Vec4(borders_.Re_left, borders_.Re_right, borders_.Im_down, borders_.Im_up));
    shader_.setUniform("winsizes", sf::Glsl::Ivec2(screenshot_sizes.x, screenshot_sizes.y));

    shader_.setUniform("itrn_max", (int)itrn_max_);
    shader_.setUniform("limit",    lim_);

    shader_.setUniform("drawing_mode", draw_mode_);
    shader_.setUniform("coloring",     coloring_);

    shader_.setUniform("julia_point", sf::Glsl::Vec2(julia_point_.x, julia_point_.y));

    sf::RenderTexture render_texture;
    render_texture.create(screenshot_sizes.x, screenshot_sizes.y);

    sf::Sprite sprite(render_texture.getTexture());

    render_texture.draw(sprite, &shader_);
    render_texture.display();

    sf::Texture screen = render_texture.getTexture();

    screen.copyToImage().saveToFile(filename);

    window_->draw(sprite_);
    window_->display();
}

//------------------------------------------------------------------------------

void Puzabrot::drawHelpMenu ()
{
    sf::RectangleShape dimRect(sf::Vector2f((float)winsizes_.x, (float)winsizes_.y));
    dimRect.setFillColor(sf::Color(0, 0, 0, 128));
    window_->draw(dimRect);
    
    sf::Font font;
    font.loadFromMemory(consola_ttf, consola_ttf_len);

    sf::Text helpMenu;
    helpMenu.setFont(font);
    helpMenu.setCharacterSize(22);
    helpMenu.setPosition(10.0f, 10.0f);
    helpMenu.setFillColor(sf::Color::White);

    char str[1000] = "";

    sprintf(str, 
        "    H - Toggle help menu viewing\n"
        "    Z - Choose zooming mode       (draw a rectangle by left and right mouse button to zoom in-out)\n"
        "    P - Choose point tracing mode (press left mouse button to trace point)\n"
        "    S - Choose sounding mode      (press left mouse button to trace point and hear sound)\n"
        "  F11 - Toggle Fullscreen\n"
        "  Esc - Exit program\n"
        "Space - Take a screenshot\n"
        "    R - Reset View\n"
        "Tilde - Open input box (enter text expression, then press enter to output the set)\n"
        "  Tab - Change input method\n"
        "    D - Toggle audio dampening\n"
        "    C - Toggle sound coloring\n"
        "    J - Hold down, move mouse, and release to make Julia sets. Press again to switch back\n"
        "    I - Hold down, scroll mouse wheel to increase or decrease max iterations\n"
        "    L - Hold down, scroll mouse wheel to increase or decrease limit\n"
        "\n"
        "\n"
        "        Current max iteration: %lu, current limit: %f\n"
        "        Borders: upper: %.5lf, bottom: %.5lf, left: %.5lf, right: %.5lf\n", itrn_max_, lim_, borders_.Im_up, borders_.Im_down, borders_.Re_left, borders_.Re_right);

    helpMenu.setString(str);
    window_->draw(helpMenu);
}

//------------------------------------------------------------------------------

int Puzabrot::makeShader ()
{
    char* expr1 = new char[MAX_STR_LEN] {};
    char* expr2 = new char[MAX_STR_LEN] {};

    Expression expression1 = { expr1, expr1, CALC_OK };
    Expression expression2 = { expr2, expr2, CALC_OK };

    Tree<CalcNodeData> test_tree((char*)"Test tree");

    switch (input_mode_)
    {
    case Z_INPUT:
    {
        std::string string = input_box_z_.getInput().toAnsiString();
        char* str = (char*)string.c_str();

        strcpy(expr1, str);
        strcpy(expr2, str);

        test_tree.Clean();
        int err = Expr2Tree(expression1, test_tree);
        if (err) return expression1.err;

        expr_trees_[0] = test_tree;
        expr_trees_[0].name_ = (char*)"Expression tree 1";

        Expr2Tree(expression2, expr_trees_[1]);
        break;
    }
    case XY_INPUT:
    {
        std::string string1 = input_box_x_.getInput().toAnsiString();
        std::string string2 = input_box_y_.getInput().toAnsiString();

        char* str1 = (char*)string1.c_str();
        char* str2 = (char*)string2.c_str();

        strcpy(expr1, str1);
        strcpy(expr2, str2);

        test_tree.Clean();
        int err = Expr2Tree(expression1, test_tree);
        if (err) return expression1.err;

        expr_trees_[0] = test_tree;
        expr_trees_[0].name_ = (char*)"Expression tree 1";

        test_tree.Clean();
        err = Expr2Tree(expression2, test_tree);
        if (err) return expression2.err;

        expr_trees_[1] = test_tree;
        expr_trees_[1].name_ = (char*)"Expression tree 2";
        break;
    }
    }

    char* str_shader = writeShader();
    if (str_shader == nullptr)
        return CALC_WRONG_VARIABLE;

    shader_.loadFromMemory(str_shader, sf::Shader::Fragment);

    delete [] str_shader;
    delete [] expr1;
    delete [] expr2;

    return 0;
}

//------------------------------------------------------------------------------

char* Puzabrot::writeShader ()
{
    char* str_initialization = writeInitialization();

    char* str_calculation = writeCalculation();
    if (str_calculation == nullptr) return nullptr;

    char* str_checking = writeChecking();

    char* str_shader = new char[10000] {};

    sprintf(str_shader,
        "#version 130\n"
        "\n"
        "const float NIL = 1e-9;\n"
        "const float PI  = atan(1.0) * 4;\n"
        "const float E   = exp(1.0);\n"
        "const vec2  I   = vec2(0, 1);\n"
        "const vec2  ONE = vec2(1, 0);\n"
        "\n"
        "uniform vec4  borders;\n"
        "uniform ivec2 winsizes;\n"
        "uniform int   itrn_max;\n"
        "uniform float limit;\n"
        "uniform int   drawing_mode;\n"
        "uniform bool  coloring;\n"
        "uniform vec2  julia_point;\n"
        "\n"
        "vec2 conj(vec2 a)\n"
        "{\n"
        "    return vec2(a.x, -a.y);\n"
        "}\n"
        "\n"
        "float norm(vec2 a)\n"
        "{\n"
        "    return a.x * a.x + a.y * a.y;\n"
        "}\n"
        "\n"
        "float cabs(vec2 a)\n"
        "{\n"
        "    return sqrt(a.x * a.x + a.y * a.y);\n"
        "}\n"
        "\n"
        "float arg(vec2 a)\n"
        "{\n"
        "    return atan(a.y, a.x);\n"
        "}\n"
        "\n"
        "vec2 cadd(vec2 a, vec2 b)\n"
        "{\n"
        "    return vec2(a.x + b.x, a.y + b.y);\n"
        "}\n"
        "\n"
        "vec2 csub(vec2 a, vec2 b)\n"
        "{\n"
        "    return vec2(a.x - b.x, a.y - b.y);\n"
        "}\n"
        "\n"
        "vec2 cmul(vec2 a, vec2 b)\n"
        "{\n"
        "    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);\n"
        "}\n"
        "\n"
        "vec2 cdiv(vec2 a, vec2 b)\n"
        "{\n"
        "    vec2 top = cmul(a, conj(b));\n"
        "    float bottom = norm(b);\n"
        "    return vec2(top.x / bottom, top.y / bottom);\n"
        "}\n"
        "\n"
        "vec2 cln(vec2 a)\n"
        "{\n"
        "    if ((a.x < 0) && (abs(a.y) <= NIL))\n"
        "        return vec2(log(a.x * a.x + a.y * a.y)/2, PI);\n"
        "    else\n"
        "        return vec2(log(a.x * a.x + a.y * a.y)/2, arg(a));\n"
        "}\n"
        "\n"
        "vec2 clg(vec2 a)\n"
        "{\n"
        "    vec2 ln = cln(a);\n"
        "    return vec2(ln.x/log(10.0), ln.y/log(10.0));\n"
        "}\n"
        "\n"
        "vec2 cexp(vec2 a)\n"
        "{\n"
        "    float e = exp(a.x);\n"
        "    return vec2(e * cos(a.y), e * sin(a.y));\n"
        "}\n"
        "\n"
        "vec2 cpow(vec2 a, vec2 b)\n"
        "{\n"
        "    return cexp(cmul(b, cln(a)));\n"
        "}\n"
        "\n"
        "vec2 csqrt(vec2 a)\n"
        "{\n"
        "    return cpow(a, vec2(0.5, 0));\n"
        "}\n"
        "\n"
        "vec2 csin(vec2 a)\n"
        "{\n"
        "    return vec2(sin(a.x) * cosh(a.y), cos(a.x) * sinh(a.y));\n"
        "}\n"
        "\n"
        "vec2 ccos(vec2 a)\n"
        "{\n"
        "    return vec2(cos(a.x) * cosh(a.y), -sin(a.x) * sinh(a.y));\n"
        "}\n"
        "\n"
        "vec2 ctan(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2, a.y * 2);\n"
        "    float bottom = cos(a_2.x) + cosh(a_2.y);\n"
        "    return vec2(sin(a_2.x) / bottom, sinh(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 ccot(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2, a.y * 2);\n"
        "    float bottom = cos(a_2.x) - cosh(a_2.y);\n"
        "    return vec2(-sin(a_2.x) / bottom, sinh(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 carcsin(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0, -1), cln(cadd(cmul(I, a), csqrt(csub(ONE, cmul(a, a))))));\n"
        "}\n"
        "\n"
        "vec2 carccos(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0, -1), cln(cadd(a, csqrt(csub(cmul(a, a), ONE)))));\n"
        "}\n"
        "\n"
        "vec2 carctan(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0, 0.5), csub(cln(cadd(I, a)), cln(csub(I, a))));\n"
        "}\n"
        "\n"
        "vec2 carccot(vec2 a)\n"
        "{\n"
        "    return csub(vec2(PI/2, 0), cmul(vec2(0, 0.5), csub(cln(cadd(I, a)), cln(csub(I, a)))));\n"
        "}\n"
        "\n"
        "vec2 csinh(vec2 a)\n"
        "{\n"
        "    return vec2(sinh(a.x) * cos(a.y), cosh(a.x) * sin(a.y));\n"
        "}\n"
        "\n"
        "vec2 ccosh(vec2 a)\n"
        "{\n"
        "    return vec2(cosh(a.x) * cos(a.y), sinh(a.x) * sin(a.y));\n"
        "}\n"
        "\n"
        "vec2 ctanh(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2, a.y * 2);\n"
        "    float bottom = cosh(a_2.x) + cos(a_2.y);\n"
        "    return vec2(sinh(a_2.x) / bottom, sin(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 ccoth(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2, a.y * 2);\n"
        "    float bottom = cos(a_2.y) - cosh(a_2.x);\n"
        "    return vec2(-sinh(a_2.x) / bottom, sin(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 carcsinh(vec2 a)\n"
        "{\n"
        "    return cln(cadd(a, csqrt(cadd(cmul(a, a), ONE))));\n"
        "}\n"
        "\n"
        "vec2 carccosh(vec2 a)\n"
        "{\n"
        "    return cln(cadd(a, csqrt(csub(cmul(a, a), ONE))));\n"
        "}\n"
        "\n"
        "vec2 carctanh(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.5, 0), csub(cln(cadd(ONE, a)), cln(csub(ONE, a))));\n"
        "}\n"
        "\n"
        "vec2 carccoth(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.5, 0), csub(cln(cadd(ONE, a)), cln(csub(ONE, a))));\n"
        "}\n"
        "\n"
        "vec3 getColor(int itrn, vec3 sumz)\n"
        "{\n"
        "    if (itrn < itrn_max)\n"
        "    {\n"
        "        itrn = itrn * 4 % 1530;\n"
        "             if (itrn < 256)  return vec3( 255,         itrn,        0           ) / 255 * (1.0 - float(coloring)*0.85);\n"
        "        else if (itrn < 511)  return vec3( 510 - itrn,  255,         0           ) / 255 * (1.0 - float(coloring)*0.85);\n"
        "        else if (itrn < 766)  return vec3( 0,           255,         itrn - 510  ) / 255 * (1.0 - float(coloring)*0.85);\n"
        "        else if (itrn < 1021) return vec3( 0,           1020 - itrn, 255         ) / 255 * (1.0 - float(coloring)*0.85);\n"
        "        else if (itrn < 1276) return vec3( itrn - 1020, 0,           255         ) / 255 * (1.0 - float(coloring)*0.85);\n"
        "        else if (itrn < 1530) return vec3( 255,         0,           1529 - itrn ) / 255 * (1.0 - float(coloring)*0.85);\n"
        "    }\n"
        "    else if (coloring) return sin(abs(abs(sumz) / itrn_max * 5.0)) * 0.45 + 0.5;\n"
        "    else return vec3( 0, 0, 0 );\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    float re0 = borders.x + (borders.y - borders.x) * gl_FragCoord.x / winsizes.x;\n"
        "    float im0 = borders.w - (borders.w - borders.z) * gl_FragCoord.y / winsizes.y;\n"
        "\n"
        "    %s\n"
        "\n"
        "    vec3 sumz = vec3(0.0, 0.0, 0.0);\n"
        "    int itrn  = 0;\n"
        "    for (itrn = 0; itrn < itrn_max; ++itrn)\n"
        "    {\n"
        "        %s\n"
        "        \n"
        "        %s\n"
        "    }\n"
        "\n"
        "    vec3 col = getColor(itrn, sumz);\n"
        "    gl_FragColor = vec4(col, 1.0);\n"
       "}", str_initialization, str_calculation, str_checking);

    delete [] str_initialization;
    delete [] str_calculation;
    delete [] str_checking;

    return str_shader;
}

//------------------------------------------------------------------------------

char* Puzabrot::writeInitialization ()
{
    switch (input_mode_)
    {
    case Z_INPUT:
    {
        char* str_initialization = new char[1000] {};

        sprintf(str_initialization,
            "vec2 z = vec2(re0, im0);\n"
            "vec2 pz = z;\n"
            "vec2 c = vec2(0, 0);\n"
            "if (drawing_mode == 0)\n"
            "    c = vec2(re0, im0);\n"
            "else if (drawing_mode == 1)\n"
            "    c = vec2(julia_point.x, julia_point.y);");

        return str_initialization;
    }
    case XY_INPUT:
    {
        char* str_initialization = new char[1000] {};

        sprintf(str_initialization,
            "float x = re0;\n"
            "float y = im0;\n"
            "vec2 pz = vec2(x, y);\n"
            "float cx = 0;\n"
            "float cy = 0;\n"
            "if (drawing_mode == 0)\n"
            "{\n"
            "    cx = re0;\n"
            "    cy = im0;\n"
            "}\n"
            "else if (drawing_mode == 1)\n"
            "{\n"
            "    cx = julia_point.x;\n"
            "    cy = julia_point.y;\n"
            "}");

        return str_initialization;
    }
    default: return nullptr;
    }
}

//------------------------------------------------------------------------------

char* Puzabrot::writeCalculation ()
{
    switch (input_mode_)
    {
    case Z_INPUT:
    {
        char* str_calculation = new char[1000] {};
        sprintf(str_calculation,
            "vec2 ppz = pz;\n"
            "pz = z;\n");

        sprintf(str_calculation + strlen(str_calculation), "z = ");

        int err = Tree2GLSL(expr_trees_[0].root_, str_calculation + strlen(str_calculation));
        if (err)
        {
            delete [] str_calculation;
            return nullptr;
        }

        sprintf(str_calculation + strlen(str_calculation), ";");

        return str_calculation;
    }
    case XY_INPUT:
    {
        char* str_calculation = new char[1000] {};
        sprintf(str_calculation,
            "vec2 ppz = pz;\n"
            "pz = vec2(x, y);\n");

        sprintf(str_calculation + strlen(str_calculation), "vec2 x1 = ");

        int err = Tree2GLSL(expr_trees_[0].root_, str_calculation + strlen(str_calculation));
        if (err)
        {
            delete [] str_calculation;
            return nullptr;
        }

        sprintf(str_calculation + strlen(str_calculation), ";\nvec2 y1 = ");

        err = Tree2GLSL(expr_trees_[1].root_, str_calculation + strlen(str_calculation));
        if (err)
        {
            delete[] str_calculation;
            return nullptr;
        }

        sprintf(str_calculation + strlen(str_calculation), ";\n");

        sprintf(str_calculation + strlen(str_calculation), "x = x1.x;\ny = y1.x;");

        return str_calculation;
    }
    default: return nullptr;
    }
}

//------------------------------------------------------------------------------

char* Puzabrot::writeChecking ()
{
    char* str_checking = new char[1000] {};

    switch (input_mode_)
    {
    case Z_INPUT:  sprintf(str_checking, "if (cabs(z) > limit) break;\n");          break;
    case XY_INPUT: sprintf(str_checking, "if (cabs(vec2(x, y)) > limit) break;\nvec2 z = vec2(x, y);\n"); break;
    default: return nullptr;
    }

    sprintf(str_checking + strlen(str_checking),
        "sumz.x += dot(z - pz, pz - ppz);\n"
        "sumz.y += dot(z - pz,  z - pz);\n"
        "sumz.z += dot(z - ppz, z - ppz);");

    return str_checking;
}

//------------------------------------------------------------------------------

int Puzabrot::Tree2GLSL (Node<CalcNodeData>* node_cur, char* str_cur)
{
    if (node_cur == nullptr)
    {
        sprintf(str_cur, "vec2(0, 0)");
        return 0;
    }

    switch (node_cur->getData().node_type)
    {
    case NODE_FUNCTION:
    {
        sprintf(str_cur, "c%s(", node_cur->getData().word);

        int err = Tree2GLSL(node_cur->right_, str_cur + strlen(str_cur));
        if (err) return err;
        sprintf(str_cur + strlen(str_cur), ")");

        break;
    }
    case NODE_OPERATOR:
    {
        switch (node_cur->getData().op_code)
        {
        case OP_ADD: sprintf(str_cur, "cadd("); break;
        case OP_SUB: sprintf(str_cur, "csub("); break;
        case OP_MUL: sprintf(str_cur, "cmul("); break;
        case OP_DIV: sprintf(str_cur, "cdiv("); break;
        case OP_POW: sprintf(str_cur, "cpow("); break;
        default: assert(0);
        }

        int err = Tree2GLSL(node_cur->left_, str_cur + strlen(str_cur));
        if (err) return err;

        sprintf(str_cur + strlen(str_cur), ", ");

        err = Tree2GLSL(node_cur->right_, str_cur + strlen(str_cur));
        if (err) return err;

        sprintf(str_cur + strlen(str_cur), ")");
        
        break;
    }
    case NODE_VARIABLE:
    {
        switch (input_mode_)
        {
        case Z_INPUT:
        {
            if ((strcmp(node_cur->getData().word, "z") != 0) &&
                (strcmp(node_cur->getData().word, "c") != 0) &&
                (strcmp(node_cur->getData().word, "i") != 0))
                return -1;

            break;
        }
        case XY_INPUT:
        {
            if ((strcmp(node_cur->getData().word, "x")  != 0) &&
                (strcmp(node_cur->getData().word, "y")  != 0) &&
                (strcmp(node_cur->getData().word, "cx") != 0) &&
                (strcmp(node_cur->getData().word, "cy") != 0) &&
                (strcmp(node_cur->getData().word, "i")  != 0))
                return -1;

            break;
        }
        }

        if (strcmp(node_cur->getData().word, "i") == 0)
            sprintf(str_cur, "I");
        else
            switch (input_mode_)
            {
            case Z_INPUT:  sprintf(str_cur, "%s",          node_cur->getData().word); break;
            case XY_INPUT: sprintf(str_cur, "vec2(%s, 0)", node_cur->getData().word); break;
            }

        break;
    }
    case NODE_NUMBER:
    {
        switch (input_mode_)
        {
        case Z_INPUT:  sprintf(str_cur, "vec2(%f, %f)", real(node_cur->getData().number), imag(node_cur->getData().number)); break;
        case XY_INPUT: sprintf(str_cur, "vec2(%f,  0)", real(node_cur->getData().number)); break;
        }

        break;
    }
    default: assert(0);
    }

    return 0;
}

//------------------------------------------------------------------------------

Synth::Synth (Puzabrot* puza) :
    puza_        (puza),
    audio_reset_ (true),
    audio_pause_ (false),
    sustain_     (true),
    volume_      (8000.0),
    point_       (sf::Vector2f(0, 0)),
    c_point_     (sf::Vector2f(0, 0)),
    new_point_   (sf::Vector2f(0, 0)),
    prev_point_  (sf::Vector2f(0, 0))
{
    initialize(2, SAMPLE_RATE);
    setLoop(true);

    updateCalc();
}

//------------------------------------------------------------------------------

void Synth::updateCalc ()
{
    calc_.variables_.Clean();
    ADD_VAR(calc_.variables_);
    puza_->initCalculator(calc_, point_.x, point_.y, c_point_.x, c_point_.y);
}

//------------------------------------------------------------------------------

void Synth::SetPoint (sf::Vector2f point)
{
    new_point_ = point;

    audio_reset_ = true;
    audio_pause_ = false;
}

//------------------------------------------------------------------------------

bool Synth::onGetData (Chunk& data)
{
    data.samples = m_samples;
    data.sampleCount = AUDIO_BUFF_SIZE;
    memset(m_samples, 0, AUDIO_BUFF_SIZE);

    if (audio_reset_)
    {
        m_audio_time = 0;

        switch (puza_->draw_mode_)
        {
        case MAIN:  c_point_ = new_point_;          break;
        case JULIA: c_point_ = puza_->julia_point_; break;
        }

        point_ = new_point_;
        prev_point_ = new_point_;

        mean_x = new_point_.x;
        mean_y = new_point_.y;
        volume_ = 8000.0;


        audio_reset_ = false;
    }

    if (audio_pause_) return true;

    const int steps = SAMPLE_RATE / MAX_FREQ;
    for (int i = 0; i < AUDIO_BUFF_SIZE; i += 2)
    {
        const int j = m_audio_time % steps;
        if (j == 0)
        {
            prev_point_ = point_;

            updateCalc();
            puza_->Mapping(calc_, point_.x, point_.y);

            if (sqrt(point_.x * point_.x + point_.y * point_.y) > puza_->lim_)
            {
                audio_pause_ = true;
                return true;
            }

            dpx = prev_point_.x - c_point_.x;
            dpy = prev_point_.y - c_point_.y;
            dx  = point_.x - c_point_.x;
            dy  = point_.y - c_point_.y;

            if (dx != 0.0 || dy != 0.0)
            {
                double dpmag = 1.0 / std::sqrt(1e-12 + dpx * dpx + dpy * dpy);
                double dmag  = 1.0 / std::sqrt(1e-12 + dx * dx + dy * dy);

                dpx *= dpmag;
                dpy *= dpmag;
                dx  *= dmag;
                dy  *= dmag;
            }

            double m = dx * dx + dy * dy;
            if (m > 2.0)
            {
                dx *= 2.0 / m;
                dy *= 2.0 / m;
            }

            m = dpx * dpx + dpy * dpy;
            if (m > 2.0)
            {
                dpx *= 2.0 / m;
                dpy *= 2.0 / m;
            }

            if (!sustain_)
            {
                volume_ *= 0.9992;
            }
        }

        double t = 0.5 - 0.5 * cos(double(j) / double(steps) * real(PI));

        double wx = t * dx + (1.0 - t) * dpx;
        double wy = t * dy + (1.0 - t) * dpy;

        m_samples[i]     = (int16_t)std::min(std::max(wx * volume_, -32000.0), 32000.0);
        m_samples[i + 1] = (int16_t)std::min(std::max(wy * volume_, -32000.0), 32000.0);

        m_audio_time += 1;
    }

    return !audio_reset_;
}

//------------------------------------------------------------------------------
