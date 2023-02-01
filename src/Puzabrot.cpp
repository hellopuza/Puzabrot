#include "Puzabrot.h"
#include "Utils.h"

#include <cstring>
#include <format>

#define COND_RETURN(cond, ret) \
    if (cond)                  \
    {                          \
        return (ret);          \
    } //

constexpr double LIMIT = 100.0;
constexpr double FREQUENCY = 5.0;
constexpr double UPPER_BORDER = 1.3;
constexpr double ZOOMING_RATIO = 0.2;
constexpr size_t MAX_ITERATION = 500;
constexpr double GRID_FONT_SIZE = 14.0;
constexpr double UI_FONT_SIZE = 16.0;
constexpr size_t SCREENSHOT_WIDTH = 7680;

static const char* TITLE_STRING = "Puzabrot";
static const char* FONT_LOCATION = "assets/consola.ttf";
static const vec2i WINDOW_SIZE = { 640, 480 };
static const vec2d INPUT_BUTTON_POS = { 10.0, 10.0 };
static const vec2d INPUT_BOX_POS = { 10.0, 35.0 };
static const vec2d COLOR_BUTTON_POS = { 10.0, 125.0 };
static const vec2d GRID_BUTTON_POS = { 10.0, 150.0 };
static const vec2d SOUND_BUTTON_POS = { 10.0, 175.0 };
static const vec2d ITERATION_BUTTON_POS = { 10.0, 200.0 };
static const vec2d PARAMETER_BUTTON_POS = { 10.0, 225.0 };

#define INPUT_BUTTON static_cast<SwitchButton*>(ui_.getVidget("input_button"))
#define INPUT_X static_cast<InputBox*>(ui_.getVidget("input_x"))
#define INPUT_Y static_cast<InputBox*>(ui_.getVidget("input_y"))
#define INPUT_Z static_cast<InputBox*>(ui_.getVidget("input_z"))
#define COLOR_BUTTON static_cast<SwitchButton*>(ui_.getVidget("color_button"))
#define GRID_BUTTON static_cast<Button*>(ui_.getVidget("grid_button"))
#define SOUND_BUTTON static_cast<Button*>(ui_.getVidget("sound_button"))
#define ITERATION_BUTTON static_cast<Button*>(ui_.getVidget("iteration_button"))
#define PARAMETER_BUTTON static_cast<Button*>(ui_.getVidget("parameter_button"))

#define SET_INPUT_Y_POS INPUT_Y->setPosition(INPUT_X->getPosition() + vec2d(0.0, INPUT_X->getSize().y + 3.0))

#define SET_INPUT_VISIBIITY if (options_.input_mode == XY_INPUT) \
                            { INPUT_X->show(); INPUT_Y->show(); INPUT_Z->hide(); } \
                            else \
                            { INPUT_X->hide(); INPUT_Y->hide(); INPUT_Z->show(); }

#define TEXT_ENTERING (INPUT_X->hasFocus() || INPUT_Y->hasFocus() || INPUT_Z->hasFocus())

