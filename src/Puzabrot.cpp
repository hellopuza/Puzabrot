#include "Puzabrot.h"

#include <cstring>

#define COND_RETURN(cond, ret) \
    if (cond)                  \
    {                          \
        return (ret);          \
    } //

constexpr double LIMIT = 100.0;
constexpr double UPPER_BORDER = 1.3;
constexpr double ZOOMING_RATIO = 0.2;
constexpr size_t MAX_ITERATION = 500;
constexpr unsigned SYM_SIZE = 12;
constexpr float FONT_SIZE = 20.0F;
constexpr size_t SCREENSHOT_WIDTH = 7680;

static const sf::String TITLE_STRING = "Puzabrot";
static const sf::Color AXIS_COLOR = sf::Color(180, 180, 180);
static const sf::Color GRID_COLOR = sf::Color(100, 100, 100);
static const sf::Color TEXT_COLOR = sf::Color(180, 180, 180);
static const sf::Color INPUT_BOX_COLOR = sf::Color(128, 128, 128, 128);
static const sf::Vector2f INPUT_X_POS = sf::Vector2f(10.0F, 10.0F);
static const sf::Vector2f INPUT_Y_POS = sf::Vector2f(10.0F, 50.0F);

Puzabrot::Puzabrot(const sf::Vector2u & size) :
    input_{ InputBox(INPUT_X_POS, INPUT_BOX_COLOR, sf::Color::White, FONT_SIZE),
            InputBox(INPUT_Y_POS, INPUT_BOX_COLOR, sf::Color::White, FONT_SIZE),
            InputBox(INPUT_X_POS, INPUT_BOX_COLOR, sf::Color::White, FONT_SIZE) },
    window_(std::make_unique<Window>(size, &font_)),
    engine_(std::make_unique<Engine>(size, window_.get())),
    synth_(std::make_unique<Synth>(engine_.get()))
{
    engine_->params.limit = LIMIT;
    engine_->params.itrn_max = MAX_ITERATION;

    window_->setZoomingRatio(ZOOMING_RATIO);
    window_->setBorders(-UPPER_BORDER, UPPER_BORDER, 0.5);

    input_.x.setLabel(sf::String("x:"));
    input_.y.setLabel(sf::String("y:"));
    input_.z.setLabel(sf::String("z:"));

    input_.x.setInput(sf::String("x*x-y*y+cx"));
    input_.y.setInput(sf::String("2*x*y+cy"));
    input_.z.setInput(sf::String("z^2+c"));

    font_.loadFromFile("assets/consola.ttf");
}

