/*------------------------------------------------------------------------------
 * File:        Puzabrot.h                                                  *
 * Description: Declaration of functions and data types for application     *
 * Created:     30 jun 2021                                                 *
 * Author:      Artem Puzankov                                              *
 * Email:       puzankov.ao@phystech.edu                                    *
 * GitHub:      https://github.com/hellopuza                                *
 * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
 *///------------------------------------------------------------------------

#ifndef PUZABROT_H
#define PUZABROT_H

#include "Calculator/Calculator.h"
#include "InputBox.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

namespace puza {

constexpr size_t DEFAULT_WIDTH  = 640;
constexpr size_t DEFAULT_HEIGHT = 480;

constexpr size_t SCREENSHOT_WIDTH = 7680;

constexpr double LIMIT         = 100.0f;
constexpr size_t MAX_ITERATION = 500;

constexpr double UPPER_BORDER = 1.3f;

constexpr double ZOOMING_RATIO = 0.33f;

constexpr size_t AUDIO_BUFF_SIZE = 4096;
constexpr size_t SAMPLE_RATE     = 48000;
constexpr size_t MAX_FREQ        = 4000;

const sf::String TITLE_STRING = "Puzabrot";

struct Screen
{
    unsigned int x1   = 0;
    unsigned int x2   = 0;
    unsigned int y1   = 0;
    unsigned int y2   = 0;
    double       zoom = 0;
};

struct ComplexFrame
{
    double re_left   = 0;
    double re_right  = 0;
    double im_bottom = 0;
    double im_top    = 0;

    ComplexFrame() = default;
    ComplexFrame(double re_left_, double re_right_, double im_bottom_, double im_top_);
};

enum ActionModes
{
    ZOOMING,
    POINT_TRACING,
    SOUNDING,
};

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

class Synth;

class Puzabrot
{
public:
    Puzabrot();
    void run();

private:
    sf::Vector2<double> Screen2Plane(sf::Vector2i point) const;
    void                updateWinSizes(size_t new_width, size_t new_height);
    void                toggleFullScreen();
    bool                InputBoxesHasFocus();
    bool                InputBoxesIsVisible();
    void                DrawSet();
    void                Zooming(double wheel_delta, sf::Vector2<double> point);
    int                 GetNewScreen(Screen& newscreen);
    void                changeBorders(Screen newscreen);
    void                initCalculator(Calculator& calc, sf::Vector2<double> z, sf::Vector2<double> c) const;
    void                Mapping(Calculator& calc, double& mapped_x, double& mapped_y);
    sf::Vector2<double> PointTrace(sf::Vector2<double> point, sf::Vector2<double> c_point);
    void                savePicture();
    void                drawHelpMenu();
    int                 makeShader();
    char*               writeShader();
    char*               writeInitialization() const;
    char*               writeCalculation();
    char*               writeChecking() const;
    int                 Tree2GLSL(Tree<CalcData>& node, char* str_cur);

    sf::Vector2u     winsizes_;
    sf::RenderWindow window_;
    ComplexFrame     borders_;

    size_t itrn_max_   = MAX_ITERATION;
    double lim_        = LIMIT;
    int    input_mode_ = Z_INPUT;
    int    draw_mode_  = MAIN;
    bool   coloring_   = false;

    sf::Vector2<double> julia_point_;

    InputBox input_box_x_;
    InputBox input_box_y_;
    InputBox input_box_z_;

    Tree<CalcData> expr_trees_[2];

    sf::Shader        shader_;
    sf::Sprite        sprite_;
    sf::RenderTexture render_texture_;

    friend class Synth;
};

class Synth : public sf::SoundStream
{
public:
    Synth(Puzabrot* puza);

    virtual void onSeek(sf::Time) override {}
    virtual bool onGetData(Chunk& data) override;

    void updateCalc();
    void SetPoint(sf::Vector2<double> point);

    bool   audio_reset_;
    bool   audio_pause_;
    bool   sustain_;
    double volume_;

private:
    Puzabrot*  puza_;
    Calculator calc_;

    sf::Vector2<double> point_;
    sf::Vector2<double> c_point_;
    sf::Vector2<double> new_point_;
    sf::Vector2<double> prev_point_;

    int16_t m_samples[AUDIO_BUFF_SIZE] = {};
    int32_t m_audio_time               = 0;

    double mean_x = 0;
    double mean_y = 0;

    double dx  = 0;
    double dy  = 0;
    double dpx = 0;
    double dpy = 0;
};

} // namespace puza

#endif // PUZABROT_H