Puzabrot::Puzabrot() :
    ShaderApplication(WINDOW_SIZE, FONT_LOCATION, GRID_FONT_SIZE, TITLE_STRING),
    synth_(std::make_unique<Synth>(this))
{
    params_.limit = LIMIT;
    params_.frequency = FREQUENCY;
    params_.itrn_max = MAX_ITERATION;

    setZoomingRatio(ZOOMING_RATIO);
    setBorders(-UPPER_BORDER, UPPER_BORDER, 0.5);

    ui_.addVidget("input_button", new SwitchButton(getFont(), UI_FONT_SIZE, INPUT_BUTTON_POS));
    INPUT_BUTTON->addText("Z");
    INPUT_BUTTON->addText("XY");

    ui_.addVidget("input_x", new InputBox(getFont(), UI_FONT_SIZE, INPUT_BOX_POS));
    ui_.addVidget("input_y", new InputBox(getFont(), UI_FONT_SIZE, INPUT_BOX_POS));
    ui_.addVidget("input_z", new InputBox(getFont(), UI_FONT_SIZE, INPUT_BOX_POS));

    INPUT_X->setLabel(sf::String("x:"));
    INPUT_Y->setLabel(sf::String("y:"));
    INPUT_Z->setLabel(sf::String("z:"));

    INPUT_X->setInput(sf::String("x*x-y*y+cx"));
    INPUT_Y->setInput(sf::String("2*x*y+cy"));
    INPUT_Z->setInput(sf::String("z^2+c"));

    SET_INPUT_Y_POS;
    SET_INPUT_VISIBIITY;

    ui_.addVidget("color_button", new SwitchButton(getFont(), UI_FONT_SIZE, COLOR_BUTTON_POS));
    COLOR_BUTTON->addText("DEFAULT");
    COLOR_BUTTON->addText("TRACER");
    COLOR_BUTTON->addText("DOMAIN");
    COLOR_BUTTON->addText("KALI");

    ui_.addVidget("grid_button", new Button(getFont(), UI_FONT_SIZE, GRID_BUTTON_POS));
    GRID_BUTTON->setText("GRID");

    ui_.addVidget("sound_button", new Button(getFont(), UI_FONT_SIZE, SOUND_BUTTON_POS));
    SOUND_BUTTON->setText("SOUND");

    ui_.addVidget("iteration_button", new Button(getFont(), UI_FONT_SIZE, ITERATION_BUTTON_POS));
    ui_.addVidget("parameter_button", new Button(getFont(), UI_FONT_SIZE, PARAMETER_BUTTON_POS));
}

void Puzabrot::prerun()
{
    setVerticalSyncEnabled(true);

    makeShader();
    render();
}

void Puzabrot::handleAppEvent(const sf::Event& event)
{
    // Handle UI events
    if (ui_.handleEvent(event))
    {
        INPUT_Y->is_visible = INPUT_X->is_visible;

        // Enter expression from input box
        if (INPUT_X->TextEntered() || INPUT_Y->TextEntered() || INPUT_Z->TextEntered())
        {
            int err = makeShader();

            if (!err)
            {
                render();
                options_.draw_mode = MAIN;
            }

            switch (options_.input_mode)
            {
            case Z_INPUT:
            {
                if (err)
                {
                    INPUT_Z->setOutput(sf::String(calc_errstr[err + 1]));
                }
                else
                {
                    INPUT_Z->setOutput(sf::String());
                }
                break;
            }
            case XY_INPUT:
            {
                if (INPUT_X->TextEntered())
                {
                    if (err)
                    {
                        INPUT_X->setOutput(sf::String(calc_errstr[err + 1]));
                        SET_INPUT_Y_POS;
                    }
                    else
                    {
                        INPUT_X->setOutput(sf::String());
                        SET_INPUT_Y_POS;
                    }
                }
                else if (err)
                {
                    INPUT_Y->setOutput(sf::String(calc_errstr[err + 1]));
                }
                else
                {
                    INPUT_Y->setOutput(sf::String());
                }
                break;
            }
            }
        }

        // Toggle input mode
        else if (options_.input_mode != INPUT_BUTTON->value())
        {
            options_.input_mode = INPUT_BUTTON->value();
            SET_INPUT_VISIBIITY;
            makeShader();
            render();
        }

        // Toggle color mode
        if (options_.coloring_mode != COLOR_BUTTON->value())
        {
            options_.coloring_mode = COLOR_BUTTON->value();
            makeShader();
            render();
        }

        // Toggle grid showing
        options_.showing_grid = GRID_BUTTON->pressed();

        // Toggle sound mode
        options_.sound_mode = SOUND_BUTTON->pressed();
        if (options_.sound_mode)
        {
            synth_->audio_pause = false;
            synth_->play();
        }
        else
        {
            synth_->pause();
        }
    }

    // Toggle UI showing
    else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::LControl) && !TEXT_ENTERING)
    {
        ui_.is_visible = !ui_.is_visible;
    }

    // Reset set drawing
    else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R) && !TEXT_ENTERING)
    {
        options_.draw_mode = MAIN;
        params_.limit = LIMIT;
        params_.frequency = FREQUENCY;
        params_.itrn_max = MAX_ITERATION;
        setBorders(-UPPER_BORDER, UPPER_BORDER, 0.5);
        makeShader();
        render();
    }

    // Take a screenshot
    else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space) && !options_.was_screenshot && !TEXT_ENTERING)
    {
        savePicture();
        options_.was_screenshot = true;
    }

    // Julia set drawing
    else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tab) && !TEXT_ENTERING)
    {
        if (options_.draw_mode != JULIA)
        {
            options_.julia_dragging = true;
            options_.draw_mode = JULIA;
            makeShader();
        }
        else
        {
            if (options_.julia_dragging)
            {
                options_.julia_dragging = false;
            }
            else
            {
                options_.draw_mode = MAIN;
                makeShader();
                render();
            }
        }
    }

    // Change max iterations
    else if ((event.type == sf::Event::MouseWheelMoved) && (ITERATION_BUTTON->pressed()))
    {
        size_t new_itrn = static_cast<size_t>(static_cast<double>(params_.itrn_max) * pow(1.03, static_cast<float>(event.mouseWheel.delta)));
        params_.itrn_max = (new_itrn == params_.itrn_max) ? params_.itrn_max + 1 : new_itrn;
        params_.itrn_max = (params_.itrn_max < 1) ? 1 : params_.itrn_max;
        render();
    }

    // Change parameter
    else if ((event.type == sf::Event::MouseWheelMoved) && (PARAMETER_BUTTON->pressed()))
    {
        switch (options_.coloring_mode)
        {
        case DEFAULT:
        case TRACER:
        {
            params_.limit *= pow(2.0, static_cast<float>(event.mouseWheel.delta));
            break;
        }
        case COMPLEX_DOMAIN:
        {
            break;
        }
        case KALI:
        {
            params_.frequency *= pow(1.005, static_cast<float>(event.mouseWheel.delta));
            break;
        }
        }
        render();
    }

    // Point tracing and sounding
    else if ((event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Right))
    {
        options_.showing_trace = true;
        options_.right_pressed = true;

        if (options_.sound_mode)
        {
            synth_->audio_pause = false;
            synth_->setPoint(Screen2Base(vec(sf::Mouse::getPosition(*this))));
            synth_->play();
        }
        else
        {
            synth_->pause();
        }
    }
    else if ((event.type == sf::Event::MouseButtonReleased) && (event.mouseButton.button == sf::Mouse::Right))
    {
        options_.right_pressed = false;
    }
    else if ((event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Middle))
    {
        options_.showing_trace = false;
        synth_->audio_pause = true;
        synth_->pause();
    }

    else if (handleEvent(event))
    {
        setRenderImageSize(vec(getSize()));
        render();
    }
}