void Puzabrot::run()
{
    window_->setVerticalSyncEnabled(true);

    makeShader();
    engine_->render();

    bool showing_grid = false;
    bool showing_menu = false;
    bool showing_trace = false;
    bool julia_dragging = false;
    bool right_pressed = false;
    bool change_iter = false;
    bool change_limit = false;

    point_t orbit(0.0, 0.0);
    point_t c_point(0.0, 0.0);

    while (window_->isOpen())
    {
        bool was_screenshot = false;

        sf::Event event;
        while (window_->pollEvent(event))
        {
            switch (engine_->options.input_mode)
            {
            case Z_INPUT:
            {
                input_.z.handleEvent(event);
                break;
            }
            case XY_INPUT:
            {
                input_.x.handleEvent(event);
                input_.y.handleEvent(event);
                break;
            }
            }

            // Enter expression from input box
            if (input_.x.TextEntered() || input_.y.TextEntered() || input_.z.TextEntered())
            {
                int err = makeShader();

                if (!err)
                {
                    engine_->render();
                    engine_->options.draw_mode = MAIN;
                }

                switch (engine_->options.input_mode)
                {
                case Z_INPUT:
                {
                    if (err)
                    {
                        input_.z.setOutput(sf::String(calc_errstr[err + 1]));
                    }
                    else
                    {
                        input_.z.setOutput(sf::String());
                    }
                    break;
                }
                case XY_INPUT:
                {
                    if (input_.x.TextEntered())
                    {
                        if (err)
                        {
                            const sf::Vector2f input_y_pos(input_.y.getPosition().x, input_.y.getPosition().y + 0.5F * input_.x.getSize().y);

                            input_.x.setOutput(sf::String(calc_errstr[err + 1]));
                            input_.y.setPosition(input_y_pos);
                        }
                        else
                        {
                            input_.x.setOutput(sf::String());
                            input_.y.setPosition(INPUT_Y_POS);
                        }
                    }
                    else if (err)
                    {
                        input_.y.setOutput(sf::String(calc_errstr[err + 1]));
                    }
                    else
                    {
                        input_.y.setOutput(sf::String());
                    }
                    break;
                }
                }
            }

            // Reset set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R) && !textEntering())
            {
                engine_->options.draw_mode = MAIN;
                engine_->params.limit = LIMIT;
                engine_->params.itrn_max = MAX_ITERATION;
                window_->setBorders(-UPPER_BORDER, UPPER_BORDER, 0.5);
                engine_->render();
            }

            // Take a screenshot
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space) && !was_screenshot && !textEntering())
            {
                savePicture();
                was_screenshot = true;
            }

            // Toggle audio dampening
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::D) && !textEntering())
            {
                synth_->sustain = !synth_->sustain;
            }

            // Toggle sound coloring
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::C) && !textEntering())
            {
                engine_->options.coloring = !engine_->options.coloring;
                engine_->render();
            }

            // Toggle help menu showing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::H) && !textEntering())
            {
                showing_menu = !showing_menu;
            }

            // Toggle grid showing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::G) && !textEntering())
            {
                showing_grid = !showing_grid;
            }

            // Toggle sound mode
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::S) && !textEntering())
            {
                engine_->options.sound_mode = !engine_->options.sound_mode;
            }

            // Toggle input mode
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tab))
            {
                switch (engine_->options.input_mode)
                {
                case Z_INPUT:
                {
                    if (input_.z.isVisible())
                    {
                        input_.x.show();
                        input_.y.show();
                    }
                    input_.z.hide();

                    engine_->options.input_mode = XY_INPUT;
                    break;
                }
                case XY_INPUT:
                {
                    if (input_.x.isVisible())
                    {
                        input_.z.show();
                    }
                    input_.x.hide();
                    input_.y.hide();

                    engine_->options.input_mode = Z_INPUT;
                    break;
                }
                }

                makeShader();
            }

            // Julia set drawing
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::J) && !textEntering())
            {
                if (engine_->options.draw_mode != JULIA)
                {
                    while (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
                    {
                        engine_->params.julia_point = window_->Screen2Base(sf::Mouse::getPosition(*window_));

                        engine_->options.draw_mode = JULIA;
                        engine_->render();
                        engine_->options.draw_mode = MAIN;

                        window_->draw(engine_->getOutput());
                        window_->display();

                        julia_dragging = true;
                    }
                }
                else
                {
                    engine_->render();
                    julia_dragging = false;
                }
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::J) && !textEntering())
            {
                engine_->options.draw_mode = !julia_dragging ? MAIN : JULIA;

                engine_->render();
            }

            // Change max iterations
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::I) && !textEntering())
            {
                change_iter = true;
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::I) && !textEntering())
            {
                change_iter = false;
            }
            else if ((event.type == sf::Event::MouseWheelMoved) && change_iter)
            {
                engine_->params.itrn_max += static_cast<size_t>(event.mouseWheel.delta * 50);
                engine_->render();
            }

            // Change limit
            else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::L) && !textEntering())
            {
                change_limit = true;
            }
            else if ((event.type == sf::Event::KeyReleased) && (event.key.code == sf::Keyboard::L) && !textEntering())
            {
                change_limit = false;
            }
            else if ((event.type == sf::Event::MouseWheelMoved) && change_limit)
            {
                engine_->params.limit *= pow(2.0, static_cast<float>(event.mouseWheel.delta));
                engine_->render();
            }

            // Point tracing and sounding
            else if ((event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Right))
            {
                input_.x.hide();
                input_.y.hide();
                input_.z.hide();

                showing_trace = true;
                right_pressed  = true;

                if (engine_->options.sound_mode)
                {
                    synth_->audio_pause = false;
                    synth_->setPoint(window_->Screen2Base(sf::Mouse::getPosition(*window_)));
                    synth_->play();
                }
                else
                {
                    synth_->pause();
                }
            }
            else if ((event.type == sf::Event::MouseButtonReleased) && (event.mouseButton.button == sf::Mouse::Right))
            {
                right_pressed = false;
            }
            else if ((event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Middle))
            {
                showing_trace = false;
                synth_->audio_pause = true;
                synth_->pause();
            }

            else if (window_->handleEvent(event))
            {
                engine_->setImageSize(window_->getSize());
                engine_->render();
            }
        }

        window_->clear();
        window_->draw(engine_->getOutput());

        if (showing_grid)
        {
            window_->draw(*window_);
        }

        input_.x.draw(*window_);
        input_.y.draw(*window_);
        input_.z.draw(*window_);

        if (right_pressed)
        {
            c_point = window_->Screen2Base(sf::Mouse::getPosition(*window_));
            orbit   = c_point;

            synth_->setPoint(c_point);
        }

        if (showing_trace)
        {
            orbit = PointTrace(orbit, c_point);
        }

        if (showing_menu)
        {
            drawHelpMenu();
        }

        window_->display();
    }

    synth_->stop();
}

