/*------------------------------------------------------------------------------
 * File:        Puzabrot.cpp                                                *
 * Description: Puzabrot implementation                                     *
 * Created:     30 jun 2021                                                 *
 * Author:      Artem Puzankov                                              *
 * Email:       puzankov.ao@phystech.edu                                    *
 * GitHub:      https://github.com/hellopuza                                *
 * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
 *///------------------------------------------------------------------------

#include "Puzabrot.h"

#include "Resource/ConsolaFont.hpp"

#include <cassert>
#include <cstring>

namespace puza {

ComplexFrame::ComplexFrame (double re_left_, double re_right_, double im_bottom_, double im_top_) :
    re_left (re_left_), re_right (re_right_), im_bottom (im_bottom_), im_top (im_top_)
{
}

Puzabrot::Puzabrot () :
    winsizes_ ({ DEFAULT_WIDTH, DEFAULT_HEIGHT }), window_ (sf::VideoMode (winsizes_.x, winsizes_.y), TITLE_STRING),
    input_box_x_ (sf::Vector2f (10, 10), sf::Color (128, 128, 128, 128), sf::Color::White, 20.0F),
    input_box_y_ (sf::Vector2f (10, 50), sf::Color (128, 128, 128, 128), sf::Color::White, 20.0F),
    input_box_z_ (sf::Vector2f (10, 10), sf::Color (128, 128, 128, 128), sf::Color::White, 20.0F)
{
    borders_.im_top = UPPER_BORDER;
    borders_.im_bottom = -UPPER_BORDER;

    borders_.re_left = -(borders_.im_top - borders_.im_bottom) * winsizes_.x / winsizes_.y / 5 * 3;
    borders_.re_right = (borders_.im_top - borders_.im_bottom) * winsizes_.x / winsizes_.y / 5 * 2;

    expr_trees_[0] = Tree<CalcData> ();
    expr_trees_[1] = Tree<CalcData> ();

    input_box_x_.setLabel (sf::String ("x:"));
    input_box_y_.setLabel (sf::String ("y:"));
    input_box_z_.setLabel (sf::String ("z:"));

    input_box_x_.setInput (sf::String ("x*x-y*y+cx"));
    input_box_y_.setInput (sf::String ("2*x*y+cy"));

    input_box_z_.setInput (sf::String ("z^2+c"));

    render_texture_.create (winsizes_.x, winsizes_.y);
    sprite_ = sf::Sprite (render_texture_.getTexture ());
}

void Puzabrot::run ()
{
    window_.setVerticalSyncEnabled (true);

    int action_mode = POINT_TRACING;

    makeShader ();
    DrawSet ();

    bool showing_menu = false;
    bool showing_trace = false;
    bool julia_dragging = false;
    bool left_pressed = false;
    bool change_iter = false;
    bool change_limit = false;

    sf::Vector2<double> orbit (0.0, 0.0);
    sf::Vector2<double> c_point (0.0, 0.0);

    Synth synth (this);

    while (window_.isOpen ())
    {
        sf::Event event;
        Screen newscreen;
        bool was_screenshot = false;
        while (window_.pollEvent (event))
        {
            // Close window
            if ((event.type == sf::Event::Closed) ||
                ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window_.close ();
                return;
            }

            // Toggle fullscreen
            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F11))
            {
                toggleFullScreen ();
                DrawSet ();
            }

            // Resize window
            else if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visible_area (0.0F, 0.0F, static_cast<float> (event.size.width),
                                            static_cast<float> (event.size.height));
                window_.setView (sf::View (visible_area));
                updateWinSizes (window_.getSize ().x, window_.getSize ().y);

                DrawSet ();
            }