void Puzabrot::activity()
{
    clear();

    if (options_.julia_dragging)
    {
        params_.julia_point = Screen2Base(vec(sf::Mouse::getPosition(*this)));
        render();
    }
    draw(getRenderOutput());

    if (options_.showing_grid)
    {
        drawGrid();
    }

    if (options_.right_pressed)
    {
        params_.c_point = Screen2Base(vec(sf::Mouse::getPosition(*this)));
        params_.orbit = params_.c_point;

        synth_->setPoint(params_.c_point);
    }

    if (options_.showing_trace)
    {
        params_.orbit = PointTrace(params_.orbit, params_.c_point);
    }

    ITERATION_BUTTON->setText(std::format("ITERATION {}", params_.itrn_max));

    switch (options_.coloring_mode)
    {
    case DEFAULT:
    case TRACER:
    {
        PARAMETER_BUTTON->show();
        PARAMETER_BUTTON->setText(std::format("LIMIT {:.3g}", params_.limit));
        break;
    }
    case COMPLEX_DOMAIN:
    {
        PARAMETER_BUTTON->hide();
        break;
    }
    case KALI:
    {
        PARAMETER_BUTTON->show();
        PARAMETER_BUTTON->setText(std::format("FREQUENCY {:.3g}", params_.frequency));
        break;
    }
    }

    draw(ui_);

    display();
}

void Puzabrot::postrun()
{
    synth_->stop();
}

