/*------------------------------------------------------------------------------
 * File:        Puzabrot.cpp                                                   *
 * Description: Puzabrot implementation                                        *
 * Created:     30 jun 2021                                                    *
 * Author:      Artem Puzankov                                                 *
 * Email:       puzankov.ao@phystech.edu                                       *
 * GitHub:      https://github.com/hellopuza                                   *
 * Copyright © 2021 Artem Puzankov. All rights reserved.                       *
 *///---------------------------------------------------------------------------

#include "Puzabrot.h"

#include <cstring>

namespace puza {

Puzabrot::Puzabrot() :
    solver_(sf::Vector2u(DEFAULT_WIDTH, DEFAULT_HEIGHT)), shader_(sf::Vector2u(DEFAULT_WIDTH, DEFAULT_HEIGHT)),
    window_(sf::VideoMode(solver_.winsizes.x, solver_.winsizes.y), TITLE_STRING),
    input_boxes_ { InputBox(sf::Vector2f(10, 10), sf::Color(128, 128, 128, 128), sf::Color::White, 20.0F),
                   InputBox(sf::Vector2f(10, 50), sf::Color(128, 128, 128, 128), sf::Color::White, 20.0F),
                   InputBox(sf::Vector2f(10, 10), sf::Color(128, 128, 128, 128), sf::Color::White, 20.0F) },
    synth_(this)
{
    input_boxes_.x.setLabel(sf::String("x:"));
    input_boxes_.y.setLabel(sf::String("y:"));
    input_boxes_.z.setLabel(sf::String("z:"));

    input_boxes_.x.setInput(sf::String("x*x-y*y+cx"));
    input_boxes_.y.setInput(sf::String("2*x*y+cy"));
    input_boxes_.z.setInput(sf::String("z^2+c"));

    font_.loadFromFile("assets/consola.ttf");
}

void Puzabrot::run()
{
    window_.setVerticalSyncEnabled(true);

    int action_mode = POINT_TRACING;

    makeShader();
    DrawSet();

    bool showing_menu   = false;
    bool showing_trace  = false;
    bool julia_dragging = false;
    bool left_pressed   = false;
    bool change_iter    = false;
    bool change_limit   = false;

    point_t orbit(0.0, 0.0);
    point_t c_point(0.0, 0.0);

    while (window_.isOpen())
    {
        sf::Event event;
        Frame     zooming_frame;
        bool      was_screenshot = false;
        while (window_.pollEvent(event))
        {
            // Close window
            if ((event.type == sf::Event::Closed) ||
                ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window_.close();
                return;
            }

            // Toggle fullscreen
            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F11))
            {
                toggleFullScreen();
                DrawSet();
            }

            // Resize window
            else if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visible_area(0.0F, 0.0F, static_cast<float>(event.size.width),
                                           static_cast<float>(event.size.height));
                window_.setView(sf::View(visible_area));
                updateWinSizes(window_.getSize().x, window_.getSize().y);
                DrawSet();
            }