            // Reset set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R) &&
                     (!InputBoxesHasFocus ()))
            {
                borders_.im_top = UPPER_BORDER;
                borders_.im_bottom = -UPPER_BORDER;

                borders_.re_left = -(borders_.im_top - borders_.im_bottom) * winsizes_.x / winsizes_.y / 5 * 3;
                borders_.re_right = (borders_.im_top - borders_.im_bottom) * winsizes_.x / winsizes_.y / 5 * 2;

                itrn_max_ = MAX_ITERATION;
                lim_ = LIMIT;

                draw_mode_ = MAIN;
                DrawSet ();
            }

            // Take a screenshot
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space) &&
                     (!was_screenshot) && (!InputBoxesHasFocus ()))
            {
                savePicture ();
                was_screenshot = true;
            }

            // Toggle audio dampening
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::D) &&
                     (!InputBoxesHasFocus ()))
            {
                synth.sustain_ = !synth.sustain_;
            }

            // Toggle sound coloring
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::C) &&
                     (!InputBoxesHasFocus ()))
            {
                coloring_ = !coloring_;

                DrawSet ();
            }

            // Toggle help menu showing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::H) &&
                     (!InputBoxesHasFocus ()))
            {
                showing_menu = !showing_menu;
                if (showing_menu)
                {
                    input_box_x_.is_visible_ = false;
                    input_box_y_.is_visible_ = false;
                    input_box_z_.is_visible_ = false;
                }
            }

            // Toggle input mode
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tab) &&
                     (!InputBoxesHasFocus ()))
            {
                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    input_box_x_.has_focus_ = false;
                    input_box_x_.is_visible_ = input_box_z_.is_visible_;

                    input_box_y_.has_focus_ = false;
                    input_box_y_.is_visible_ = input_box_z_.is_visible_;

                    input_box_z_.has_focus_ = false;
                    input_box_z_.is_visible_ = false;

                    input_mode_ = XY_INPUT;
                    break;
                }
                case XY_INPUT:
                {
                    input_box_z_.has_focus_ = false;
                    input_box_z_.is_visible_ = input_box_x_.is_visible_;

                    input_box_x_.has_focus_ = false;
                    input_box_x_.is_visible_ = false;

                    input_box_y_.has_focus_ = false;
                    input_box_y_.is_visible_ = false;

                    input_mode_ = Z_INPUT;
                    break;
                }
                }
            }

            // Toggle input box visibility
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tilde) &&
                     (!InputBoxesHasFocus ()))
            {
                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    input_box_z_.is_visible_ = !input_box_z_.is_visible_;
                    if (!input_box_z_.is_visible_)
                        input_box_z_.has_focus_ = false;
                    break;
                }
                case XY_INPUT:
                {
                    input_box_x_.is_visible_ = !input_box_x_.is_visible_;
                    input_box_y_.is_visible_ = !input_box_y_.is_visible_;

                    if (!input_box_x_.is_visible_)
                    {
                        input_box_x_.has_focus_ = false;
                        input_box_y_.has_focus_ = false;
                    }
                    break;
                }
                }
            }

            // Toggle input box focus
            else if (InputBoxesIsVisible () && (event.type == sf::Event::MouseButtonPressed) &&
                     (event.mouseButton.button == sf::Mouse::Left))
            {
                sf::Vector2f mouse_button (static_cast<float> (event.mouseButton.x),
                                           static_cast<float> (event.mouseButton.y));
                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    input_box_z_.has_focus_ =
                        ((input_box_z_.getPosition ().x < mouse_button.x) &&
                         (mouse_button.x < input_box_z_.getPosition ().x + input_box_z_.getSize ().x) &&
                         (input_box_z_.getPosition ().y < mouse_button.y) &&
                         (mouse_button.y < input_box_z_.getPosition ().y + input_box_z_.getSize ().y));
                    break;
                }
                case XY_INPUT:
                {
                    input_box_x_.has_focus_ =
                        ((input_box_x_.getPosition ().x < mouse_button.x) &&
                         (mouse_button.x < input_box_x_.getPosition ().x + input_box_x_.getSize ().x) &&
                         (input_box_x_.getPosition ().y < mouse_button.y) &&
                         (mouse_button.y < input_box_x_.getPosition ().y + input_box_x_.getSize ().y));

                    input_box_y_.has_focus_ =
                        ((input_box_y_.getPosition ().x < mouse_button.x) &&
                         (mouse_button.x < input_box_y_.getPosition ().x + input_box_y_.getSize ().x) &&
                         (input_box_y_.getPosition ().y < mouse_button.y) &&
                         (mouse_button.y < input_box_y_.getPosition ().y + input_box_y_.getSize ().y));
                    break;
                }
                }
            }

            // Input text expression to input box
            else if (InputBoxesHasFocus () && (event.type == sf::Event::TextEntered) && (event.text.unicode < 128))
            {
                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    input_box_z_.setInput (event.text.unicode);
                    break;
                }
                case XY_INPUT:
                {
                    if (input_box_x_.has_focus_)
                        input_box_x_.setInput (event.text.unicode);
                    else
                        input_box_y_.setInput (event.text.unicode);
                    break;
                }
                }
            }

            // Enter expression from input box
            else if (InputBoxesHasFocus () && (event.type == sf::Event::KeyPressed) &&
                     (event.key.code == sf::Keyboard::Enter))
            {
                int err = makeShader ();

                if (!err)
                {
                    DrawSet ();
                    draw_mode_ = MAIN;
                }

                switch (input_mode_)
                {
                case Z_INPUT:
                {
                    if (err)
                        input_box_z_.setOutput (sf::String (calc_errstr[err + 1]));
                    else
                        input_box_z_.setOutput (sf::String ());
                    break;
                }
                case XY_INPUT:
                {
                    if (input_box_x_.has_focus_)
                        if (err)
                        {
                            input_box_x_.setOutput (sf::String (calc_errstr[err + 1]));
                            input_box_y_.setPosition (
                                sf::Vector2f (input_box_y_.getPosition ().x,
                                              input_box_y_.getPosition ().y + 0.5F * input_box_x_.getSize ().y));
                        }
                        else
                        {
                            input_box_x_.setOutput (sf::String ());
                            input_box_y_.setPosition (sf::Vector2f (10.0F, 50.0F));
                        }
                    else if (err)
                        input_box_y_.setOutput (sf::String (calc_errstr[err + 1]));
                    else
                        input_box_y_.setOutput (sf::String ());
                    break;
                }
                }
            }

            // Julia set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::J) &&
                     (!InputBoxesHasFocus ()))
            {
                if (draw_mode_ != JULIA)
                {
                    while (sf::Keyboard::isKeyPressed (sf::Keyboard::J))
                    {
                        julia_point_ = Screen2Plane (sf::Mouse::getPosition (window_));

                        draw_mode_ = JULIA;
                        DrawSet ();
                        draw_mode_ = MAIN;

                        window_.draw (sprite_);
                        window_.display ();

                        julia_dragging = true;
                    }
                }
                else
                {
                    DrawSet ();
                    julia_dragging = false;
                }
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::J) &&
                     (!InputBoxesHasFocus ()))
            {
                if (!julia_dragging)
                    draw_mode_ = MAIN;
                else
                    draw_mode_ = JULIA;

                DrawSet ();
            }

            // Change max iterations
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::I) &&
                     (!InputBoxesHasFocus ()))
            {
                change_iter = true;
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::I) &&
                     (!InputBoxesHasFocus ()))
            {
                change_iter = false;
            }
            else if ((event.type == sf::Event::MouseWheelMoved) && change_iter)
            {
                itrn_max_ += static_cast<size_t> (event.mouseWheel.delta * 50);

                DrawSet ();
            }

            // Change limit
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::L) &&
                     (!InputBoxesHasFocus ()))
            {
                change_limit = true;
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::L) &&
                     (!InputBoxesHasFocus ()))
            {
                change_limit = false;
            }
            else if ((event.type == sf::Event::MouseWheelMoved) && change_limit)
            {
                lim_ *= pow (2.0, static_cast<float> (event.mouseWheel.delta));

                DrawSet ();
            }

            // Toggle action modes
            else if ((event.type == sf::Event::KeyPressed) && (!InputBoxesHasFocus ()))
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Z: action_mode = ZOOMING; break;
                case sf::Keyboard::P: action_mode = POINT_TRACING; break;
                case sf::Keyboard::S: action_mode = SOUNDING; break;
                default: break;
                }
            }

            // Zooming
            else if (event.type == sf::Event::MouseWheelMoved)
            {
                Zooming (static_cast<float> (event.mouseWheel.delta), Screen2Plane (sf::Mouse::getPosition (window_)));

                DrawSet ();
            }
            else if ((action_mode == ZOOMING) &&
                     (sf::Mouse::isButtonPressed (sf::Mouse::Left) || sf::Mouse::isButtonPressed (sf::Mouse::Right)))
            {
                if (GetNewScreen (newscreen))
                {
                    changeBorders (newscreen);

                    DrawSet ();
                }
            }

            // Point tracing and sounding
            else if (((action_mode == POINT_TRACING) || (action_mode == SOUNDING)) &&
                     (event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
            {
                input_box_x_.is_visible_ = false;
                input_box_x_.has_focus_ = false;

                input_box_y_.is_visible_ = false;
                input_box_y_.has_focus_ = false;

                input_box_z_.is_visible_ = false;
                input_box_z_.has_focus_ = false;

                showing_trace = true;
                left_pressed = true;

                if (action_mode == SOUNDING)
                {
                    synth.SetPoint (Screen2Plane (sf::Mouse::getPosition (window_)));
                    synth.audio_pause_ = false;
                    synth.play ();
                }
            }
            else if ((event.type == sf::Event::MouseButtonReleased) && (event.mouseButton.button == sf::Mouse::Left))
            {
                left_pressed = false;
            }
            else if (((action_mode == POINT_TRACING) || (action_mode == SOUNDING)) &&
                     (sf::Mouse::isButtonPressed (sf::Mouse::Right)))
            {
                showing_trace = false;
                synth.audio_pause_ = true;
                synth.pause ();
            }
        }

        window_.clear ();
        window_.draw (sprite_);

        if ((input_mode_ == Z_INPUT) && input_box_z_.is_visible_)
        {
            input_box_z_.draw (window_);
        }
        if ((input_mode_ == XY_INPUT) && input_box_x_.is_visible_)
        {
            input_box_x_.draw (window_);
            input_box_y_.draw (window_);
        }

        if (left_pressed)
        {
            c_point = Screen2Plane (sf::Mouse::getPosition (window_));
            orbit = c_point;

            synth.SetPoint (Screen2Plane (sf::Mouse::getPosition (window_)));
        }

        if (showing_trace)
            orbit = PointTrace (orbit, c_point);

        if (showing_menu)
            drawHelpMenu ();

        window_.display ();
    }

    synth.stop ();
}

