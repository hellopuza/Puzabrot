#ifndef PUZABROT_H
#define PUZABROT_H

#include "Calculator/Calculator.h"
#include "Application/ShaderApplication.h"
#include "UI/UI.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class Puzabrot final : public ShaderApplication
{
public:
    Puzabrot();

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

    enum ColorModes
    {
        DEFAULT,
        TRACER,
        COMPLEX_DOMAIN,
        KALI,
    };

    UI ui_;

    struct Options final
    {
        int draw_mode = MAIN;
        int input_mode = Z_INPUT;
        int coloring_mode = DEFAULT;
        bool sound_mode = false;
        bool showing_grid = false;
        bool showing_trace = false;
        bool julia_dragging = false;
        bool right_pressed = false;
        bool was_screenshot = false;
    } options_;

    struct Parameters
    {
        double limit;
        double frequency;
        size_t itrn_max;
        vec2d julia_point;
        vec2d orbit;
        vec2d c_point;
    } params_;

    struct ExprTrees
    {
        Tree<CalcData> x;
        Tree<CalcData> y;
        Tree<CalcData> z;
    } expr_trees_;

    class Synth;
    std::unique_ptr<Synth> synth_;

    void prerun() override;
    void handleAppEvent(const sf::Event& event) override;
    void activity() override;
    void postrun() override;

    vec2d PointTrace(const vec2d& point, const vec2d& c_point);
    void initCalculator(Calculator& calc, const vec2d& z, const vec2d& c) const;
    void Mapping(Calculator& calc, ExprTrees& expr_trees, vec2d& mapped_point) const;
    void savePicture();
    int makeShader();
    void render();
    int writeShader();
    std::string writeFunctions() const;
    std::string writeColorFunction() const;
    std::string writeInitialization() const;
    std::string writeCalculation() const;
    std::string writeChecking() const;
    std::string writeMain() const;
    int Tree2GLSL(const Tree<CalcData>& node, std::string* str) const;
};

constexpr size_t SYNTH_AUDIO_BUFF_SIZE = 4096;
constexpr size_t SYNTH_SAMPLE_RATE = 48000;
constexpr size_t SYNTH_MAX_FREQ = 4000;

class Puzabrot::Synth : public sf::SoundStream
{
public:
    Synth(const Puzabrot* application);

    virtual void onSeek(sf::Time) override {}
    virtual bool onGetData(Chunk& data) override;

    void setPoint(const vec2d& point);
    void setExpressions(const ExprTrees& expr_trees);

    bool audio_reset;
    bool audio_pause;

private:
    const Puzabrot* application_;
    ExprTrees expr_trees_;

    vec2d point_;
    vec2d c_point_;
    vec2d new_point_;
    vec2d prev_point_;

    int16_t m_samples_[SYNTH_AUDIO_BUFF_SIZE] = {};
    int32_t m_audio_time_ = 0;

    vec2d d_;
    vec2d dp_;
    vec2d mean_;

    double mag_ = 0.0;
    double pmag_ = 0.0;
    double phase_ = 0.0;
    double volume_ = 8000.0;
};

#endif // PUZABROT_H