Puzabrot::point_t Puzabrot::PointTrace(point_t point, point_t c_point)
{
    Calculator calc;
    engine_->initCalculator(calc, point, (engine_->options.draw_mode == MAIN) ? c_point : engine_->params.julia_point);

    point_t point1 = point;
    point_t point2;

    for (size_t i = 0; i < engine_->params.itrn_max; ++i)
    {
        engine_->Mapping(calc, engine_->expr_trees, point2);

        if (sqrt(pow(point2.x, 2.0) + pow(point2.y, 2.0)) > engine_->params.limit)
        {
            break;
        }

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(window_->Base2Screen(point1)), sf::Color::White),
            sf::Vertex(sf::Vector2f(window_->Base2Screen(point2)), sf::Color::Black)
        };

        point1 = point2;

        static_cast<sf::RenderTarget*>(window_.get())->draw(line, 2, sf::Lines);
    }

    return point1;
}

void Puzabrot::savePicture()
{
    static int  shot_num = 0;
    std::string filename = "screenshot(" + std::to_string(shot_num++) + ")" + ".png";

    window_->draw(engine_->getOutput());

    sf::RectangleShape rectangle;
    rectangle.setPosition(0.0F, 0.0F);
    rectangle.setSize(sf::Vector2f(window_->getSize()));
    rectangle.setFillColor(sf::Color(10, 10, 10, 150));

    window_->draw(rectangle);
    window_->display();

    sf::Vector2u screenshot_sizes(SCREENSHOT_WIDTH, static_cast<unsigned int>(static_cast<float>(SCREENSHOT_WIDTH) /
        static_cast<float>(window_->getSize().x) * static_cast<float>(window_->getSize().y)));

    Engine engine(screenshot_sizes, window_.get());
    engine.options = engine_->options;
    engine.expr_trees = engine_->expr_trees;
    engine.params = engine_->params;
    engine.makeShader();
    engine.render();

    engine.getOutput().getTexture()->copyToImage().saveToFile(filename);

    window_->draw(engine_->getOutput());
    window_->display();
}