sf::Vector2<double> Puzabrot::Screen2Plane (sf::Vector2i point) const
{
    return sf::Vector2<double> (borders_.re_left + (borders_.re_right - borders_.re_left) * point.x / winsizes_.x,
                                borders_.im_top - (borders_.im_top - borders_.im_bottom) * point.y / winsizes_.y);
}

void Puzabrot::updateWinSizes (size_t new_width, size_t new_height)
{
    winsizes_.x = static_cast<unsigned int> (new_width);
    winsizes_.y = static_cast<unsigned int> (new_height);

    render_texture_.create (winsizes_.x, winsizes_.y);
    sprite_ = sf::Sprite (render_texture_.getTexture ());

    borders_.re_right = borders_.re_left + (borders_.im_top - borders_.im_bottom) * static_cast<float> (winsizes_.x) /
                                               static_cast<float> (winsizes_.y);
}

void Puzabrot::toggleFullScreen ()
{
    if ((winsizes_.x == sf::VideoMode::getDesktopMode ().width) &&
        (winsizes_.y == sf::VideoMode::getDesktopMode ().height))
    {
        window_.create (sf::VideoMode (DEFAULT_WIDTH, DEFAULT_HEIGHT), TITLE_STRING);
        updateWinSizes (DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
    else
    {
        window_.create (sf::VideoMode::getDesktopMode (), TITLE_STRING, sf::Style::Fullscreen);
        updateWinSizes (sf::VideoMode::getDesktopMode ().width, sf::VideoMode::getDesktopMode ().height);
    }
}

bool Puzabrot::InputBoxesHasFocus ()
{
    return (input_box_x_.has_focus_) || (input_box_y_.has_focus_) || (input_box_z_.has_focus_);
}

bool Puzabrot::InputBoxesIsVisible ()
{
    return (input_box_x_.is_visible_) || (input_box_y_.is_visible_) || (input_box_z_.is_visible_);
}

void Puzabrot::DrawSet ()
{
    shader_.setUniform ("borders",
                        sf::Glsl::Vec4 (static_cast<float> (borders_.re_left), static_cast<float> (borders_.re_right),
                                        static_cast<float> (borders_.im_bottom), static_cast<float> (borders_.im_top)));

    shader_.setUniform ("winsizes", sf::Glsl::Ivec2 (static_cast<int> (winsizes_.x), static_cast<int> (winsizes_.y)));

    shader_.setUniform ("itrn_max", static_cast<int> (itrn_max_));
    shader_.setUniform ("limit", static_cast<float> (lim_));

    shader_.setUniform ("drawing_mode", draw_mode_);
    shader_.setUniform ("coloring", coloring_);

    shader_.setUniform ("julia_point",
                        sf::Glsl::Vec2 (static_cast<float> (julia_point_.x), static_cast<float> (julia_point_.y)));

    render_texture_.draw (sprite_, &shader_);
}

void Puzabrot::Zooming (double wheel_delta, sf::Vector2<double> point)
{
    double width = borders_.re_right - borders_.re_left;
    double height = borders_.im_top - borders_.im_bottom;

    double x_ratio = (point.x - borders_.re_left) / width;
    double y_ratio = (point.y - borders_.im_bottom) / height;

    ComplexFrame new_frame = {
        borders_.re_left + x_ratio * ZOOMING_RATIO * width * wheel_delta,
        borders_.re_right - (1 - x_ratio) * ZOOMING_RATIO * width * wheel_delta,
        borders_.im_bottom + y_ratio * ZOOMING_RATIO * height * wheel_delta,
        borders_.im_top - (1 - y_ratio) * ZOOMING_RATIO * height * wheel_delta,
    };

    borders_ = new_frame;
}

int Puzabrot::GetNewScreen (Screen& newscreen)
{
    unsigned int w = winsizes_.x;
    unsigned int h = winsizes_.y;

    sf::Vector2i start (-1, -1);
    sf::Vector2i end (-1, -1);

    sf::RectangleShape rectangle;
    rectangle.setOutlineThickness (1);
    rectangle.setFillColor (sf::Color::Transparent);

    while (true)
    {
        if (sf::Mouse::isButtonPressed (sf::Mouse::Left) || sf::Mouse::isButtonPressed (sf::Mouse::Right))
        {
            start = sf::Mouse::getPosition (window_);
            rectangle.setPosition (sf::Vector2f (start));

            while (sf::Mouse::isButtonPressed (sf::Mouse::Left) || sf::Mouse::isButtonPressed (sf::Mouse::Right))
            {
                end = sf::Mouse::getPosition (window_) + sf::Vector2i (1, 1);

                if ((abs (end.x - start.x) > 8) && (abs (end.y - start.y) > 8))
                {
                    double sx = start.x;
                    double sy = start.y;
                    double ex = end.x;
                    double ey = end.y;

                    if (((end.y > start.y) && (end.x > start.x)) || ((end.y < start.y) && (end.x < start.x)))
                    {
                        end.x = static_cast<int> ((w * h * (ey - sy) + w * w * ex + h * h * sx) / (w * w + h * h));
                        end.y = static_cast<int> ((w * h * (ex - sx) + w * w * sy + h * h * ey) / (w * w + h * h));
                    }
                    else
                    {
                        end.x = static_cast<int> ((w * h * (sy - ey) + w * w * ex + h * h * sx) / (w * w + h * h));
                        end.y = static_cast<int> ((w * h * (sx - ex) + w * w * sy + h * h * ey) / (w * w + h * h));
                    }

                    if (sf::Mouse::isButtonPressed (sf::Mouse::Left))
                    {
                        rectangle.setOutlineColor (sf::Color::Blue);
                        newscreen.zoom = static_cast<double> (w) * h / abs (end.x - start.x) / abs (end.y - start.y);
                    }
                    else if (sf::Mouse::isButtonPressed (sf::Mouse::Right))
                    {
                        rectangle.setOutlineColor (sf::Color::Red);
                        newscreen.zoom = static_cast<double> (abs (end.x - start.x)) * abs (end.y - start.y) / w / h;
                    }

                    rectangle.setSize (sf::Vector2f (end - start));

                    window_.draw (sprite_);
                    window_.draw (rectangle);
                    window_.display ();
                }
                else
                    end.x = 0;
            }
        }

        window_.draw (sprite_);

        if (end.x != -1)
            break;
        return 0;
    }

    if (start.x > end.x)
    {
        newscreen.x2 = static_cast<unsigned int> (start.x);
        newscreen.x1 = static_cast<unsigned int> (end.x);
    }
    else
    {
        newscreen.x1 = static_cast<unsigned int> (start.x);
        newscreen.x2 = static_cast<unsigned int> (end.x);
    }

    if (start.y > end.y)
    {
        newscreen.y2 = static_cast<unsigned int> (start.y);
        newscreen.y1 = static_cast<unsigned int> (end.y);
    }
    else
    {
        newscreen.y1 = static_cast<unsigned int> (start.y);
        newscreen.y2 = static_cast<unsigned int> (end.y);
    }

    return 1;
}

void Puzabrot::changeBorders (Screen newscreen)
{
    double releft = borders_.re_left;
    double reright = borders_.re_right;
    double imup = borders_.im_top;
    double imdown = borders_.im_bottom;

    if (newscreen.zoom > 1)
    {
        borders_.re_left = releft + (reright - releft) * newscreen.x1 / winsizes_.x;
        borders_.re_right = releft + (reright - releft) * newscreen.x2 / winsizes_.x;
        borders_.im_bottom = imup - (imup - imdown) * newscreen.y2 / winsizes_.y;

        borders_.im_top = borders_.im_bottom + (borders_.re_right - borders_.re_left) * winsizes_.y / winsizes_.x;
    }
    else
    {
        borders_.re_left = releft - (reright - releft) * newscreen.x1 / (newscreen.x2 - newscreen.x1);
        borders_.re_right = reright + (reright - releft) * (winsizes_.x - newscreen.x2) / (newscreen.x2 - newscreen.x1);
        borders_.im_bottom = imdown - (imup - imdown) * (winsizes_.y - newscreen.y2) / (newscreen.y2 - newscreen.y1);

        borders_.im_top = borders_.im_bottom + (borders_.re_right - borders_.re_left) * winsizes_.y / winsizes_.x;
    }
}

void Puzabrot::initCalculator (Calculator& calc, sf::Vector2<double> z, sf::Vector2<double> c) const
{
    switch (input_mode_)
    {
    case Z_INPUT:
    {
        calc.variables.push_back ({ { c.x, c.y }, "c" });
        calc.variables.push_back ({ { z.x, z.y }, "z" });
        break;
    }
    case XY_INPUT:
    {
        calc.variables.push_back ({ { c.x, 0 }, "cx" });
        calc.variables.push_back ({ { c.y, 0 }, "cy" });

        calc.variables.push_back ({ { z.x, 0 }, "x" });
        calc.variables.push_back ({ { z.y, 0 }, "y" });
        break;
    }
    }
}

void Puzabrot::Mapping (Calculator& calc, double& mapped_x, double& mapped_y)
{
    switch (input_mode_)
    {
    case Z_INPUT:
    {
        calc.Calculate (expr_trees_[0]);
        calc.variables[calc.variables.size () - 1] = { expr_trees_[0].data.number, "z" };

        mapped_x = real (expr_trees_[0].data.number);
        mapped_y = imag (expr_trees_[0].data.number);
        break;
    }
    case XY_INPUT:
    {
        calc.Calculate (expr_trees_[0]);
        calc.Calculate (expr_trees_[1]);

        calc.variables[calc.variables.size () - 2] = { { real (expr_trees_[0].data.number), 0.0 }, "x" };
        calc.variables[calc.variables.size () - 1] = { { real (expr_trees_[1].data.number), 0.0 }, "y" };

        mapped_x = real (expr_trees_[0].data.number);
        mapped_y = real (expr_trees_[1].data.number);
        break;
    }
    }
}

sf::Vector2<double> Puzabrot::PointTrace (sf::Vector2<double> point, sf::Vector2<double> c_point)
{
    static Calculator calc;
    switch (draw_mode_)
    {
    case MAIN: initCalculator (calc, point, c_point); break;
    case JULIA: initCalculator (calc, point, julia_point_); break;
    }

    double x1 = point.x;
    double y1 = point.y;

    double x2 = 0.0;
    double y2 = 0.0;

    for (size_t i = 0; (i < itrn_max_) && (sqrt (x2 * x2 + y2 * y2) < lim_); ++i)
    {
        Mapping (calc, x2, y2);

        sf::Vertex line[] = {
            sf::Vertex (
                sf::Vector2f (
                    static_cast<float> ((x1 - borders_.re_left) / (borders_.re_right - borders_.re_left) * winsizes_.x),
                    static_cast<float> ((borders_.im_top - y1) / (borders_.im_top - borders_.im_bottom) * winsizes_.y)),
                sf::Color::White),

            sf::Vertex (
                sf::Vector2f (
                    static_cast<float> ((x2 - borders_.re_left) / (borders_.re_right - borders_.re_left) * winsizes_.x),
                    static_cast<float> ((borders_.im_top - y2) / (borders_.im_top - borders_.im_bottom) * winsizes_.y)),
                sf::Color::Black)
        };

        x1 = x2;
        y1 = y2;

        window_.draw (line, 2, sf::Lines);
    }

    calc.clear ();

    return sf::Vector2<double> (x1, y1);
}

void Puzabrot::savePicture ()
{
    static int shot_num = 0;
    std::string filename = "screenshot(" + std::to_string (shot_num++) + ")" + ".png";

    window_.draw (sprite_);

    sf::RectangleShape rectangle;
    rectangle.setPosition (0, 0);
    rectangle.setSize (sf::Vector2f (winsizes_));
    rectangle.setFillColor (sf::Color (10, 10, 10, 150));

    window_.draw (rectangle);
    window_.display ();

    sf::Vector2u screenshot_sizes (SCREENSHOT_WIDTH,
                                   static_cast<unsigned int> (static_cast<float> (SCREENSHOT_WIDTH) /
                                                              static_cast<float> (winsizes_.x * winsizes_.y)));

    shader_.setUniform ("borders",
                        sf::Glsl::Vec4 (static_cast<float> (borders_.re_left), static_cast<float> (borders_.re_right),
                                        static_cast<float> (borders_.im_bottom), static_cast<float> (borders_.im_top)));

    shader_.setUniform ("winsizes", sf::Glsl::Ivec2 (static_cast<int> (winsizes_.x), static_cast<int> (winsizes_.y)));

    shader_.setUniform ("itrn_max", static_cast<int> (itrn_max_));
    shader_.setUniform ("limit", static_cast<float> (lim_));

    shader_.setUniform ("drawing_mode", draw_mode_);
    shader_.setUniform ("coloring", coloring_);

    shader_.setUniform ("julia_point",
                        sf::Glsl::Vec2 (static_cast<float> (julia_point_.x), static_cast<float> (julia_point_.y)));

    sf::RenderTexture render_texture;
    render_texture.create (screenshot_sizes.x, screenshot_sizes.y);

    sf::Sprite sprite (render_texture.getTexture ());

    render_texture.draw (sprite, &shader_);

    sf::Texture screen = render_texture.getTexture ();

    screen.copyToImage ().saveToFile (filename);

    window_.draw (sprite_);
    window_.display ();
}

void Puzabrot::drawHelpMenu ()
{
    sf::RectangleShape dim_rect (sf::Vector2f (static_cast<float> (winsizes_.x), static_cast<float> (winsizes_.y)));
    dim_rect.setFillColor (sf::Color (0, 0, 0, 128));
    window_.draw (dim_rect);

    sf::Font font;
    font.loadFromMemory (consola_ttf, consola_ttf_len);

    sf::Text help_menu;
    help_menu.setFont (font);
    help_menu.setCharacterSize (22);
    help_menu.setPosition (10.0F, 10.0F);
    help_menu.setFillColor (sf::Color::White);

    char str[1000] = "";

    sprintf (str,
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
             "        Borders: upper: %.5lf, bottom: %.5lf, left: %.5lf, right: %.5lf\n",
             itrn_max_, lim_, borders_.im_top, borders_.im_bottom, borders_.re_left, borders_.re_right);

    help_menu.setString (str);
    window_.draw (help_menu);
}

int Puzabrot::makeShader ()
{
    switch (input_mode_)
    {
    case Z_INPUT:
    {
        Expression expr_z (input_box_z_.getInput ());

        int err = expr_z.getTree (expr_trees_[0]);
        if (err)
            return err;
        break;
    }
    case XY_INPUT:
    {
        Expression expr_x (input_box_x_.getInput ());
        Expression expr_y (input_box_y_.getInput ());

        int err = expr_x.getTree (expr_trees_[0]);
        if (err)
            return err;

        err = expr_y.getTree (expr_trees_[1]);
        if (err)
            return err;
    }
    }

    char* str_shader = writeShader ();
    if (str_shader == nullptr)
        return CALC_WRONG_VARIABLE;

    shader_.loadFromMemory (str_shader, sf::Shader::Fragment);

    delete[] str_shader;

    return 0;
}

char* Puzabrot::writeShader ()
{
    char* str_initialization = writeInitialization ();

    char* str_calculation = writeCalculation ();
    if (str_calculation == nullptr)
        return nullptr;

    char* str_checking = writeChecking ();

    char* str_shader = new char[10000] {};

    sprintf (str_shader,
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
             "vec2 cabs(vec2 a)\n"
             "{\n"
             "    return vec2(sqrt(a.x * a.x + a.y * a.y), 0.0);\n"
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
             "        itrn = itrn * 4 %% 1530;\n"
             "             if (itrn < 256)  return vec3( 255,         itrn,        0           ) / 255 * (1.0 - "
             "float(coloring)*0.85);\n"
             "        else if (itrn < 511)  return vec3( 510 - itrn,  255,         0           ) / 255 * (1.0 - "
             "float(coloring)*0.85);\n"
             "        else if (itrn < 766)  return vec3( 0,           255,         itrn - 510  ) / 255 * (1.0 - "
             "float(coloring)*0.85);\n"
             "        else if (itrn < 1021) return vec3( 0,           1020 - itrn, 255         ) / 255 * (1.0 - "
             "float(coloring)*0.85);\n"
             "        else if (itrn < 1276) return vec3( itrn - 1020, 0,           255         ) / 255 * (1.0 - "
             "float(coloring)*0.85);\n"
             "        else if (itrn < 1530) return vec3( 255,         0,           1529 - itrn ) / 255 * (1.0 - "
             "float(coloring)*0.85);\n"
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
             "}",
             str_initialization, str_calculation, str_checking);

    delete[] str_initialization;
    delete[] str_calculation;
    delete[] str_checking;

    return str_shader;
}

char* Puzabrot::writeInitialization () const
{
    switch (input_mode_)
    {
    case Z_INPUT:
    {
        char* str_initialization = new char[1000] {};

        sprintf (str_initialization,
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

        sprintf (str_initialization,
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

char* Puzabrot::writeCalculation ()
{
    switch (input_mode_)
    {
    case Z_INPUT:
    {
        char* str_calculation = new char[1000] {};
        sprintf (str_calculation,
                 "vec2 ppz = pz;\n"
                 "pz = z;\n");

        sprintf (str_calculation + strlen (str_calculation), "z = ");

        int err = Tree2GLSL (expr_trees_[0], str_calculation + strlen (str_calculation));
        if (err)
        {
            delete[] str_calculation;
            return nullptr;
        }

        sprintf (str_calculation + strlen (str_calculation), ";");

        return str_calculation;
    }
    case XY_INPUT:
    {
        char* str_calculation = new char[1000] {};
        sprintf (str_calculation,
                 "vec2 ppz = pz;\n"
                 "pz = vec2(x, y);\n");

        sprintf (str_calculation + strlen (str_calculation), "vec2 x1 = ");

        int err = Tree2GLSL (expr_trees_[0], str_calculation + strlen (str_calculation));
        if (err)
        {
            delete[] str_calculation;
            return nullptr;
        }

        sprintf (str_calculation + strlen (str_calculation), ";\nvec2 y1 = ");

        err = Tree2GLSL (expr_trees_[1], str_calculation + strlen (str_calculation));
        if (err)
        {
            delete[] str_calculation;
            return nullptr;
        }

        sprintf (str_calculation + strlen (str_calculation), ";\n");

        sprintf (str_calculation + strlen (str_calculation), "x = x1.x;\ny = y1.x;");

        return str_calculation;
    }
    default: return nullptr;
    }
}

char* Puzabrot::writeChecking () const
{
    char* str_checking = new char[1000] {};

    switch (input_mode_)
    {
    case Z_INPUT: sprintf (str_checking, "if (cabs(z).x > limit) break;\n"); break;
    case XY_INPUT: sprintf (str_checking, "if (cabs(vec2(x, y)).x > limit) break;\nvec2 z = vec2(x, y);\n"); break;
    default: return nullptr;
    }

    sprintf (str_checking + strlen (str_checking),
             "sumz.x += dot(z - pz, pz - ppz);\n"
             "sumz.y += dot(z - pz,  z - pz);\n"
             "sumz.z += dot(z - ppz, z - ppz);");

    return str_checking;
}

int Puzabrot::Tree2GLSL (Tree<CalcData>& node, char* str_cur)
{
    assert (str_cur != nullptr);

    switch (node.data.node_type)
    {
    case NODE_FUNCTION:
    {
        sprintf (str_cur, "c%s(", node.data.word.c_str ());

        int err = Tree2GLSL (node.branches[0], str_cur + strlen (str_cur));
        if (err)
            return err;
        sprintf (str_cur + strlen (str_cur), ")");

        break;
    }
    case NODE_OPERATOR:
    {
        switch (node.data.op_code)
        {
        case OP_ADD: sprintf (str_cur, "cadd("); break;
        case OP_SUB: sprintf (str_cur, "csub("); break;
        case OP_MUL: sprintf (str_cur, "cmul("); break;
        case OP_DIV: sprintf (str_cur, "cdiv("); break;
        case OP_POW: sprintf (str_cur, "cpow("); break;
        default: assert (0);
        }

        if (node.branches.size () < 2)
        {
            sprintf (str_cur + strlen (str_cur), "vec2(0, 0), ");
        }
        else
        {
            int err = Tree2GLSL (node.branches[1], str_cur + strlen (str_cur));
            if (err)
                return err;
            sprintf (str_cur + strlen (str_cur), ", ");
        }

        int err = Tree2GLSL (node.branches[0], str_cur + strlen (str_cur));
        if (err)
            return err;

        sprintf (str_cur + strlen (str_cur), ")");

        break;
    }
    case NODE_VARIABLE:
    {
        switch (input_mode_)
        {
        case Z_INPUT:
        {
            if ((node.data.word != "z") && (node.data.word != "c") && (node.data.word != "pi") &&
                (node.data.word != "e") && (node.data.word != "i"))
                return -1;

            break;
        }
        case XY_INPUT:
        {
            if ((node.data.word != "x") && (node.data.word != "y") && (node.data.word != "cx") &&
                (node.data.word != "cy") && (node.data.word != "pi") && (node.data.word != "e") &&
                (node.data.word != "i"))
                return -1;

            break;
        }
        }

        if (node.data.word == "i")
            sprintf (str_cur, "I");
        else
            switch (input_mode_)
            {
            case Z_INPUT: sprintf (str_cur, "%s", node.data.word.c_str ()); break;
            case XY_INPUT: sprintf (str_cur, "vec2(%s, 0)", node.data.word.c_str ()); break;
            }

        break;
    }
    case NODE_NUMBER:
    {
        switch (input_mode_)
        {
        case Z_INPUT: sprintf (str_cur, "vec2(%f, %f)", real (node.data.number), imag (node.data.number)); break;
        case XY_INPUT: sprintf (str_cur, "vec2(%f,  0)", real (node.data.number)); break;
        }

        break;
    }
    default: assert (0);
    }

    return 0;
}

Synth::Synth (Puzabrot* puza) :
    audio_reset_ (true), audio_pause_ (false), sustain_ (true), volume_ (8000.0), puza_ (puza),
    point_ (sf::Vector2<double> (0.0, 0.0)), c_point_ (sf::Vector2<double> (0.0, 0.0)),
    new_point_ (sf::Vector2<double> (0.0, 0.0)), prev_point_ (sf::Vector2<double> (0.0, 0.0))
{
    initialize (2, SAMPLE_RATE);
    setLoop (true);

    updateCalc ();
}

void Synth::updateCalc ()
{
    calc_.variables.clear ();
    puza_->initCalculator (calc_, point_, c_point_);
}

void Synth::SetPoint (sf::Vector2<double> point)
{
    new_point_ = point;

    audio_reset_ = true;
    audio_pause_ = false;
}

bool Synth::onGetData (Chunk& data)
{
    data.samples = m_samples;
    data.sampleCount = AUDIO_BUFF_SIZE;
    memset (m_samples, 0, AUDIO_BUFF_SIZE * sizeof (int16_t));

    if (audio_reset_)
    {
        m_audio_time = 0;

        switch (puza_->draw_mode_)
        {
        case MAIN: c_point_ = new_point_; break;
        case JULIA: c_point_ = puza_->julia_point_; break;
        }

        point_ = new_point_;
        prev_point_ = new_point_;

        mean_x = new_point_.x;
        mean_y = new_point_.y;
        volume_ = 8000.0;

        audio_reset_ = false;
    }

    if (audio_pause_)
        return true;

    const int steps = SAMPLE_RATE / MAX_FREQ;
    for (size_t i = 0; i < AUDIO_BUFF_SIZE; i += 2)
    {
        const int j = m_audio_time % steps;
        if (j == 0)
        {
            prev_point_ = point_;

            updateCalc ();
            puza_->Mapping (calc_, point_.x, point_.y);

            if (sqrt (point_.x * point_.x + point_.y * point_.y) > puza_->lim_)
            {
                audio_pause_ = true;
                return true;
            }

            dpx = prev_point_.x - c_point_.x;
            dpy = prev_point_.y - c_point_.y;
            dx = point_.x - c_point_.x;
            dy = point_.y - c_point_.y;

            if (dx != 0.0 || dy != 0.0)
            {
                double dpmag = 1.0 / std::sqrt (1e-12 + dpx * dpx + dpy * dpy);
                double dmag = 1.0 / std::sqrt (1e-12 + dx * dx + dy * dy);

                dpx *= dpmag;
                dpy *= dpmag;
                dx *= dmag;
                dy *= dmag;
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

        double t = 0.5 - 0.5 * cos (double (j) / double (steps) * real (PI));

        double wx = t * dx + (1.0 - t) * dpx;
        double wy = t * dy + (1.0 - t) * dpy;

        m_samples[i] = static_cast<int16_t> (std::min (std::max (wx * volume_, -32000.0), 32000.0));
        m_samples[i + 1] = static_cast<int16_t> (std::min (std::max (wy * volume_, -32000.0), 32000.0));

        m_audio_time += 1;
    }

    return !audio_reset_;
}

} // namespace puza