            // Reset set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R) &&
                     (!InputBoxesHasFocus()))
            {
                solver_.reset();

                options_.draw_mode = MAIN;
                DrawSet();
            }

            // Take a screenshot
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space) &&
                     (!was_screenshot) && (!InputBoxesHasFocus()))
            {
                savePicture();
                was_screenshot = true;
            }

            // Toggle audio dampening
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::D) &&
                     (!InputBoxesHasFocus()))
            {
                synth_.sustain_ = !synth_.sustain_;
            }

            // Toggle sound coloring
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::C) &&
                     (!InputBoxesHasFocus()))
            {
                options_.coloring = !options_.coloring;
                DrawSet();
            }

            // Toggle help menu showing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::H) &&
                     (!InputBoxesHasFocus()))
            {
                showing_menu = !showing_menu;
                if (showing_menu)
                {
                    input_boxes_.x.is_visible_ = false;
                    input_boxes_.y.is_visible_ = false;
                    input_boxes_.z.is_visible_ = false;
                }
            }

            // Toggle input mode
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tab) &&
                     (!InputBoxesHasFocus()))
            {
                switch (options_.input_mode)
                {
                case ComplexShader::Z_INPUT:
                {
                    input_boxes_.x.has_focus_  = false;
                    input_boxes_.x.is_visible_ = input_boxes_.z.is_visible_;

                    input_boxes_.y.has_focus_  = false;
                    input_boxes_.y.is_visible_ = input_boxes_.z.is_visible_;

                    input_boxes_.z.has_focus_  = false;
                    input_boxes_.z.is_visible_ = false;

                    options_.input_mode = ComplexShader::XY_INPUT;
                    break;
                }
                case ComplexShader::XY_INPUT:
                {
                    input_boxes_.z.has_focus_  = false;
                    input_boxes_.z.is_visible_ = input_boxes_.x.is_visible_;

                    input_boxes_.x.has_focus_  = false;
                    input_boxes_.x.is_visible_ = false;

                    input_boxes_.y.has_focus_  = false;
                    input_boxes_.y.is_visible_ = false;

                    options_.input_mode = ComplexShader::Z_INPUT;
                    break;
                }
                }
            }

            // Toggle input box visibility
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tilde) &&
                     (!InputBoxesHasFocus()))
            {
                switch (options_.input_mode)
                {
                case ComplexShader::Z_INPUT:
                {
                    input_boxes_.z.is_visible_ = !input_boxes_.z.is_visible_;
                    if (!input_boxes_.z.is_visible_)
                        input_boxes_.z.has_focus_ = false;
                    break;
                }
                case ComplexShader::XY_INPUT:
                {
                    input_boxes_.x.is_visible_ = !input_boxes_.x.is_visible_;
                    input_boxes_.y.is_visible_ = !input_boxes_.y.is_visible_;

                    if (!input_boxes_.x.is_visible_)
                    {
                        input_boxes_.x.has_focus_ = false;
                        input_boxes_.y.has_focus_ = false;
                    }
                    break;
                }
                }
            }

            // Toggle input box focus
            else if (InputBoxesIsVisible() && (event.type == sf::Event::MouseButtonPressed) &&
                     (event.mouseButton.button == sf::Mouse::Left))
            {
                sf::Vector2f mouse_button(static_cast<float>(event.mouseButton.x),
                                          static_cast<float>(event.mouseButton.y));
                switch (options_.input_mode)
                {
                case ComplexShader::Z_INPUT:
                {
                    input_boxes_.z.has_focus_ =
                        ((input_boxes_.z.getPosition().x < mouse_button.x) &&
                         (mouse_button.x < input_boxes_.z.getPosition().x + input_boxes_.z.getSize().x) &&
                         (input_boxes_.z.getPosition().y < mouse_button.y) &&
                         (mouse_button.y < input_boxes_.z.getPosition().y + input_boxes_.z.getSize().y));
                    break;
                }
                case ComplexShader::XY_INPUT:
                {
                    input_boxes_.x.has_focus_ =
                        ((input_boxes_.x.getPosition().x < mouse_button.x) &&
                         (mouse_button.x < input_boxes_.x.getPosition().x + input_boxes_.x.getSize().x) &&
                         (input_boxes_.x.getPosition().y < mouse_button.y) &&
                         (mouse_button.y < input_boxes_.x.getPosition().y + input_boxes_.x.getSize().y));

                    input_boxes_.y.has_focus_ =
                        ((input_boxes_.y.getPosition().x < mouse_button.x) &&
                         (mouse_button.x < input_boxes_.y.getPosition().x + input_boxes_.y.getSize().x) &&
                         (input_boxes_.y.getPosition().y < mouse_button.y) &&
                         (mouse_button.y < input_boxes_.y.getPosition().y + input_boxes_.y.getSize().y));
                    break;
                }
                }
            }

            // Input text expression to input box
            else if (InputBoxesHasFocus() && (event.type == sf::Event::TextEntered) && (event.text.unicode < 128))
            {
                switch (options_.input_mode)
                {
                case ComplexShader::Z_INPUT:
                {
                    input_boxes_.z.setInput(event.text.unicode);
                    break;
                }
                case ComplexShader::XY_INPUT:
                {
                    if (input_boxes_.x.has_focus_)
                        input_boxes_.x.setInput(event.text.unicode);
                    else
                        input_boxes_.y.setInput(event.text.unicode);
                    break;
                }
                }
            }

            // Enter expression from input box
            else if (InputBoxesHasFocus() && (event.type == sf::Event::KeyPressed) &&
                     (event.key.code == sf::Keyboard::Enter))
            {
                int err = makeShader();

                if (!err)
                {
                    DrawSet();
                    options_.draw_mode = MAIN;
                }

                switch (options_.input_mode)
                {
                case ComplexShader::Z_INPUT:
                {
                    if (err)
                        input_boxes_.z.setOutput(sf::String(calc_errstr[err + 1]));
                    else
                        input_boxes_.z.setOutput(sf::String());
                    break;
                }
                case ComplexShader::XY_INPUT:
                {
                    if (input_boxes_.x.has_focus_)
                        if (err)
                        {
                            input_boxes_.x.setOutput(sf::String(calc_errstr[err + 1]));
                            input_boxes_.y.setPosition(
                                sf::Vector2f(input_boxes_.y.getPosition().x,
                                             input_boxes_.y.getPosition().y + 0.5F * input_boxes_.x.getSize().y));
                        }
                        else
                        {
                            input_boxes_.x.setOutput(sf::String());
                            input_boxes_.y.setPosition(sf::Vector2f(10.0F, 50.0F));
                        }
                    else if (err)
                        input_boxes_.y.setOutput(sf::String(calc_errstr[err + 1]));
                    else
                        input_boxes_.y.setOutput(sf::String());
                    break;
                }
                }
            }

            // Julia set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::J) &&
                     (!InputBoxesHasFocus()))
            {
                if (options_.draw_mode != JULIA)
                {
                    while (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
                    {
                        solver_.julia_point = solver_.Screen2Plane(sf::Mouse::getPosition(window_));

                        options_.draw_mode = JULIA;
                        DrawSet();
                        options_.draw_mode = MAIN;

                        window_.draw(shader_.sprite);
                        window_.display();

                        julia_dragging = true;
                    }
                }
                else
                {
                    DrawSet();
                    julia_dragging = false;
                }
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::J) &&
                     (!InputBoxesHasFocus()))
            {
                if (!julia_dragging)
                    options_.draw_mode = MAIN;
                else
                    options_.draw_mode = JULIA;

                DrawSet();
            }

            // Change max iterations
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::I) &&
                     (!InputBoxesHasFocus()))
            {
                change_iter = true;
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::I) &&
                     (!InputBoxesHasFocus()))
            {
                change_iter = false;
            }
            else if ((event.type == sf::Event::MouseWheelMoved) && change_iter)
            {
                solver_.itrn_max += static_cast<size_t>(event.mouseWheel.delta * 50);
                DrawSet();
            }

            // Change limit
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::L) &&
                     (!InputBoxesHasFocus()))
            {
                change_limit = true;
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::L) &&
                     (!InputBoxesHasFocus()))
            {
                change_limit = false;
            }
            else if ((event.type == sf::Event::MouseWheelMoved) && change_limit)
            {
                solver_.limit *= pow(2.0, static_cast<float>(event.mouseWheel.delta));
                DrawSet();
            }

            // Toggle action modes
            else if ((event.type == sf::Event::KeyPressed) && (!InputBoxesHasFocus()))
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
                solver_.zoom(static_cast<float>(event.mouseWheel.delta),
                             solver_.Screen2Plane(sf::Mouse::getPosition(window_)));
                DrawSet();
            }
            else if ((action_mode == ZOOMING) &&
                     (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right)))
            {
                if (GetZoomingFrame(zooming_frame))
                {
                    solver_.changeBorders(zooming_frame);
                    DrawSet();
                }
            }

            // Point tracing and sounding
            else if (((action_mode == POINT_TRACING) || (action_mode == SOUNDING)) &&
                     (event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
            {
                input_boxes_.x.is_visible_ = false;
                input_boxes_.x.has_focus_  = false;

                input_boxes_.y.is_visible_ = false;
                input_boxes_.y.has_focus_  = false;

                input_boxes_.z.is_visible_ = false;
                input_boxes_.z.has_focus_  = false;

                showing_trace = true;
                left_pressed  = true;

                if (action_mode == SOUNDING)
                {
                    synth_.SetPoint(solver_.Screen2Plane(sf::Mouse::getPosition(window_)));
                    synth_.audio_pause_ = false;
                    synth_.play();
                }
            }
            else if ((event.type == sf::Event::MouseButtonReleased) && (event.mouseButton.button == sf::Mouse::Left))
            {
                left_pressed = false;
            }
            else if (((action_mode == POINT_TRACING) || (action_mode == SOUNDING)) &&
                     (sf::Mouse::isButtonPressed(sf::Mouse::Right)))
            {
                showing_trace       = false;
                synth_.audio_pause_ = true;
                synth_.pause();
            }
        }

        window_.clear();
        window_.draw(shader_.sprite);

        if ((options_.input_mode == ComplexShader::Z_INPUT) && input_boxes_.z.is_visible_)
        {
            input_boxes_.z.draw(window_);
        }
        if ((options_.input_mode == ComplexShader::XY_INPUT) && input_boxes_.x.is_visible_)
        {
            input_boxes_.x.draw(window_);
            input_boxes_.y.draw(window_);
        }

        if (left_pressed)
        {
            c_point = solver_.Screen2Plane(sf::Mouse::getPosition(window_));
            orbit   = c_point;

            synth_.SetPoint(solver_.Screen2Plane(sf::Mouse::getPosition(window_)));
        }

        if (showing_trace)
            orbit = PointTrace(orbit, c_point);

        if (showing_menu)
            drawHelpMenu();

        window_.display();
    }

    synth_.stop();
}