void Puzabrot::drawHelpMenu()
{
    sf::RectangleShape dim_rect(sf::Vector2f(window_->getSize()));
    dim_rect.setFillColor(sf::Color(0, 0, 0, 128));
    window_->draw(dim_rect);

    sf::Text help_menu;
    help_menu.setFont(font_);
    help_menu.setCharacterSize(22);
    help_menu.setPosition(10.0F, 10.0F);
    help_menu.setFillColor(sf::Color::White);

    char str[1000] = "";

    sprintf(str,
        "    H - Toggle help menu viewing\n"
        "    G - Toggle grid viewing\n"
        "    S - Toggle sound mode        (press left mouse button to trace point and hear sound)\n"
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
        "        Current max iteration: %lu, current limit: %f\n",
        engine_->params.itrn_max, engine_->params.limit
    );

    help_menu.setString(str);
    window_->draw(help_menu);
}

int Puzabrot::makeShader()
{
    switch (engine_->options.input_mode)
    {
    case Z_INPUT:
    {
        Expression expr_z(input_.z.getInput());

        int err = expr_z.getTree(engine_->expr_trees.z);
        COND_RETURN(err, err);
        break;
    }
    case XY_INPUT:
    {
        Expression expr_x(input_.x.getInput());
        Expression expr_y(input_.y.getInput());

        int err = expr_x.getTree(engine_->expr_trees.x);
        COND_RETURN(err, err);

        err = expr_y.getTree(engine_->expr_trees.y);
        COND_RETURN(err, err);
    }
    }
    synth_->setExpressions(engine_->expr_trees);

    COND_RETURN(engine_->makeShader(), CALC_WRONG_VARIABLE);

    return 0;
}

bool Puzabrot::textEntering() const
{
    return input_.x.hasFocus() || input_.y.hasFocus() || input_.z.hasFocus();
}

Puzabrot::Window::Window(const sf::Vector2u& size, const sf::Font* font) : Engine2D(size, TITLE_STRING), font_(font) {}

void Puzabrot::Window::draw(const sf::Drawable& drawable, const sf::RenderStates& states)
{
    static_cast<sf::RenderTarget*>(this)->draw(drawable, states);
}

void Puzabrot::Window::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    drawGrid(target, states, *font_, GRID_COLOR, TEXT_COLOR, SYM_SIZE);
    drawAxes(target, states, AXIS_COLOR);
}

Puzabrot::Engine::Engine(const sf::Vector2u& image_size, const Window* window_) : ShaderEngine(image_size), window(window_) {}

void Puzabrot::Engine::render()
{
    shader_.setUniform("borders.left", static_cast<float>(window->getBorders().left));
    shader_.setUniform("borders.right", static_cast<float>(window->getBorders().right));
    shader_.setUniform("borders.bottom", static_cast<float>(window->getBorders().bottom));
    shader_.setUniform("borders.top", static_cast<float>(window->getBorders().top));

    shader_.setUniform("winsizes", sf::Glsl::Ivec2(sf::Vector2i(render_texture_.getSize())));

    shader_.setUniform("itrn_max", static_cast<int>(params.itrn_max));
    shader_.setUniform("limit", static_cast<float>(params.limit));

    shader_.setUniform("drawing_mode", options.draw_mode);
    shader_.setUniform("coloring", options.coloring);

    shader_.setUniform("julia_point", sf::Glsl::Vec2(sf::Vector2f(params.julia_point)));

    render_texture_.draw(sprite_, &shader_);
}

