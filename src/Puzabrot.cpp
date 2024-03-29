#include "Puzabrot.h"
#include "Utils.h"

#include <cstring>

#define COND_RETURN(cond, ret) \
    if (cond)                  \
    {                          \
        return (ret);          \
    } //

constexpr float LIMIT = 100.0F;
constexpr float FREQUENCY = 5.0F;
constexpr float UPPER_BORDER = 1.3F;
constexpr float ZOOMING_RATIO = 0.2F;
constexpr size_t MAX_ITERATION = 500;
constexpr float GRID_FONT_SIZE = 14.0F;
constexpr float UI_FONT_SIZE = 16.0F;
constexpr size_t SCREENSHOT_WIDTH = 7680;

static const char* TITLE_STRING = "Puzabrot";
static const char* FONT_LOCATION = "assets/consola.ttf";
static const vec2u WINDOW_SIZE = { 640, 480 };
static const vec2f INPUT_BUTTON_POS = { 10.0F, 10.0F };
static const vec2f INPUT_BOX_POS = { 10.0F, 35.0F };
static const vec2f RENDER_BUTTON_POS = { 10.0F, 125.0F };
static const vec2f GRID_BUTTON_POS = { 10.0F, 150.0F };
static const vec2f SOUND_BUTTON_POS = { 10.0F, 175.0F };
static const vec2f ITERATION_BUTTON_POS = { 10.0F, 200.0F };
static const vec2f PARAMETER_BUTTON_POS = { 10.0F, 225.0F };
static const vec2f ANTI_ALIASING_BUTTON_POS = { 10.0F, 250.0F };

#define INPUT_BUTTON static_cast<SwitchButton*>(ui_.getVidget("input_button"))
#define INPUT_X static_cast<InputBox*>(ui_.getVidget("input_x"))
#define INPUT_Y static_cast<InputBox*>(ui_.getVidget("input_y"))
#define INPUT_Z static_cast<InputBox*>(ui_.getVidget("input_z"))
#define RENDER_BUTTON static_cast<SwitchButton*>(ui_.getVidget("render_button"))
#define GRID_BUTTON static_cast<Button*>(ui_.getVidget("grid_button"))
#define SOUND_BUTTON static_cast<Button*>(ui_.getVidget("sound_button"))
#define ITERATION_BUTTON static_cast<Button*>(ui_.getVidget("iteration_button"))
#define PARAMETER_BUTTON static_cast<Button*>(ui_.getVidget("parameter_button"))
#define ANTI_ALIASING_BUTTON static_cast<SwitchButton*>(ui_.getVidget("antialiasing_button"))

