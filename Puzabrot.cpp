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
                case MAIN: DrawSet();  break;
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
                case MAIN: DrawSet();  break;
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

                switch (drawing_mode)
                {
                case MAIN: DrawSet();  break;
                case JULIA: DrawJulia(julia_point); break;
                }
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

                if (!err) DrawSet();

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
                        julia_point = sf::Vector2f((float)sf::Mouse::getPosition(*window_).x / winsizes_.x * (borders_.Re_right - borders_.Re_left) + borders_.Re_left,
                                                   (float)sf::Mouse::getPosition(*window_).y / winsizes_.y * (borders_.Im_up    - borders_.Im_down) + borders_.Im_down );

                        DrawJulia(julia_point);
                    }
                    drawing_mode = JULIA;
                }
                else
                {
                    DrawSet();
                    drawing_mode = MAIN;
                }
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
                    case MAIN: DrawSet();  break;
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
                    PointTrace(sf::Mouse::getPosition(*window_));
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
    shader_.setUniform("borders",  sf::Glsl::Vec4((float)borders_.Re_left, (float)borders_.Re_right, (float)borders_.Im_down, (float)borders_.Im_up));
    shader_.setUniform("winsizes", sf::Glsl::Ivec2(winsizes_.x, winsizes_.y));

    shader_.setUniform("itrn_max", (int)itrn_max_);
    shader_.setUniform("limit",    (float)lim_);

    shader_.setUniform("drawing_mode", MAIN);

    render_texture_.draw(sprite_, &shader_);
}

//------------------------------------------------------------------------------

void Puzabrot::DrawJulia (sf::Vector2f point)
{
    shader_.setUniform("borders", sf::Glsl::Vec4((float)borders_.Re_left, (float)borders_.Re_right, (float)borders_.Im_down, (float)borders_.Im_up));
    shader_.setUniform("winsizes", sf::Glsl::Ivec2(winsizes_.x, winsizes_.y));

    shader_.setUniform("itrn_max", (int)itrn_max_);
    shader_.setUniform("limit", (float)lim_);

    shader_.setUniform("drawing_mode", JULIA);

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
    double releft  = borders_.Re_left;
    double reright = borders_.Re_right;
    double imup    = borders_.Im_up;
    double imdown  = borders_.Im_down;

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

void Puzabrot::PointTrace (sf::Vector2i point)
{
    double re0 = borders_.Re_left + (borders_.Re_right - borders_.Re_left) * point.x / winsizes_.x;
    double im0 = borders_.Im_down + (borders_.Im_up    - borders_.Im_down) * point.y / winsizes_.y;

    double x1 = re0;
    double y1 = im0;

    static Calculator calc;
    calc.trees_[0] = expr_tree_;

    calc.variables_.Push({ {re0, im0}, "c" });
    calc.variables_.Push({ {x1,  y1 }, "z" });

    calc.trees_[0].root_->setData({ {x1, y1}, calc.trees_[0].root_->getData().word, calc.trees_[0].root_->getData().op_code, calc.trees_[0].root_->getData().node_type });

    for (int i = 0; (i < itrn_max_) && (abs(calc.trees_[0].root_->getData().number) < lim_); ++i)
    {
        calc.Calculate(calc.trees_[0].root_, false);
        calc.variables_[calc.variables_.getSize() - 1] = { calc.trees_[0].root_->getData().number, "z" };

        double x2 = real(calc.trees_[0].root_->getData().number);
        double y2 = imag(calc.trees_[0].root_->getData().number);

        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f((x1 - borders_.Re_left) / (borders_.Re_right - borders_.Re_left) * winsizes_.x,
                                    (y1 - borders_.Im_down) / (borders_.Im_up    - borders_.Im_down) * winsizes_.y), sf::Color::White),

            sf::Vertex(sf::Vector2f((x2 - borders_.Re_left) / (borders_.Re_right - borders_.Re_left) * winsizes_.x,
                                    (y2 - borders_.Im_down) / (borders_.Im_up    - borders_.Im_down) * winsizes_.y), sf::Color::Black)
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
    if (err) return err;

    char* str_shader = writeShader();

    shader_.loadFromMemory(str_shader, sf::Shader::Fragment);

    delete [] str_shader;
    delete [] expr;

    return 0;
}

//------------------------------------------------------------------------------

char* Puzabrot::writeShader ()
{
    char* str_shader = new char[10000] {};

    char* str_calculation = Tree2GLSL();

    sprintf(str_shader,
       "#version 400 compatibility\n"
       "\n"
       "uniform vec4  borders;\n"
       "uniform ivec2 winsizes;\n"
       "uniform int   itrn_max;\n"
       "uniform float limit;\n"
       "uniform int   drawing_mode;\n"
       "uniform vec2  julia_point;\n"
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
       "    double re0 = borders.x + (borders.y - borders.x) * gl_FragCoord.x / winsizes.x;\n"
       "    double im0 = borders.z + (borders.w - borders.z) * gl_FragCoord.y / winsizes.y;\n"
       "\n"
       "    dvec2 z = dvec2(re0, im0);\n"
       "    dvec2 c;\n"
       "    if (drawing_mode == 0)"
       "        c = dvec2(re0, im0);\n"
       "    else if (drawing_mode == 1)\n"
       "        c = dvec2(julia_point.x, julia_point.y);\n"
       "\n"
       "    int itrn = 0;\n"
       "    for (itrn = 0; itrn < itrn_max; ++itrn)\n"
       "    {\n"
       "        %s\n"
       "        \n"
       "    if (dot(z, z) > limit) break;\n"
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

char* Puzabrot::Tree2GLSL()
{
    char* str_calculation = new char[1000]{};

    sprintf(str_calculation,
        "z = dvec2(z.x*z.x - z.y*z.y, 2*z.x*z.y) + c;\n"
        );

    return str_calculation;
}

//------------------------------------------------------------------------------