int Puzabrot::Engine::makeShader()
{
    const char* str_initialization = writeInitialization();

    std::string str_calculation = writeCalculation();
    COND_RETURN(str_calculation.empty(), -1);

    std::string str_checking = writeChecking();

    std::string str_shader =
        "#version 130\n"
        "\n"
        "const float NIL = 1e-9;\n"
        "const float PI  = atan(1.0) * 4.0;\n"
        "const float E   = exp(1.0);\n"
        "const vec2  I   = vec2(0.0, 1.0);\n"
        "const vec2  ONE = vec2(1.0, 0.0);\n"
        "\n"
        "struct Borders\n"
        "{\n"
        "   float left;\n"
        "   float right;\n"
        "   float bottom;\n"
        "   float top;\n"
        "};\n"
        "\n"
        "uniform Borders borders;\n"
        "uniform ivec2   winsizes;\n"
        "uniform int     itrn_max;\n"
        "uniform float   limit;\n"
        "uniform int     drawing_mode;\n"
        "uniform bool    coloring;\n"
        "uniform vec2   julia_point;\n"
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
        "float arg(vec2 a)\n"
        "{\n"
        "    return atan(a.y, a.x);\n"
        "}\n"
        "\n"
        "vec2 cabs(vec2 a)\n"
        "{\n"
        "    return vec2(sqrt(a.x * a.x + a.y * a.y), 0.0);\n"
        "}\n"
        "\n"
        "vec2 carg(vec2 a)\n"
        "{\n"
        "    return vec2(arg(a), 0.0);\n"
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
        "        return vec2(log(a.x * a.x + a.y * a.y) / 2, PI);\n"
        "    else\n"
        "        return vec2(log(a.x * a.x + a.y * a.y) / 2, arg(a));\n"
        "}\n"
        "\n"
        "vec2 clg(vec2 a)\n"
        "{\n"
        "    vec2 ln = cln(a);\n"
        "    return vec2(ln.x / log(10.0), ln.y / log(10.0));\n"
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
        "    return cpow(a, vec2(0.5, 0.0));\n"
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
        "    vec2 a_2 = vec2(a.x * 2.0, a.y * 2.0);\n"
        "    float bottom = cos(a_2.x) + cosh(a_2.y);\n"
        "    return vec2(sin(a_2.x) / bottom, sinh(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 ccot(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2.0, a.y * 2.0);\n"
        "    float bottom = cos(a_2.x) - cosh(a_2.y);\n"
        "    return vec2(-sin(a_2.x) / bottom, sinh(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 carcsin(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.0, -1.0), cln(cadd(cmul(I, a), csqrt(csub(ONE, cmul(a, a))))));\n"
        "}\n"
        "\n"
        "vec2 carccos(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.0, -1.0), cln(cadd(a, csqrt(csub(cmul(a, a), ONE)))));\n"
        "}\n"
        "\n"
        "vec2 carctan(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.0, 0.5), csub(cln(cadd(I, a)), cln(csub(I, a))));\n"
        "}\n"
        "\n"
        "vec2 carccot(vec2 a)\n"
        "{\n"
        "    return csub(vec2(PI / 2, 0.0), cmul(vec2(0.0, 0.5), csub(cln(cadd(I, a)), cln(csub(I, a)))));\n"
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
        "    vec2 a_2 = vec2(a.x * 2.0, a.y * 2.0);\n"
        "    float bottom = cosh(a_2.x) + cos(a_2.y);\n"
        "    return vec2(sinh(a_2.x) / bottom, sin(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 ccoth(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2.0, a.y * 2.0);\n"
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
        "    return cmul(vec2(0.5, 0.0), csub(cln(cadd(ONE, a)), cln(csub(ONE, a))));\n"
        "}\n"
        "\n"
        "vec2 carccoth(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.5, 0.0), csub(cln(cadd(ONE, a)), cln(csub(ONE, a))));\n"
        "}\n"
        "\n"
        "vec3 getColor(int itrn, vec3 sumz)\n"
        "{\n"
        "    if (itrn < itrn_max)\n"
        "    {\n"
        "        itrn = itrn * 4 % 1530;\n"
        "        if (itrn < 256)  return vec3(255, itrn, 0)        / 255 * (1.0 - float(coloring) * 0.85);\n"
        "        if (itrn < 511)  return vec3(510 - itrn, 255, 0)  / 255 * (1.0 - float(coloring) * 0.85);\n"
        "        if (itrn < 766)  return vec3(0, 255, itrn - 510)  / 255 * (1.0 - float(coloring) * 0.85);\n"
        "        if (itrn < 1021) return vec3(0, 1020 - itrn, 255) / 255 * (1.0 - float(coloring) * 0.85);\n"
        "        if (itrn < 1276) return vec3(itrn - 1020, 0, 255) / 255 * (1.0 - float(coloring) * 0.85);\n"
        "        if (itrn < 1530) return vec3(255, 0, 1529 - itrn) / 255 * (1.0 - float(coloring) * 0.85);\n"
        "    }\n"
        "    if (coloring) return sin(abs(abs(sumz) / itrn_max * 5.0)) * 0.45 + 0.5;\n"
        "    return vec3(0.0, 0.0, 0.0);\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    float re0 = borders.left + (borders.right - borders.left) * gl_FragCoord.x / winsizes.x;\n"
        "    float im0 = borders.top - (borders.top - borders.bottom) * gl_FragCoord.y / winsizes.y;\n"
        "\n"
            + std::string(str_initialization) + 
        "\n"
        "    vec3 sumz = vec3(0.0, 0.0, 0.0);\n"
        "    int itrn  = 0;\n"
        "    for (itrn = 0; itrn < itrn_max; ++itrn)\n"
        "    {\n"
            + str_calculation + 
        "        \n"
            + str_checking +
        "    }\n"
        "\n"
        "    vec3 col = getColor(itrn, sumz);\n"
        "    gl_FragColor = vec4(col, 1.0);\n"
        "}";

    shader_.loadFromMemory(str_shader, sf::Shader::Fragment);

    return 0;
}

