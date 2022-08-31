#ifndef PUZABROT_H
#define PUZABROT_H

#include "Calculator/Calculator.h"
#include "Engine2D.h"
#include "InputBox.h"
#include "ShaderEngine.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class Puzabrot final
{
public:
    using point_t = Base2D::point_t;

    Puzabrot(const sf::Vector2u& size = sf::Vector2u(640, 480));

    void run();

private:
    enum DrawingModes
    {
        MAIN,
        JULIA,
    };

    enum InputModes
    {
        Z_INPUT,
        XY_INPUT,
    };

    point_t PointTrace(point_t point, point_t c_point);
    void savePicture();
    void drawHelpMenu();
    int makeShader();
    bool textEntering() const;

    struct InputBoxes
    {
        InputBox x;
        InputBox y;
        InputBox z;
    } input_;

    sf::Font font_;

    struct Window;
    std::unique_ptr<Window> window_;

    struct Engine;
    std::unique_ptr<Engine> engine_;

    class Synth;
    std::unique_ptr<Synth> synth_;
};

struct Puzabrot::Window : public Engine2D
{
    Window(const sf::Vector2u& size, const sf::Font* font);

    void draw(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates::Default);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    const sf::Font* font_;
};

struct Puzabrot::Engine : public ShaderEngine
{
    const Window* window;

    struct Options final
    {
        int draw_mode = MAIN;
        int input_mode = Z_INPUT;
        bool sound_mode = false;
        bool coloring = false;
    } options;

    struct ExprTrees
    {
        Tree<CalcData> x;
        Tree<CalcData> y;
        Tree<CalcData> z;
    } expr_trees;

    struct Parameters
    {
        double limit;
        size_t itrn_max;
        point_t julia_point;
    } params;

    Engine(const sf::Vector2u& image_size, const Window* window_);

    void render();
    int makeShader();
    void initCalculator(Calculator& calc, point_t z, point_t c) const;
    void Mapping(Calculator& calc, ExprTrees& expr_trees, point_t& mapped_point) const;

private:
    const char* writeInitialization() const;
    std::string writeCalculation() const;
    std::string writeChecking() const;
    int Tree2GLSL(const Tree<CalcData>& node, std::string* str) const;
};

constexpr size_t SYNTH_AUDIO_BUFF_SIZE = 4096;
constexpr size_t SYNTH_SAMPLE_RATE = 48000;
constexpr size_t SYNTH_MAX_FREQ = 4000;

class Puzabrot::Synth : public sf::SoundStream
{
public:
    Synth(const Engine* engine);

    virtual void onSeek(sf::Time) override {}
    virtual bool onGetData(Chunk& data) override;

    void setPoint(point_t point);
    void setExpressions(const Engine::ExprTrees& expr_trees);

    bool audio_reset;
    bool audio_pause;
    bool sustain;

private:
    const Engine* engine_;
    Engine::ExprTrees expr_trees_;

    point_t point_;
    point_t c_point_;
    point_t new_point_;
    point_t prev_point_;

    int16_t m_samples_[SYNTH_AUDIO_BUFF_SIZE] = {};
    int32_t m_audio_time_ = 0;

    point_t d_;
    point_t dp_;
    point_t mean_;

    double mag_ = 0.0;
    double pmag_ = 0.0;
    double phase_ = 0.0;
    double volume_ = 8000.0;
};

#endif // PUZABROT_H