vec2d Puzabrot::PointTrace(const vec2d& point, const vec2d& c_point)
{
    Calculator calc;
    initCalculator(calc, point, (options_.draw_mode == MAIN) ? c_point : params_.julia_point);

    vec2d point1 = point;
    vec2d point2;

    for (size_t i = 0; i < params_.itrn_max; ++i)
    {
        Mapping(calc, expr_trees_, point2);

        if (sqrt(pow(point2.x, 2.0) + pow(point2.y, 2.0)) > params_.limit)
        {
            break;
        }

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(vec(Base2Screen(point1))), sf::Color::White),
            sf::Vertex(sf::Vector2f(vec(Base2Screen(point2))), sf::Color::Black)
        };

        point1 = point2;

        draw(line, 2, sf::Lines);
    }

    return point1;
}

void Puzabrot::initCalculator(Calculator& calc, const vec2d& z, const vec2d& c) const
{
    switch (options_.input_mode)
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

void Puzabrot::Mapping(Calculator& calc, ExprTrees& expr_trees, vec2d& mapped_point) const
{
    switch (options_.input_mode)
    {
    case Z_INPUT:
    {
        calc.Calculate(expr_trees.z);
        calc.variables[calc.variables.size() - 1] = { expr_trees.z.value().number, "z" };

        mapped_point = vec2d(real(expr_trees.z.value().number), imag(expr_trees.z.value().number));
        break;
    }
    case XY_INPUT:
    {
        calc.Calculate(expr_trees.x);
        calc.Calculate(expr_trees.y);

        calc.variables[calc.variables.size() - 2] = { { real(expr_trees.x.value().number), 0.0 }, "x" };
        calc.variables[calc.variables.size() - 1] = { { real(expr_trees.y.value().number), 0.0 }, "y" };

        mapped_point = vec2d(real(expr_trees.x.value().number), real(expr_trees.y.value().number));
        break;
    }
    }
}

void Puzabrot::savePicture()
{
    static int  shot_num = 0;
    std::string filename = "screenshot(" + std::to_string(shot_num++) + ")" + ".png";

    draw(getRenderOutput());

    sf::RectangleShape rectangle;
    rectangle.setPosition(0.0F, 0.0F);
    rectangle.setSize(sf::Vector2f(getSize()));
    rectangle.setFillColor(sf::Color(10, 10, 10, 150));

    draw(rectangle);
    display();

    sf::Vector2u screenshot_sizes(SCREENSHOT_WIDTH, static_cast<unsigned int>(static_cast<float>(SCREENSHOT_WIDTH) /
        static_cast<float>(getSize().x) * static_cast<float>(getSize().y)));

    sf::Vector2u old_sizes = getSize();
    setRenderImageSize(vec(screenshot_sizes));
    render();
    getRenderOutput().getTexture()->copyToImage().saveToFile(filename);

    setRenderImageSize(vec(old_sizes));
    render();
    draw(getRenderOutput());
    display();
}

int Puzabrot::makeShader()
{
    switch (options_.input_mode)
    {
    case Z_INPUT:
    {
        Expression expr_z(INPUT_Z->getInput());

        int err = expr_z.getTree(expr_trees_.z);
        COND_RETURN(err, err);
        break;
    }
    case XY_INPUT:
    {
        Expression expr_x(INPUT_X->getInput());
        Expression expr_y(INPUT_Y->getInput());

        int err = expr_x.getTree(expr_trees_.x);
        COND_RETURN(err, err);

        err = expr_y.getTree(expr_trees_.y);
        COND_RETURN(err, err);
    }
    }
    synth_->setExpressions(expr_trees_);

    COND_RETURN(writeShader(), CALC_WRONG_VARIABLE);

    return 0;
}

void Puzabrot::render()
{
    shader_.setUniform("borders.left", static_cast<float>(getBorders().left));
    shader_.setUniform("borders.right", static_cast<float>(getBorders().right));
    shader_.setUniform("borders.bottom", static_cast<float>(getBorders().bottom));
    shader_.setUniform("borders.top", static_cast<float>(getBorders().top));

    shader_.setUniform("winsizes", sf::Glsl::Ivec2(sf::Vector2i(render_texture_.getSize())));

    shader_.setUniform("itrn_max", static_cast<int>(params_.itrn_max));
    shader_.setUniform("limit", static_cast<float>(params_.limit));
    shader_.setUniform("frequency", static_cast<float>(1.0 / (1.0 + std::exp(-params_.frequency))));
    shader_.setUniform("julia_point", sf::Glsl::Vec2(sf::Vector2f(vec(params_.julia_point))));

    render_texture_.draw(sprite_, &shader_);
}

int Puzabrot::writeShader()
{
    std::string str_functions = writeFunctions();
    std::string str_color_function = writeColorFunction();
    std::string str_main = writeMain();
    COND_RETURN(str_main.empty(), -1);

    std::string str_shader =
        "#version 130\n"
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
        "uniform float   frequency;\n"
        "uniform vec2    julia_point;\n"
        "\n"
            + str_functions +
        "\n"
            + str_color_function +
        "\n"
            + str_main;

    shader_.loadFromMemory(str_shader, sf::Shader::Fragment);

    return 0;
}

std::string Puzabrot::writeFunctions() const
{
    return
        "const float NIL = 1e-9;\n"
        "const float PI  = atan(1.0) * 4.0;\n"
        "const float E   = exp(1.0);\n"
        "const vec2  I   = vec2(0.0, 1.0);\n"
        "const vec2  ONE = vec2(1.0, 0.0);\n"
        "\n"
        "vec2 conj(vec2 a)\n"
        "{\n"
        "    return vec2(a.x, -a.y);\n"
        "}\n"
        "\n"
        "float norm(vec2 a)\n"
        "{\n"
        "    return dot(a, a);\n"
        "}\n"
        "\n"
        "float arg(vec2 a)\n"
        "{\n"
        "    return atan(a.y, a.x);\n"
        "}\n"
        "\n"
        "vec2 cabs(vec2 a)\n"
        "{\n"
        "    return vec2(length(a), 0.0);\n"
        "}\n"
        "\n"
        "vec2 carg(vec2 a)\n"
        "{\n"
        "    return vec2(arg(a), 0.0);\n"
        "}\n"
        "\n"
        "vec2 cadd(vec2 a, vec2 b)\n"
        "{\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "vec2 csub(vec2 a, vec2 b)\n"
        "{\n"
        "    return a - b;\n"
        "}\n"
        "\n"
        "vec2 cmul(vec2 a, vec2 b)\n"
        "{\n"
        "    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);\n"
        "}\n"
        "\n"
        "vec2 cdiv(vec2 a, vec2 b)\n"
        "{\n"
        "    return cmul(a, conj(b)) / norm(b);\n"
        "}\n"
        "\n"
        "vec2 cln(vec2 a)\n"
        "{\n"
        "    return vec2(log(a.x * a.x + a.y * a.y) * 0.5, arg(a));\n"
        "}\n"
        "\n"
        "vec2 clg(vec2 a)\n"
        "{\n"
        "    return cln(a) / log(10.0);\n"
        "}\n"
        "\n"
        "vec2 cexp(vec2 a)\n"
        "{\n"
        "    return vec2(cos(a.y), sin(a.y)) * exp(a.x);\n"
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
        "}\n";
}

std::string Puzabrot::writeColorFunction() const
{
    std::string str;
    switch (options_.coloring_mode)
    {
    case DEFAULT:
    case TRACER:
    {
        str +=
            "const float K = PI / 3.0;\n"
            "\n"
            "float pf(float x)\n"
            "{\n"
            "    return min(max(acos(cos(x * K)) / K - 1.0F, 0.0), 1.0);\n"
            "}\n"
            "\n";

        str += (options_.coloring_mode == DEFAULT) ?
            "vec3 getColor(int itrn)\n" :
            "vec3 getColor(int itrn, vec3 sum)\n";

        str +=
            "{\n"
            "if (itrn < itrn_max)\n"
            "{\n"
            "    float x = float(itrn) * 4.0 / 255.0;\n"
            "    return vec3(pf(x - 3.0F), pf(x - 5.0F), pf(x - 7.0F));\n"
            "}\n";

        str += (options_.coloring_mode == DEFAULT) ?
            "return vec3(0.0, 0.0, 0.0);\n"
            "}\n" :
            "return sin(abs(sum / itrn_max * 5.0)) * 0.45 + 0.5;\n"
            "}\n";
        break;
    }
    case COMPLEX_DOMAIN:
    {
        str +=
            "vec3 getColor(vec2 z)\n"
            "{\n"
            "float magnitude = length(z);\n"
            "float phase = atan(z.y, z.x);\n"
            "float color_lightness = (0.5 + atan(0.5 * log(magnitude)) / PI) * 2.0;\n"
            "float color_saturation = 1.0 - abs(color_lightness - 1.0);\n"
            "float color_value = (color_lightness + color_saturation) / 2.0;\n"
            "color_saturation /= color_value;\n"
            "vec3 c = vec3(phase / (2.0 * PI), color_saturation, color_value);\n"
            "vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
            "vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
            "return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
            "}\n";
        break;
    }
    case KALI:
    {
        str +=
            "vec3 getColor(vec3 sum)\n"
            "{\n"
            "return vec3(cos(sum.x), cos(sum.y), cos(sum.z)) * 0.5 + 0.5;\n"
            "}\n";
        break;
    }
    }
    return str;
}

std::string Puzabrot::writeInitialization() const
{
    std::string str;
    switch (options_.input_mode)
    {
    case Z_INPUT:
    {
        str +=
            "vec2 z = vec2(re0, im0);\n";

        str += (options_.draw_mode == MAIN) ?
            "vec2 c = vec2(re0, im0);\n" :
            "vec2 c = vec2(julia_point.x, julia_point.y);";

        str += (options_.coloring_mode == TRACER) ?
            "vec2 pz = z;\n" :
            "";
        break;
    }
    case XY_INPUT:
    {
        str +=
            "float x = re0;\n"
            "float y = im0;\n";

        str += (options_.draw_mode == MAIN) ?
            "float cx = re0;\n"
            "float cy = im0;\n" :
            "float cx = julia_point.x;\n"
            "float cy = julia_point.y;\n";

        str += (options_.coloring_mode == TRACER) ?
            "vec2 pz = vec2(x, y);\n" :
            "";

        break;
    }
    }

    str +=
        "vec3 sum = vec3(0.0, 0.0, 0.0);\n";

    str += (options_.coloring_mode == KALI) ?
        "float weight = 1.0;\n" :
        "";

    return str;
}

std::string Puzabrot::writeCalculation() const
{
    std::string str;
    switch (options_.input_mode)
    {
    case Z_INPUT:
    {
        str += (options_.coloring_mode == TRACER) ?
            "vec2 ppz = pz;\n"
            "pz = z;\n" :
            "";

        str +=
            "z = ";

        COND_RETURN(Tree2GLSL(expr_trees_.z, &str), std::string());

        str += ";\n";

        break;
    }
    case XY_INPUT:
    {
        str += (options_.coloring_mode == TRACER) ?
            "vec2 ppz = pz;\n"
            "pz = vec2(x, y);\n" :
            "";

        str +=
            "vec2 x1 = ";

        COND_RETURN(Tree2GLSL(expr_trees_.x, &str), std::string());

        str += ";\n"
            "vec2 y1 = ";

        COND_RETURN(Tree2GLSL(expr_trees_.y, &str), std::string());

        str += ";\n"
            "x = x1.x;\n"
            "y = y1.x;\n";
        break;
    }
    }
    return str;
}

std::string Puzabrot::writeChecking() const
{
    std::string str;
    switch (options_.coloring_mode)
    {
    case DEFAULT:
    case TRACER:
    {
        str += (options_.input_mode == XY_INPUT) ?
            "vec2 z = vec2(x, y);\n" :
            "";

        str +=
            "if (cabs(z).x > limit) break;\n";

        str += (options_.coloring_mode == TRACER) ?
            "sum.x += dot(z - pz,  pz - ppz);\n"
            "sum.y += dot(z - pz,  z - pz);\n"
            "sum.z += dot(z - ppz, z - ppz);\n" :
            "";
        break;
    }
    case COMPLEX_DOMAIN:
    {
        break;
    }
    case KALI:
    {
        str += (options_.input_mode == Z_INPUT) ?
            "float r = dot(z, z);\n"
            "if (r > 1.0) z /= r;\n"
            "weight *= frequency;\n"
            "sum += vec3(z.x * z.x, z.y * z.y, abs(z.x * z.y)) * weight;\n" :
            "float r = x * x + y *y;\n"
            "if (r > 1.0) { x /= r; y /= r; }\n"
            "weight *= frequency;\n"
            "sum += vec3(x * x, y * y, abs(x * y)) * weight;\n";
        break;
    }
    }

    return str;
}

std::string Puzabrot::writeMain() const
{
    std::string str_initialization = writeInitialization();

    std::string str_calculation = writeCalculation();
    COND_RETURN(str_calculation.empty(), std::string());

    std::string str_checking = writeChecking();

    std::string str =
        "void main()\n"
        "{\n"
        "float re0 = borders.left + (borders.right - borders.left) * gl_FragCoord.x / winsizes.x;\n"
        "float im0 = borders.top - (borders.top - borders.bottom) * gl_FragCoord.y / winsizes.y;\n"
        "\n"
            + str_initialization +
        "int itrn;\n"
        "for (itrn = 0; itrn < itrn_max; ++itrn)\n"
        "{\n"
            + str_calculation +
        "\n"
            + str_checking +
        "}\n";

    switch (options_.coloring_mode)
    {
    case DEFAULT:
    {
        str +=
            "vec3 col = getColor(itrn);\n";
        break;
    }
    case TRACER:
    {
        str +=
            "vec3 col = getColor(itrn, sum);\n";
        break;
    }
    case COMPLEX_DOMAIN:
    {
        str += (options_.input_mode == Z_INPUT) ?
            "vec3 col = getColor(z);\n" :
            "vec3 col = getColor(vec2(x, y));\n";
        break;
    }
    case KALI:
    {
        str +=
            "vec3 col = getColor(sum);\n";
        break;
    }
    }

    str +=
        "gl_FragColor = vec4(col, 1.0);\n"
        "}";

    return str;
}

int Puzabrot::Tree2GLSL(const Tree<CalcData>& node, std::string* str) const
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
        switch (options_.input_mode)
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
            switch (options_.input_mode)
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
        switch (options_.input_mode)
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

Puzabrot::Synth::Synth(const Puzabrot* application) : audio_reset(true), audio_pause(false), application_(application)
{
    initialize(2, SYNTH_SAMPLE_RATE);
    setLoop(true);
}

void Puzabrot::Synth::setPoint(const vec2d& point)
{
    new_point_ = point;

    audio_reset = true;
    audio_pause = false;
}

void Puzabrot::Synth::setExpressions(const ExprTrees& expr_trees)
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

    c_point_ = (application_->options_.draw_mode == MAIN) ? new_point_ : application_->params_.julia_point;

    Calculator calc;
    const int steps = SYNTH_SAMPLE_RATE / SYNTH_MAX_FREQ;

    for (size_t i = 0; i < SYNTH_AUDIO_BUFF_SIZE; i += 2)
    {
        const int j = m_audio_time_ % steps;
        if (j == 0)
        {
            prev_point_ = point_;

            calc.clear();
            application_->initCalculator(calc, point_, c_point_);
            application_->Mapping(calc, expr_trees_, point_);

            if (sqrt(pow(point_.x, 2.0) + pow(point_.y, 2.0)) > application_->params_.limit)
            {
                audio_pause = true;
                return true;
            }

            d_  = point_ - mean_;
            dp_ = prev_point_ - mean_;

            pmag_ = sqrt(1e-12 + dp_.x * dp_.x + dp_.y * dp_.y);
            mag_  = sqrt(1e-12 + d_.x * d_.x + d_.y * d_.y);

            mean_ = mean_ * 0.99 + point_ * 0.01;

            double m = d_.magnitute2();
            if (m > 2.0)
            {
                d_ *= 2.0 / m;
            }

            m = dp_.magnitute2();
            if (m > 2.0)
            {
                dp_ *= 2.0 / m;
            }
        }

        double t = double(j) / double(steps);
        t = 0.5 - 0.5 * std::cos(t * std::atan(1.0) * 4.0);
        vec2d w = t * d_ + (1.0 - t) * dp_;

        m_samples_[i] = static_cast<int16_t>(std::min(std::max(w.x * volume_, -32000.0), 32000.0));
        m_samples_[i + 1] = static_cast<int16_t>(std::min(std::max(w.y * volume_, -32000.0), 32000.0));

        m_audio_time_ += 1;
    }

    return !audio_reset;
}