void Puzabrot::Engine::initCalculator(Calculator& calc, point_t z, point_t c) const
{
    switch (options.input_mode)
    {
    case Z_INPUT:
    {
        calc.variables.push_back({ { c.x, c.y }, "c" });
        calc.variables.push_back({ { z.x, z.y }, "z" });
        break;
    }
    case XY_INPUT:
    {
        calc.variables.push_back({ { c.x, 0.0 }, "cx" });
        calc.variables.push_back({ { c.y, 0.0 }, "cy" });

        calc.variables.push_back({ { z.x, 0.0 }, "x" });
        calc.variables.push_back({ { z.y, 0.0 }, "y" });
        break;
    }
    }
}

void Puzabrot::Engine::Mapping(Calculator& calc, ExprTrees& expr_trees, point_t& mapped_point) const
{
    switch (options.input_mode)
    {
    case Z_INPUT:
    {
        calc.Calculate(expr_trees.z);
        calc.variables[calc.variables.size() - 1] = { expr_trees.z.value().number, "z" };

        mapped_point = point_t(real(expr_trees.z.value().number), imag(expr_trees.z.value().number));
        break;
    }
    case XY_INPUT:
    {
        calc.Calculate(expr_trees.x);
        calc.Calculate(expr_trees.y);

        calc.variables[calc.variables.size() - 2] = { { real(expr_trees.x.value().number), 0.0 }, "x" };
        calc.variables[calc.variables.size() - 1] = { { real(expr_trees.y.value().number), 0.0 }, "y" };

        mapped_point = point_t(real(expr_trees.x.value().number), real(expr_trees.y.value().number));
        break;
    }
    }
}

const char* Puzabrot::Engine::writeInitialization() const
{
    switch (options.input_mode)
    {
    case Z_INPUT:
    {
        return
            "vec2 z = vec2(re0, im0);\n"
            "vec2 pz = z;\n"
            "vec2 c = vec2(0.0, 0.0);\n"
            "if (drawing_mode == 0)\n"
            "    c = vec2(re0, im0);\n"
            "else if (drawing_mode == 1)\n"
            "    c = vec2(julia_point.x, julia_point.y);";
    }
    case XY_INPUT:
    {
        return
            "float x = re0;\n"
            "float y = im0;\n"
            "vec2 pz = vec2(x, y);\n"
            "float cx = 0.0;\n"
            "float cy = 0.0;\n"
            "if (drawing_mode == 0)\n"
            "{\n"
            "    cx = re0;\n"
            "    cy = im0;\n"
            "}\n"
            "else if (drawing_mode == 1)\n"
            "{\n"
            "    cx = julia_point.x;\n"
            "    cy = julia_point.y;\n"
            "}";
    }
    }

    return nullptr;
}

