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

Puzabrot::Puzabrot() :
    winsizes_  ({ DEFAULT_WIDTH, DEFAULT_HEIGHT }),
    input_box_ (sf::Vector2f(10, 10), sf::Color(128, 128, 128, 128), sf::Color::White, 20),
    expr_tree_ ((char*)"Expression tree")
{
    window_ = new sf::RenderWindow(sf::VideoMode(winsizes_.x, winsizes_.y), title_string);
    
    borders_.Im_up   =  UPPER_BORDER;
    borders_.Im_down = -UPPER_BORDER;

    borders_.Re_left  = -(borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *3;
    borders_.Re_right =  (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *2;

    input_box_.setInput(sf::String("z^2+c"));

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

    makeShader();

    DrawSet();

    int action_mode = ZOOMING;
    int drawing_mode = MAIN;

    bool julia_dragging = false;

    sf::Vector2f julia_point = sf::Vector2f(0, 0);

    while (window_->isOpen())
    {
        sf::Event event;
        Screen newscreen = {};
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

                switch (drawing_mode)
                {
                case MAIN:  DrawSet();              break;
                case JULIA: DrawJulia(julia_point); break;
                }
            }

            //Resize window
            else if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window_->setView(sf::View(visibleArea));
                updateWinSizes(window_->getSize().x, window_->getSize().y);

                switch (drawing_mode)
                {
                case MAIN:  DrawSet();              break;
                case JULIA: DrawJulia(julia_point); break;
                }
            }

            //Reset set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R) && (not input_box_.has_focus_))
            {
                borders_.Im_up   =  UPPER_BORDER;
                borders_.Im_down = -UPPER_BORDER;

                borders_.Re_left  = -(borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *3;
                borders_.Re_right =  (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *2;

                itrn_max_ = MAX_ITERATION;
                lim_      = LIMIT;

                DrawSet();
                drawing_mode = MAIN;
            }

            //Toggle input box visibility
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::I) && (not input_box_.has_focus_))
            {
                input_box_.is_visible_ = 1 - input_box_.is_visible_;
                if (not input_box_.is_visible_)
                    input_box_.has_focus_ = false;
            }

            //Toggle input box focus
            else if (input_box_.is_visible_ && (event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
            {
                if ((input_box_.getPos().x < event.mouseButton.x) && (event.mouseButton.x < input_box_.getPos().x + input_box_.getSize().x) &&
                    (input_box_.getPos().y < event.mouseButton.y) && (event.mouseButton.y < input_box_.getPos().y + input_box_.getSize().y))
                    input_box_.has_focus_ = true;
                else
                    input_box_.has_focus_ = false;
            }

            //Input text expression to input box
            else if (input_box_.has_focus_ && (event.type == sf::Event::TextEntered) && (event.text.unicode < 128))
            {
                input_box_.setInput(event.text.unicode);
            }

            //Enter expression from input box
            else if (input_box_.has_focus_ && (event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Enter))
            {
                int err = makeShader();

                if (!err)
                {
                    DrawSet();
                    drawing_mode = MAIN;
                }

                if (err)
                    input_box_.setOutput(sf::String(calc_errstr[err + 1]));
                else
                    input_box_.setOutput(sf::String());
            }

            //Julia set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::J))
            {
                if (drawing_mode != JULIA)
                {
                    while (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
                    {
                        julia_point = sf::Vector2f(borders_.Re_left + (borders_.Re_right - borders_.Re_left) * (float)sf::Mouse::getPosition(*window_).x / winsizes_.x,
                                                   borders_.Im_up   - (borders_.Im_up    - borders_.Im_down) * (float)sf::Mouse::getPosition(*window_).y / winsizes_.y);

                        DrawJulia(julia_point);
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
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::J))
            {
                if (not julia_dragging)
                    drawing_mode = MAIN;
                else
                    drawing_mode = JULIA;
            }

            //Toggle action modes
            else if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Z: action_mode = ZOOMING;       break;
                case sf::Keyboard::P: action_mode = POINT_TRACING; break;
                case sf::Keyboard::S: action_mode = SOUNDING;      break;
                }
            }

            //Zooming
            else if ((action_mode == ZOOMING) && (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right)))
            {
                if (GetNewScreen(newscreen))
                {
                    changeBorders(newscreen);

                    if (newscreen.zoom > 1)
                        itrn_max_ = (int)(itrn_max_*(1 + newscreen.zoom/DELTA_ZOOM));
                    else
                        itrn_max_ = (int)(itrn_max_*(1 - 1/(newscreen.zoom*DELTA_ZOOM + 1)));

                    switch (drawing_mode)
                    {
                    case MAIN:  DrawSet();              break;
                    case JULIA: DrawJulia(julia_point); break;
                    }
                }
            }

            //Point tracing
            else if ((action_mode == POINT_TRACING) && (sf::Mouse::isButtonPressed(sf::Mouse::Left)))
            {
                while (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    input_box_.is_visible_ = false;
                    input_box_.has_focus_  = false;

                    window_->clear();
                    window_->draw(sprite_);

                    switch (drawing_mode)
                    {
                    case MAIN:
                        PointTrace(sf::Mouse::getPosition(*window_), sf::Vector2f(NAN, NAN));
                        break;
                    case JULIA:
                        PointTrace(sf::Mouse::getPosition(*window_), julia_point);
                        break;
                    }
                }
            }

            /*
            //Sounding
            else if (mode == SOUNDING)
            {
            }
            */
        }

        window_->clear();
        window_->draw(sprite_);

        if (input_box_.is_visible_)
            input_box_.draw(window_);
        else
            window_->display();
    }
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

void Puzabrot::DrawSet ()
{
    shader_.setUniform("borders",  sf::Glsl::Vec4(borders_.Re_left, borders_.Re_right, borders_.Im_down, borders_.Im_up));
    shader_.setUniform("winsizes", sf::Glsl::Ivec2(winsizes_.x, winsizes_.y));

    shader_.setUniform("itrn_max", (int)itrn_max_);
    shader_.setUniform("limit",    (float)lim_);

    shader_.setUniform("drawing_mode", (int)MAIN);

    render_texture_.draw(sprite_, &shader_);
}

//------------------------------------------------------------------------------

void Puzabrot::DrawJulia (sf::Vector2f point)
{
    shader_.setUniform("borders", sf::Glsl::Vec4(borders_.Re_left, borders_.Re_right, borders_.Im_down, borders_.Im_up));
    shader_.setUniform("winsizes", sf::Glsl::Ivec2(winsizes_.x, winsizes_.y));

    shader_.setUniform("itrn_max", (int)itrn_max_);
    shader_.setUniform("limit",    (float)lim_);

    shader_.setUniform("drawing_mode", (int)JULIA);

    shader_.setUniform("julia_point", sf::Glsl::Vec2(point.x, point.y));

    render_texture_.draw(sprite_, &shader_);
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

void Puzabrot::PointTrace (sf::Vector2i point, sf::Vector2f julia_point)
{
    float re0 = borders_.Re_left + (borders_.Re_right - borders_.Re_left) * point.x / winsizes_.x;
    float im0 = borders_.Im_up   - (borders_.Im_up    - borders_.Im_down) * point.y / winsizes_.y;

    float x1 = 0;
    float y1 = 0;
    
    if (isnan(julia_point.x) || isnan(julia_point.y))
    {
        x1 = re0;
        y1 = im0;
    }
    else
    {
        x1 = julia_point.x;
        y1 = julia_point.y;
    }

    static Calculator calc;
    calc.trees_[0] = expr_tree_;

    calc.variables_.Push({ {re0, im0}, "c" });
    calc.variables_.Push({ {x1,  y1 }, "z" });

    calc.trees_[0].root_->setData({ {x1, y1}, calc.trees_[0].root_->getData().word, calc.trees_[0].root_->getData().op_code, calc.trees_[0].root_->getData().node_type });

    for (int i = 0; (i < itrn_max_) && (abs(calc.trees_[0].root_->getData().number) < lim_); ++i)
    {
        calc.Calculate(calc.trees_[0].root_, false);
        calc.variables_[calc.variables_.getSize() - 1] = { calc.trees_[0].root_->getData().number, "z" };

        float x2 = real(calc.trees_[0].root_->getData().number);
        float y2 = imag(calc.trees_[0].root_->getData().number);

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
    }

    window_->display();
    calc.variables_.Clean();
    ADD_VAR(calc.variables_);
}

//------------------------------------------------------------------------------

int Puzabrot::makeShader ()
{
    std::string string = input_box_.getInput().toAnsiString();
    char* str = (char*)string.c_str();

    char* expr = new char[MAX_STR_LEN] {};
    strcpy(expr, str);
    Expression expression = { expr, expr, CALC_OK };

    int err = Expr2Tree(expression, expr_tree_);
    if (err) return expression.err;

    char* str_shader = writeShader();
    if (str_shader == nullptr) return CALC_WRONG_VARIABLE;

    shader_.loadFromMemory(str_shader, sf::Shader::Fragment);

    delete [] str_shader;
    delete [] expr;

    return 0;
}

//------------------------------------------------------------------------------

char* Puzabrot::writeShader ()
{
    char* str_calculation = new char[1000] {};
    int err = Tree2GLSL(expr_tree_.root_, str_calculation);
    if (err)
    {
        delete [] str_calculation;
        return nullptr;
    }

    char* str_shader = new char[8000] {};

    sprintf(str_shader,
        "#version 400 compatibility\n"
        "\n"
        "const float NIL = 1e-9;\n"
        "const float PI  = atan(1) * 4;\n"
        "const float E   = exp(1);\n"
        "const vec2  I   = vec2(0, 1);\n"
        "const vec2  ONE = vec2(1, 0);\n"
        "\n"
        "uniform vec4  borders;\n"
        "uniform ivec2 winsizes;\n"
        "uniform int   itrn_max;\n"
        "uniform float limit;\n"
        "uniform int   drawing_mode;\n"
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
        "    return vec2(ln.x/log(10), ln.y/log(10));\n"
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
        "vec3 getColor(int itrn)\n"
        "{\n"
        "    if (itrn < itrn_max)\n"
        "    {\n"
        "        itrn = itrn * 4 % 1530;\n"
        "             if (itrn < 256)  return vec3( 255,         itrn,        0           );\n"
        "        else if (itrn < 511)  return vec3( 510 - itrn,  255,         0           );\n"
        "        else if (itrn < 766)  return vec3( 0,           255,         itrn - 510  );\n"
        "        else if (itrn < 1021) return vec3( 0,           1020 - itrn, 255         );\n"
        "        else if (itrn < 1276) return vec3( itrn - 1020, 0,           255         );\n"
        "        else if (itrn < 1530) return vec3( 255,         0,           1529 - itrn );\n"
        "    }\n"
        "    return vec3( 0, 0, 0 );\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    float re0 = borders.x + (borders.y - borders.x) * gl_FragCoord.x / winsizes.x;\n"
        "    float im0 = borders.w - (borders.w - borders.z) * gl_FragCoord.y / winsizes.y;\n"
        "\n"
        "    vec2 z = vec2(re0, im0);\n"
        "    vec2 c;\n"
        "    if (drawing_mode == 0)"
        "        c = vec2(re0, im0);\n"
        "    else if (drawing_mode == 1)\n"
        "        c = vec2(julia_point.x, julia_point.y);\n"
        "\n"
        "    int itrn = 0;\n"
        "    for (itrn = 0; itrn < itrn_max; ++itrn)\n"
        "    {\n"
        "        z = %s;\n"
        "        \n"
        "    if (cabs(z) > limit) break;\n"
        "    }\n"
        "\n"
        "    vec3 col = getColor(itrn);\n"
        "    col = vec3(col.x / 255, col.y / 255, col.z / 255);\n"
        "    gl_FragColor = vec4(col, 1.0);\n"
       "}", str_calculation);

    delete [] str_calculation;

    return str_shader;
}

//------------------------------------------------------------------------------

int Puzabrot::Tree2GLSL (Node<CalcNodeData>* node_cur, char* str_cur)
{
    assert(node_cur != nullptr);
    assert(str_cur  != nullptr);

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
        if ((strcmp(node_cur->getData().word, "z") != 0) && (strcmp(node_cur->getData().word, "c") != 0) && (strcmp(node_cur->getData().word, "i") != 0))
            return -1;

        if (strcmp(node_cur->getData().word, "i") == 0)
            sprintf(str_cur, "vec2(1, 0)");
        else
            sprintf(str_cur, "%s", node_cur->getData().word);

        break;
    }
    case NODE_NUMBER:
    {
        sprintf(str_cur, "vec2(%f, %f)", real(node_cur->getData().number), imag(node_cur->getData().number));

        break;
    }
    default: assert(0);
    }

    return 0;
}

//------------------------------------------------------------------------------