void Puzabrot::updateWinSizes(size_t new_width, size_t new_height)
{
    solver_.updateWinSizes(new_width, new_height);
    shader_.updateWinSizes(new_width, new_height);
}

void Puzabrot::toggleFullScreen()
{
    if ((solver_.winsizes.x == sf::VideoMode::getDesktopMode().width) &&
        (solver_.winsizes.y == sf::VideoMode::getDesktopMode().height))
    {
        window_.create(sf::VideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT), TITLE_STRING);
        updateWinSizes(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
    else
    {
        window_.create(sf::VideoMode::getDesktopMode(), TITLE_STRING, sf::Style::Fullscreen);
        updateWinSizes(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
    }
}

bool Puzabrot::InputBoxesHasFocus()
{
    return (input_boxes_.x.has_focus_) || (input_boxes_.y.has_focus_) || (input_boxes_.z.has_focus_);
}

bool Puzabrot::InputBoxesIsVisible()
{
    return (input_boxes_.x.is_visible_) || (input_boxes_.y.is_visible_) || (input_boxes_.z.is_visible_);
}

void Puzabrot::DrawSet()
{
    shader_.draw(solver_, options_.draw_mode, options_.coloring);
}

int Puzabrot::GetZoomingFrame(Frame& frame)
{
    unsigned int w = solver_.winsizes.x;
    unsigned int h = solver_.winsizes.y;

    sf::Vector2i start(-1, -1);
    sf::Vector2i end(-1, -1);

    sf::RectangleShape rectangle;
    rectangle.setOutlineThickness(1);
    rectangle.setFillColor(sf::Color::Transparent);

    while (true)
    {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            start = sf::Mouse::getPosition(window_);
            rectangle.setPosition(sf::Vector2f(start));

            while (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                end = sf::Mouse::getPosition(window_) + sf::Vector2i(1, 1);

                if ((abs(end.x - start.x) > 8) && (abs(end.y - start.y) > 8))
                {
                    double sx = start.x;
                    double sy = start.y;
                    double ex = end.x;
                    double ey = end.y;

                    if (((end.y > start.y) && (end.x > start.x)) || ((end.y < start.y) && (end.x < start.x)))
                    {
                        end.x = static_cast<int>((w * h * (ey - sy) + w * w * ex + h * h * sx) / (w * w + h * h));
                        end.y = static_cast<int>((w * h * (ex - sx) + w * w * sy + h * h * ey) / (w * w + h * h));
                    }
                    else
                    {
                        end.x = static_cast<int>((w * h * (sy - ey) + w * w * ex + h * h * sx) / (w * w + h * h));
                        end.y = static_cast<int>((w * h * (sx - ex) + w * w * sy + h * h * ey) / (w * w + h * h));
                    }

                    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                    {
                        rectangle.setOutlineColor(sf::Color::Blue);
                        frame.zoom = static_cast<double>(w) * h / abs(end.x - start.x) / abs(end.y - start.y);
                    }
                    else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
                    {
                        rectangle.setOutlineColor(sf::Color::Red);
                        frame.zoom = static_cast<double>(abs(end.x - start.x)) * abs(end.y - start.y) / w / h;
                    }

                    rectangle.setSize(sf::Vector2f(end - start));

                    window_.draw(shader_.sprite);
                    window_.draw(rectangle);
                    window_.display();
                }
                else
                    end.x = 0;
            }
        }

        window_.draw(shader_.sprite);

        if (end.x != -1)
            break;
        return 0;
    }

    if (start.x > end.x)
    {
        frame.x2 = static_cast<unsigned int>(start.x);
        frame.x1 = static_cast<unsigned int>(end.x);
    }
    else
    {
        frame.x1 = static_cast<unsigned int>(start.x);
        frame.x2 = static_cast<unsigned int>(end.x);
    }

    if (start.y > end.y)
    {
        frame.y2 = static_cast<unsigned int>(start.y);
        frame.y1 = static_cast<unsigned int>(end.y);
    }
    else
    {
        frame.y1 = static_cast<unsigned int>(start.y);
        frame.y2 = static_cast<unsigned int>(end.y);
    }

    return 1;
}

void Puzabrot::initCalculator(Calculator& calc, point_t z, point_t c) const
{
    switch (options_.input_mode)
    {
    case ComplexShader::Z_INPUT:
    {
        calc.variables.push_back({ { c.x, c.y }, "c" });
        calc.variables.push_back({ { z.x, z.y }, "z" });
        break;
    }
    case ComplexShader::XY_INPUT:
    {
        calc.variables.push_back({ { c.x, 0 }, "cx" });
        calc.variables.push_back({ { c.y, 0 }, "cy" });

        calc.variables.push_back({ { z.x, 0 }, "x" });
        calc.variables.push_back({ { z.y, 0 }, "y" });
        break;
    }
    }
}

void Puzabrot::Mapping(Calculator& calc, point_t& mapped_point)
{
    switch (options_.input_mode)
    {
    case ComplexShader::Z_INPUT:
    {
        calc.Calculate(expr_trees_.z);
        calc.variables[calc.variables.size() - 1] = { expr_trees_.z.data.number, "z" };

        mapped_point = point_t(real(expr_trees_.z.data.number), imag(expr_trees_.z.data.number));
        break;
    }
    case ComplexShader::XY_INPUT:
    {
        calc.Calculate(expr_trees_.x);
        calc.Calculate(expr_trees_.y);

        calc.variables[calc.variables.size() - 2] = { { real(expr_trees_.x.data.number), 0.0 }, "x" };
        calc.variables[calc.variables.size() - 1] = { { real(expr_trees_.y.data.number), 0.0 }, "y" };

        mapped_point = point_t(real(expr_trees_.x.data.number), real(expr_trees_.y.data.number));
        break;
    }
    }
}

point_t Puzabrot::PointTrace(point_t point, point_t c_point)
{
    static Calculator calc;
    switch (options_.draw_mode)
    {
    case MAIN: initCalculator(calc, point, c_point); break;
    case JULIA: initCalculator(calc, point, solver_.julia_point); break;
    }

    point_t point1 = point;
    point_t point2(0.0, 0.0);

    for (size_t i = 0; (i < solver_.itrn_max) && (sqrt(pow(point2.x, 2.0) + pow(point2.y, 2.0)) < solver_.limit); ++i)
    {
        Mapping(calc, point2);

        sf::Vertex line[] = { sf::Vertex(sf::Vector2f(solver_.Plane2Screen(point1)), sf::Color::White),
                              sf::Vertex(sf::Vector2f(solver_.Plane2Screen(point2)), sf::Color::Black) };
        point1            = point2;

        window_.draw(line, 2, sf::Lines);
    }

    calc.clear();

    return point1;
}

void Puzabrot::savePicture()
{
    static int  shot_num = 0;
    std::string filename = "screenshot(" + std::to_string(shot_num++) + ")" + ".png";

    window_.draw(shader_.sprite);

    sf::RectangleShape rectangle;
    rectangle.setPosition(0, 0);
    rectangle.setSize(sf::Vector2f(solver_.winsizes));
    rectangle.setFillColor(sf::Color(10, 10, 10, 150));

    window_.draw(rectangle);
    window_.display();

    sf::Vector2u  screenshot_sizes(SCREENSHOT_WIDTH, static_cast<unsigned int>(static_cast<float>(SCREENSHOT_WIDTH) /
                                                                              static_cast<float>(solver_.winsizes.x) *
                                                                              static_cast<float>(solver_.winsizes.y)));
    ComplexSolver solver = solver_;
    solver.winsizes      = screenshot_sizes;

    ComplexShader shader(screenshot_sizes);
    shader.make(expr_trees_, options_.input_mode);
    shader.draw(solver, options_.draw_mode, options_.coloring);

    sf::Texture screen = shader.render_texture.getTexture();
    screen.copyToImage().saveToFile(filename);

    window_.draw(shader_.sprite);
    window_.display();
}

void Puzabrot::drawHelpMenu()
{
    sf::RectangleShape dim_rect(sf::Vector2f(solver_.winsizes));
    dim_rect.setFillColor(sf::Color(0, 0, 0, 128));
    window_.draw(dim_rect);

    sf::Text help_menu;
    help_menu.setFont(font_);
    help_menu.setCharacterSize(22);
    help_menu.setPosition(10.0F, 10.0F);
    help_menu.setFillColor(sf::Color::White);

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
            "        Borders: upper: %.5lf, bottom: %.5lf, left: %.5lf, right: %.5lf\n",
            solver_.itrn_max, solver_.limit, solver_.borders.im_top, solver_.borders.im_bottom, solver_.borders.re_left,
            solver_.borders.re_right);

    help_menu.setString(str);
    window_.draw(help_menu);
}