std::string Puzabrot::Engine::writeCalculation() const
{
    switch (options.input_mode)
    {
    case Z_INPUT:
    {
        std::string str_calculation =
            "vec2 ppz = pz;\n"
            "pz = z;\n";

        str_calculation += "z = ";
        int err = Tree2GLSL(expr_trees.z, &str_calculation);
        if (err)
        {
            return std::string();
        }

        str_calculation += ";";

        return str_calculation;
    }
    case XY_INPUT:
    {
        std::string str_calculation =
            "vec2 ppz = pz;\n"
            "pz = vec2(x, y);\n";

        str_calculation += "vec2 x1 = ";
        int err = Tree2GLSL(expr_trees.x, &str_calculation);
        if (err)
        {
            return std::string();
        }

        str_calculation += ";\nvec2 y1 = ";
        err = Tree2GLSL(expr_trees.y, &str_calculation);
        if (err)
        {
            return std::string();
        }

        str_calculation += ";\nx = x1.x;\ny = y1.x;";

        return str_calculation;
    }
    }

    return std::string();
}

std::string Puzabrot::Engine::writeChecking() const
{
    std::string str_checking;

    switch (options.input_mode)
    {
    case Z_INPUT:
        str_checking = "if (cabs(z).x > limit) break;\n";
        break;
    case XY_INPUT:
        str_checking = "if (cabs(vec2(x, y)).x > limit) break;\nvec2 z = vec2(x, y);\n";
        break;
    }

    str_checking +=
        "sumz.x += dot(z - pz, pz - ppz);\n"
        "sumz.y += dot(z - pz,  z - pz);\n"
        "sumz.z += dot(z - ppz, z - ppz);";

    return str_checking;
}

int Puzabrot::Engine::Tree2GLSL(const Tree<CalcData>& node, std::string* str) const
{
    switch (node.value().node_type)
    {
    case CalcData::FUNCTION:
    {
        *str += "c" + node.value().word + "(";

        int err = Tree2GLSL(node[0], str);
        COND_RETURN(err, err);

        *str += ")";
        break;
    }
    case CalcData::OPERATOR:
    {
        switch (node.value().op_code)
        {
        case Operation::ADD:
            *str += "cadd(";
            break;
        case Operation::SUB:
            *str += "csub(";
            break;
        case Operation::MUL:
            *str += "cmul(";
            break;
        case Operation::DIV:
            *str += "cdiv(";
            break;
        case Operation::POW:
            *str += "cpow(";
            break;
        default:
            break;
        }

        if (node.branches_num() < 2)
        {
            *str += "vec2(0.0, 0.0), ";
        }
        else
        {
            int err = Tree2GLSL(node[1], str);
            COND_RETURN(err, err);

            *str += ", ";
        }

        int err = Tree2GLSL(node[0], str);
        COND_RETURN(err, err);

        *str += ")";
        break;
    }
    case CalcData::VARIABLE:
    {
        switch (options.input_mode)
        {
        case Z_INPUT:
        {
            COND_RETURN((node.value().word != "z") && (node.value().word != "c") && (node.value().word != "pi") &&
                (node.value().word != "e") && (node.value().word != "i"), -1);

            break;
        }
        case XY_INPUT:
        {
            COND_RETURN((node.value().word != "x") && (node.value().word != "y") && (node.value().word != "cx") &&
                (node.value().word != "cy") && (node.value().word != "pi") && (node.value().word != "e") &&
                (node.value().word != "i"), -1);

            break;
        }
        }

        if (node.value().word == "i")
        {
            *str += "I";
        }
        else
        {
            switch (options.input_mode)
            {
            case Z_INPUT:
                *str += node.value().word;
                break;
            case XY_INPUT:
                *str += "vec2(" + node.value().word + ", 0.0)";
                break;
            }
        }
        break;
    }
    case CalcData::NUMBER:
    {
        switch (options.input_mode)
        {
        case Z_INPUT:
            *str += "vec2(" + std::to_string(real(node.value().number)) + ", " + std::to_string(imag(node.value().number)) + ")";
            break;
        case XY_INPUT:
            *str += "vec2(" + std::to_string(real(node.value().number)) + ", 0.0)";
            break;
        }
        break;
    }
    default:
        break;
    }

    return 0;
}