#define SET_INPUT_Y_POS INPUT_Y->setPosition(INPUT_X->getPosition() + vec2f(0.0F, INPUT_X->getSize().y + 3.0F))

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
    setBorders(-UPPER_BORDER, UPPER_BORDER, 0.5F);

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

    ui_.addVidget("render_button", new SwitchButton(getFont(), UI_FONT_SIZE, RENDER_BUTTON_POS));
    RENDER_BUTTON->addText("DEFAULT");
    RENDER_BUTTON->addText("TRACER");
    RENDER_BUTTON->addText("DOMAIN");
    RENDER_BUTTON->addText("KALI");

    ui_.addVidget("grid_button", new Button(getFont(), UI_FONT_SIZE, GRID_BUTTON_POS));
    GRID_BUTTON->setText("GRID");

    ui_.addVidget("sound_button", new Button(getFont(), UI_FONT_SIZE, SOUND_BUTTON_POS));
    SOUND_BUTTON->setText("SOUND");

    ui_.addVidget("iteration_button", new Button(getFont(), UI_FONT_SIZE, ITERATION_BUTTON_POS));
    ui_.addVidget("parameter_button", new Button(getFont(), UI_FONT_SIZE, PARAMETER_BUTTON_POS));

    ui_.addVidget("antialiasing_button", new SwitchButton(getFont(), UI_FONT_SIZE, ANTI_ALIASING_BUTTON_POS));
    ANTI_ALIASING_BUTTON->addText("NO ANTI ALIASING");
    ANTI_ALIASING_BUTTON->addText("ANTI ALIASING x4");
    ANTI_ALIASING_BUTTON->addText("ANTI ALIASING x9");
    ANTI_ALIASING_BUTTON->addText("ANTI ALIASING x16");
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
    if (ui_.is_visible && ui_.handleEvent(event))
    {
        INPUT_Y->is_visible = INPUT_X->is_visible;

        // Enter expression from input box
        if (INPUT_X->TextEntered() || INPUT_Y->TextEntered() || INPUT_Z->TextEntered())
        {
            AST::Error err = makeShader();

            if (err == AST::Error::OK)
            {
                render();
                options_.fractal_mode = MAIN;
            }

            switch (options_.input_mode)
            {
            case Z_INPUT:
            {
                if (err != AST::Error::OK)
                {
                    INPUT_Z->setOutput(sf::String(ASTStringError(err)));
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
                    if (err != AST::Error::OK)
                    {
                        INPUT_X->setOutput(sf::String(ASTStringError(err)));
                        SET_INPUT_Y_POS;
                    }
                    else
                    {
                        INPUT_X->setOutput(sf::String());
                        SET_INPUT_Y_POS;
                    }
                }
                else if (err != AST::Error::OK)
                {
                    INPUT_Y->setOutput(sf::String(ASTStringError(err)));
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

        // Toggle render mode
        if (options_.rendering_mode != RENDER_BUTTON->value())
        {
            options_.rendering_mode = RENDER_BUTTON->value();
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

        // Toggle antialiasing level
        if (options_.antialiasing != ANTI_ALIASING_BUTTON->value())
        {
            options_.antialiasing = ANTI_ALIASING_BUTTON->value();
            makeShader();
            render();
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
        options_.fractal_mode = MAIN;
        params_.limit = LIMIT;
        params_.frequency = FREQUENCY;
        params_.itrn_max = MAX_ITERATION;
        setBorders(-UPPER_BORDER, UPPER_BORDER, 0.5F);
        makeShader();
        render();
    }

    // Take a screenshot
    else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space) && !TEXT_ENTERING)
    {
        savePicture();
    }

    // Julia set drawing
    else if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tab) && !TEXT_ENTERING)
    {
        if (options_.fractal_mode != JULIA)
        {
            options_.julia_dragging = true;
            options_.fractal_mode = JULIA;
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
                options_.fractal_mode = MAIN;
                makeShader();
                render();
            }
        }
    }

    // Change max iterations
    else if ((event.type == sf::Event::MouseWheelMoved) && (ITERATION_BUTTON->pressed()))
    {
        size_t new_itrn = static_cast<size_t>(static_cast<float>(params_.itrn_max) * std::pow(1.03F, static_cast<float>(event.mouseWheel.delta)));
        params_.itrn_max = (new_itrn == params_.itrn_max) ? params_.itrn_max + 1 : new_itrn;
        params_.itrn_max = (params_.itrn_max < 1) ? 1 : params_.itrn_max;
        render();
    }

    // Change parameter
    else if ((event.type == sf::Event::MouseWheelMoved) && (PARAMETER_BUTTON->pressed()))
    {
        switch (options_.rendering_mode)
        {
        case DEFAULT:
        case TRACER:
        {
            params_.limit *= std::pow(2.0F, static_cast<float>(event.mouseWheel.delta));
            break;
        }
        case COMPLEX_DOMAIN:
        {
            break;
        }
        case KALI:
        {
            params_.frequency *= std::pow(1.005F, static_cast<float>(event.mouseWheel.delta));
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

    ITERATION_BUTTON->setText(std::string("ITERATION ") + std::to_string(params_.itrn_max));

    switch (options_.rendering_mode)
    {
    case DEFAULT:
    case TRACER:
    {
        PARAMETER_BUTTON->show();
        PARAMETER_BUTTON->setText(std::string("LIMIT ") + std::to_string(params_.limit));
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
        PARAMETER_BUTTON->setText(std::string("FREQUENCY ") + std::to_string(params_.frequency));
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

vec2f Puzabrot::PointTrace(const vec2f& point, const vec2f& c_point)
{
    vec2f c = (options_.fractal_mode == MAIN) ? c_point : params_.julia_point;
    vec2f point1 = point;
    vec2f point2;

    for (size_t i = 0; i < params_.itrn_max; ++i)
    {
        point2 = Mapping(expr_trees_, c, point1);

        if (point2.magnitude() > params_.limit)
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

vec2f Puzabrot::Mapping(ExprTrees& expr_trees, const vec2f& c, vec2f& z) const
{
    switch (options_.input_mode)
    {
    case Z_INPUT:
    {
        auto result = expr_trees.z({ {"c", {c.x, c.y}}, {"z", {z.x, z.y}} });
        return vec2f(real(result), imag(result));
    }
    case XY_INPUT:
    {
        auto result_x = expr_trees.x({ {"cx", c.x}, {"cy", c.y}, {"x", z.x}, {"y", z.y} });
        auto result_y = expr_trees.y({ {"cx", c.x}, {"cy", c.y}, {"x", z.x}, {"y", z.y} });
        return vec2f(result_x, result_y);
    }
    }
    return vec2f();
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

AST::Error Puzabrot::makeShader()
{
    AST::Error err = AST::Error::OK;
    switch (options_.input_mode)
    {
    case Z_INPUT:
    {
        expr_trees_.z = ASTz(INPUT_Z->getInput(), reinterpret_cast<ASTz::Error*>(&err));
        COND_RETURN(err != AST::Error::OK, err);
        break;
    }
    case XY_INPUT:
    {
        expr_trees_.x = ASTx(INPUT_X->getInput(), reinterpret_cast<ASTx::Error*>(&err));
        COND_RETURN(err != AST::Error::OK, err);

        expr_trees_.y = ASTx(INPUT_Y->getInput(), reinterpret_cast<ASTx::Error*>(&err));
        COND_RETURN(err != AST::Error::OK, err);
        break;
    }
    }
    synth_->setExpressions(expr_trees_);

    COND_RETURN(writeShader(), AST::Error::UNIDENTIFIED_VARIABLE);

    return err;
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
    shader_.setUniform("frequency", static_cast<float>(1.0F / (1.0F + std::exp(-params_.frequency))));
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
        "vec2 clog(vec2 a)\n"
        "{\n"
        "    return vec2(log(a.x * a.x + a.y * a.y) * 0.5, arg(a));\n"
        "}\n"
        "\n"
        "vec2 clog10(vec2 a)\n"
        "{\n"
        "    return clog(a) / log(10.0);\n"
        "}\n"
        "\n"
        "vec2 cexp(vec2 a)\n"
        "{\n"
        "    return vec2(cos(a.y), sin(a.y)) * exp(a.x);\n"
        "}\n"
        "\n"
        "vec2 cpow(vec2 a, vec2 b)\n"
        "{\n"
        "    return cexp(cmul(b, clog(a)));\n"
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
        "    return cmul(vec2(0.0, -1.0), clog(cadd(cmul(I, a), csqrt(csub(ONE, cmul(a, a))))));\n"
        "}\n"
        "\n"
        "vec2 carccos(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.0, -1.0), clog(cadd(a, csqrt(csub(cmul(a, a), ONE)))));\n"
        "}\n"
        "\n"
        "vec2 carctan(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.0, 0.5), csub(clog(cadd(I, a)), clog(csub(I, a))));\n"
        "}\n"
        "\n"
        "vec2 carccot(vec2 a)\n"
        "{\n"
        "    return csub(vec2(PI / 2, 0.0), cmul(vec2(0.0, 0.5), csub(clog(cadd(I, a)), clog(csub(I, a)))));\n"
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
        "    return clog(cadd(a, csqrt(cadd(cmul(a, a), ONE))));\n"
        "}\n"
        "\n"
        "vec2 carccosh(vec2 a)\n"
        "{\n"
        "    return clog(cadd(a, csqrt(csub(cmul(a, a), ONE))));\n"
        "}\n"
        "\n"
        "vec2 carctanh(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.5, 0.0), csub(clog(cadd(ONE, a)), clog(csub(ONE, a))));\n"
        "}\n"
        "\n"
        "vec2 carccoth(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.5, 0.0), csub(clog(cadd(ONE, a)), clog(csub(ONE, a))));\n"
        "}\n";
}

std::string Puzabrot::writeColorFunction() const
{
    std::string str;
    switch (options_.rendering_mode)
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

        str += (options_.rendering_mode == DEFAULT) ?
            "vec3 getColor(int itrn)\n" :
            "vec3 getColor(int itrn, vec3 sum)\n";

        str +=
            "{\n"
            "if (itrn < itrn_max)\n"
            "{\n"
            "    float x = float(itrn) * 4.0 / 255.0;\n"
            "    return vec3(pf(x - 3.0F), pf(x - 5.0F), pf(x - 7.0F));\n"
            "}\n";

        str += (options_.rendering_mode == DEFAULT) ?
            "return vec3(0.0, 0.0, 0.0);\n"
            "}\n" :
            "return sin(abs(sum / itrn_max * 5.0)) * 0.5 + 0.5;\n"
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

        str += (options_.fractal_mode == MAIN) ?
            "vec2 c = vec2(re0, im0);\n" :
            "vec2 c = vec2(julia_point.x, julia_point.y);";

        str += (options_.rendering_mode == TRACER) ?
            "vec2 pz = z;\n" :
            "";
        break;
    }
    case XY_INPUT:
    {
        str +=
            "float x = re0;\n"
            "float y = im0;\n";

        str += (options_.fractal_mode == MAIN) ?
            "float cx = re0;\n"
            "float cy = im0;\n" :
            "float cx = julia_point.x;\n"
            "float cy = julia_point.y;\n";

        str += (options_.rendering_mode == TRACER) ?
            "vec2 pz = vec2(x, y);\n" :
            "";

        break;
    }
    }

    str +=
        "vec3 sum = vec3(0.0, 0.0, 0.0);\n";

    str += (options_.rendering_mode == KALI) ?
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
        str += (options_.rendering_mode == TRACER) ?
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
        str += (options_.rendering_mode == TRACER) ?
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
    switch (options_.rendering_mode)
    {
    case DEFAULT:
    case TRACER:
    {
        str += (options_.input_mode == XY_INPUT) ?
            "vec2 z = vec2(x, y);\n" :
            "";

        str +=
            "if (cabs(z).x > limit) break;\n";

        str += (options_.rendering_mode == TRACER) ?
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
            "float r = x * x + y * y;\n"
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

    std::string str;
    switch (options_.antialiasing)
    {
    case 0: str += "const int AA = 1;\n"; break;
    case 1: str += "const int AA = 2;\n"; break;
    case 2: str += "const int AA = 3;\n"; break;
    case 3: str += "const int AA = 4;\n"; break;
    }

    str +=
        "void main()\n"
        "{\n"
        "vec3 col = vec3(0.0);\n"
        "for (int sx = 0; sx < AA; sx++)\n"
        "for (int sy = 0; sy < AA; sy++)\n"
        "{\n"
        "float re0 = borders.left + (borders.right - borders.left) * (gl_FragCoord.x + float(sx) / float(AA)) / winsizes.x;\n"
        "float im0 = borders.top  - (borders.top - borders.bottom) * (gl_FragCoord.y + float(sy) / float(AA)) / winsizes.y;\n"
        "\n"
            + str_initialization +
        "int itrn;\n"
        "for (itrn = 0; itrn < itrn_max; ++itrn)\n"
        "{\n"
            + str_calculation +
        "\n"
            + str_checking +
        "}\n";

    switch (options_.rendering_mode)
    {
    case DEFAULT:
    {
        str +=
            "col += getColor(itrn);\n";
        break;
    }
    case TRACER:
    {
        str +=
            "col += getColor(itrn, sum);\n";
        break;
    }
    case COMPLEX_DOMAIN:
    {
        str += (options_.input_mode == Z_INPUT) ?
            "col += getColor(z);\n" :
            "col += getColor(vec2(x, y));\n";
        break;
    }
    case KALI:
    {
        str +=
            "col += getColor(sum);\n";
        break;
    }
    }

    str +=
        "}\n"
        "gl_FragColor = vec4(col / float(AA * AA), 1.0);\n"
        "}";

    return str;
}

using ASTNodez = ast::ASTNode<std::complex<float>>;
using OperationNodez = ast::OperationNode<std::complex<float>>;
using FunctionNodez = ast::FunctionNode<std::complex<float>>;
using VariableNodez = ast::VariableNode<std::complex<float>>;
using NumberNodez = ast::NumberNode<std::complex<float>>;

int Puzabrot::Tree2GLSL(const ASTz& node, std::string* str) const
{
    switch (node.value().get()->NodeType())
    {
    case ASTNodez::Type::OPERATION:
    {
        switch (static_cast<OperationNodez*>(node.value().get())->type)
        {
        case OperationNodez::Type::ADD: *str += "cadd("; break;
        case OperationNodez::Type::SUB: *str += "csub("; break;
        case OperationNodez::Type::MUL: *str += "cmul("; break;
        case OperationNodez::Type::DIV: *str += "cdiv("; break;
        case OperationNodez::Type::POW: *str += "cpow("; break;
        default: break;
        }

        if (node.branches_num() < 2)
        {
            *str += "vec2(0.0, 0.0), ";

            int err = Tree2GLSL(*static_cast<const ASTz*>(&(node[0])), str);
            COND_RETURN(err, err);
        }
        else
        {
            int err = Tree2GLSL(*static_cast<const ASTz*>(&(node[0])), str);
            COND_RETURN(err, err);

            *str += ", ";

            err = Tree2GLSL(*static_cast<const ASTz*>(&(node[1])), str);
            COND_RETURN(err, err);
        }

        *str += ")";
        break;
    }
    case ASTNodez::Type::FUNCTION:
    {
        *str += "c" + std::string(ASTz::FunctionName(static_cast<FunctionNodez*>(node.value().get())->type)) + "(";

        int err = Tree2GLSL(*static_cast<const ASTz*>(&(node[0])), str);
        COND_RETURN(err, err);

        *str += ")";
        break;
    }
    case ASTNodez::Type::VARIABLE:
    {
        auto name = static_cast<VariableNodez*>(node.value().get())->name;
        COND_RETURN((name != "z") && (name != "c"), -1);

        *str += name;
        break;
    }
    case ASTNodez::Type::NUMBER:
    {
        auto number = static_cast<NumberNodez*>(node.value().get())->number;
        *str += "vec2(" + std::to_string(real(number)) + ", " + std::to_string(imag(number)) + ")";
        break;
    }
    default: break;
    }

    return 0;
}

using ASTNodex = ast::ASTNode<float>;
using OperationNodex = ast::OperationNode<float>;
using FunctionNodex = ast::FunctionNode<float>;
using VariableNodex = ast::VariableNode<float>;
using NumberNodex = ast::NumberNode<float>;

int Puzabrot::Tree2GLSL(const ASTx& node, std::string* str) const
{
    switch (node.value().get()->NodeType())
    {
    case ASTNodex::Type::OPERATION:
    {
        switch (static_cast<OperationNodex*>(node.value().get())->type)
        {
        case OperationNodex::Type::ADD: *str += "cadd("; break;
        case OperationNodex::Type::SUB: *str += "csub("; break;
        case OperationNodex::Type::MUL: *str += "cmul("; break;
        case OperationNodex::Type::DIV: *str += "cdiv("; break;
        case OperationNodex::Type::POW: *str += "cpow("; break;
        default: break;
        }

        if (node.branches_num() < 2)
        {
            *str += "vec2(0.0, 0.0), ";

            int err = Tree2GLSL(*static_cast<const ASTx*>(&(node[0])), str);
            COND_RETURN(err, err);
        }
        else
        {
            int err = Tree2GLSL(*static_cast<const ASTx*>(&(node[0])), str);
            COND_RETURN(err, err);

            *str += ", ";

            err = Tree2GLSL(*static_cast<const ASTx*>(&(node[1])), str);
            COND_RETURN(err, err);
        }

        *str += ")";
        break;
    }
    case ASTNodex::Type::FUNCTION:
    {
        *str += "c" + std::string(ASTx::FunctionName(static_cast<FunctionNodex*>(node.value().get())->type)) + "(";

        int err = Tree2GLSL(*static_cast<const ASTx*>(&(node[0])), str);
        COND_RETURN(err, err);

        *str += ")";
        break;
    }
    case ASTNodex::Type::VARIABLE:
    {
        auto name = static_cast<VariableNodex*>(node.value().get())->name;
        COND_RETURN((name != "x") && (name != "y") && (name != "cx") && (name != "cy"), -1);

        *str += "vec2(" + name + ", 0.0)";
        break;
    }
    case ASTNodex::Type::NUMBER:
    {
        auto number = static_cast<NumberNodex*>(node.value().get())->number;
        *str += "vec2(" + std::to_string(number) + ", 0.0)";
        break;
    }
    default: break;
    }

    return 0;
}

const char* Puzabrot::ASTStringError(const AST::Error& err)
{
    switch (err)
    {
    case AST::Error::SYNTAX_ERROR: return "syntax error";
    case AST::Error::NO_CLOSE_BRACKET: return "no close bracket";
    case AST::Error::UNIDENTIFIED_OPERATION: return "unidentified operation";
    case AST::Error::UNIDENTIFIED_FUNCTION: return "unidentified function";
    case AST::Error::UNIDENTIFIED_VARIABLE: return "unidentified variable";
    default: break;
    }
    return "";
}

Puzabrot::Synth::Synth(const Puzabrot* application) : audio_reset(true), audio_pause(false), application_(application)
{
    initialize(2, SYNTH_SAMPLE_RATE);
    setLoop(true);
}

void Puzabrot::Synth::setPoint(const vec2f& point)
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

        mag_ = 0.0F;
        pmag_ = 0.0F;
        phase_ = 0.0F;

        volume_ = 8000.0F;

        audio_reset = false;
    }

    COND_RETURN(audio_pause, true);

    c_point_ = (application_->options_.fractal_mode == MAIN) ? new_point_ : application_->params_.julia_point;

    const int steps = SYNTH_SAMPLE_RATE / SYNTH_MAX_FREQ;

    for (size_t i = 0; i < SYNTH_AUDIO_BUFF_SIZE; i += 2)
    {
        const int j = m_audio_time_ % steps;
        if (j == 0)
        {
            prev_point_ = point_;
            point_ = application_->Mapping(expr_trees_, c_point_, point_);

            if (point_.magnitude() > application_->params_.limit)
            {
                audio_pause = true;
                return true;
            }

            d_  = point_ - mean_;
            dp_ = prev_point_ - mean_;

            pmag_ = std::sqrt(1e-12F + dp_.magnitude());
            mag_  = std::sqrt(1e-12F + d_.magnitude());

            mean_ = mean_ * 0.99F + point_ * 0.01F;

            float m = d_.magnitude2();
            if (m > 2.0F)
            {
                d_ *= 2.0F / m;
            }

            m = dp_.magnitude2();
            if (m > 2.0F)
            {
                dp_ *= 2.0F / m;
            }
        }

        float t = float(j) / float(steps);
        t = 0.5F - 0.5F * std::cos(t * std::atan(1.0F) * 4.0F);
        vec2f w = t * d_ + (1.0F - t) * dp_;

        m_samples_[i] = static_cast<int16_t>(std::min(std::max(w.x * volume_, -32000.0F), 32000.0F));
        m_samples_[i + 1] = static_cast<int16_t>(std::min(std::max(w.y * volume_, -32000.0F), 32000.0F));

        m_audio_time_ += 1;
    }

    return !audio_reset;
}