int Puzabrot::makeShader()
{
    switch (options_.input_mode)
    {
    case ComplexShader::Z_INPUT:
    {
        Expression expr_z(input_boxes_.z.getInput());

        int err = expr_z.getTree(expr_trees_.z);
        if (err)
            return err;
        break;
    }
    case ComplexShader::XY_INPUT:
    {
        Expression expr_x(input_boxes_.x.getInput());
        Expression expr_y(input_boxes_.y.getInput());

        int err = expr_x.getTree(expr_trees_.x);
        if (err)
            return err;

        err = expr_y.getTree(expr_trees_.y);
        if (err)
            return err;
    }
    }

    if (shader_.make(expr_trees_, options_.input_mode))
        return CALC_WRONG_VARIABLE;

    return 0;
}

Puzabrot::Synth::Synth(Puzabrot* app) :
    audio_reset_(true), audio_pause_(false), sustain_(true), volume_(8000.0), app_(app), point_(point_t(0.0, 0.0)),
    c_point_(point_t(0.0, 0.0)), new_point_(point_t(0.0, 0.0)), prev_point_(point_t(0.0, 0.0))
{
    initialize(2, SAMPLE_RATE);
    setLoop(true);

    updateCalc();
}

void Puzabrot::Synth::updateCalc()
{
    calc_.variables.clear();
    app_->initCalculator(calc_, point_, c_point_);
}