Puzabrot::Synth::Synth(const Engine* engine) : audio_reset(true), audio_pause(false), sustain(true), engine_(engine)
{
    initialize(2, SYNTH_SAMPLE_RATE);
    setLoop(true);
}

void Puzabrot::Synth::setPoint(point_t point)
{
    new_point_ = point;

    audio_reset = true;
    audio_pause = false;
}

void Puzabrot::Synth::setExpressions(const Engine::ExprTrees& expr_trees)
{
    expr_trees_ = expr_trees;
}

bool Puzabrot::Synth::onGetData(Chunk& data)
{
    data.samples = m_samples_;
    data.sampleCount = SYNTH_AUDIO_BUFF_SIZE;
    memset(m_samples_, 0, SYNTH_AUDIO_BUFF_SIZE * sizeof(int16_t));

    if (audio_reset)
    {
        m_audio_time_ = 0;

        point_ = new_point_;
        prev_point_ = new_point_;
        mean_ = new_point_;

        mag_ = 0.0;
        pmag_ = 0.0;
        phase_ = 0.0;

        volume_ = 8000.0;

        audio_reset = false;
    }

    COND_RETURN(audio_pause, true);

    c_point_ = (engine_->options.draw_mode == MAIN) ? new_point_ : engine_->params.julia_point;

    Calculator calc;
    const int steps = SYNTH_SAMPLE_RATE / SYNTH_MAX_FREQ;

    for (size_t i = 0; i < SYNTH_AUDIO_BUFF_SIZE; i += 2)
    {
        const int j = m_audio_time_ % steps;
        if (j == 0)
        {
            prev_point_ = point_;

            calc.clear();
            engine_->initCalculator(calc, point_, c_point_);
            engine_->Mapping(calc, expr_trees_, point_);

            if (sqrt(pow(point_.x, 2.0) + pow(point_.y, 2.0)) > engine_->params.limit)
            {
                audio_pause = true;
                return true;
            }

            d_  = point_ - mean_;
            dp_ = prev_point_ - mean_;

            pmag_ = sqrt(1e-12 + dp_.x * dp_.x + dp_.y * dp_.y);
            mag_  = sqrt(1e-12 + d_.x * d_.x + d_.y * d_.y);

            mean_ = mean_ * 0.99 + point_ * 0.01;

            double m = d_.x * d_.x + d_.y * d_.y;
            if (m > 2.0)
            {
                d_ *= 2.0 / m;
            }

            m = dp_.x * dp_.x + dp_.y * dp_.y;
            if (m > 2.0)
            {
                dp_ *= 2.0 / m;
            }

            if (!sustain)
            {
                volume_ *= 0.9992;
            }
        }

        double t = 0.5 - 0.5 * cos(double(j) / double(steps) * real(PI));

        double wy = t * point_.y + (1.0 - t) * prev_point_.y;
        double wmag = t * mag_ + (1.0 - t) * pmag_;

        phase_ += wy / real(PI) / steps;
        double s = std::sin(phase_) * wmag;

        m_samples_[i] = static_cast<int16_t>(std::min(std::max(s * volume_, -32000.0), 32000.0));
        m_samples_[i + 1] = static_cast<int16_t>(std::min(std::max(s * volume_, -32000.0), 32000.0));

        m_audio_time_ += 1;
    }

    return !audio_reset;
}