void Puzabrot::Synth::SetPoint(point_t point)
{
    new_point_ = point;

    audio_reset_ = true;
    audio_pause_ = false;
}

bool Puzabrot::Synth::onGetData(Chunk& data)
{
    data.samples     = m_samples;
    data.sampleCount = AUDIO_BUFF_SIZE;
    memset(m_samples, 0, AUDIO_BUFF_SIZE * sizeof(int16_t));

    if (audio_reset_)
    {
        m_audio_time = 0;

        switch (app_->options_.draw_mode)
        {
        case MAIN: c_point_ = new_point_; break;
        case JULIA: c_point_ = app_->solver_.julia_point; break;
        }

        point_      = new_point_;
        prev_point_ = new_point_;

        mean_x  = new_point_.x;
        mean_y  = new_point_.y;
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

            updateCalc();
            app_->Mapping(calc_, point_);

            if (sqrt(pow(point_.x, 2.0) + pow(point_.y, 2.0)) > app_->solver_.limit)
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

        double t = 0.5 - 0.5 * cos(double(j) / double(steps) * real(PI));

        double wx = t * dx + (1.0 - t) * dpx;
        double wy = t * dy + (1.0 - t) * dpy;

        m_samples[i]     = static_cast<int16_t>(std::min(std::max(wx * volume_, -32000.0), 32000.0));
        m_samples[i + 1] = static_cast<int16_t>(std::min(std::max(wy * volume_, -32000.0), 32000.0));

        m_audio_time += 1;
    }

    return !audio_reset_;
}

} // namespace puza