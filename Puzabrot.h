/*------------------------------------------------------------------------------
    * File:        Puzabrot.h                                                  *
    * Description: Declaration of functions and data types for application     *
    * Created:     30 jun 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef PUZABROT_H_INCLUDED
#define PUZABROT_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS

#include "Calculator/Calculator.h"

#include "InputBox.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

constexpr size_t DEFAULT_WIDTH  = 640;
constexpr size_t DEFAULT_HEIGHT = 480;

constexpr size_t DELTA_ZOOM    = 3000;
constexpr size_t LIMIT         = 100;
constexpr size_t MAX_ITERATION = 3000;

constexpr float UPPER_BORDER = 1.3f;

constexpr float ZOOMING_RATIO = 0.33f;

constexpr int AUDIO_BUFF_SIZE = 4096;
constexpr int SAMPLE_RATE     = 48000;
constexpr int MAX_FREQ        = 4000;

const sf::String title_string = "Puzabrot";

struct Screen
{
    int    x1   = -1;
    int    x2   = -1;
    int    y1   = -1;
    int    y2   = -1;
    double zoom =  0;
};

struct ComplexFrame
{
    float Re_left  = 0;
    float Re_right = 0;
    float Im_down  = 0;
    float Im_up    = 0;
};

enum ActionModes
{
    ZOOMING       ,
    POINT_TRACING ,
    SOUNDING      ,
};

enum DrawingModes
{
    MAIN  ,
    JULIA ,
};

enum InputModes
{
    Z_INPUT  ,
    XY_INPUT ,
};

class Synth;

//------------------------------------------------------------------------------

class Puzabrot
{
public:

    Puzabrot ();
   ~Puzabrot ();

    void run ();

private:

    sf::RenderWindow* window_ = nullptr;
    ComplexFrame      borders_;
    sf::Vector2u      winsizes_;

    size_t itrn_max_   = MAX_ITERATION;
    size_t lim_        = LIMIT;
    int    input_mode_ = Z_INPUT;
    int    draw_mode_  = MAIN;
    bool   coloring_   = false;

    sf::Vector2f julia_point_ = sf::Vector2f(0, 0);

    InputBox           input_box_x_;
    InputBox           input_box_y_;
    InputBox           input_box_z_;
    Tree<CalcNodeData> expr_trees_[2];

    sf::Shader         shader_;
    sf::Sprite         sprite_;
    sf::RenderTexture  render_texture_;

    sf::Vector2f Screen2Plane        (sf::Vector2i point);
    void         updateWinSizes      (size_t new_width, size_t new_height);
    void         toggleFullScreen    ();
    bool         InputBoxesHasFocus  ();
    bool         InputBoxesIsVisible ();
    void         DrawSet             ();
    void         DrawJulia           ();
    void         Zooming             (int wheel_delta, sf::Vector2f point);
    int          GetNewScreen        (Screen& newscreen);
    void         changeBorders       (Screen newscreen);
    void         initCalculator      (Calculator& calc, float x, float y, float cx, float cy);
    void         Mapping             (Calculator& calc, float& mapped_x, float& mapped_y);
    sf::Vector2f PointTrace          (sf::Vector2f point, sf::Vector2f c_point);
    void         savePicture         ();
    void         drawHelpMenu        ();
    int          makeShader          ();
    char*        writeShader         ();
    char*        writeInitialization ();
    char*        writeCalculation    ();
    char*        writeChecking       ();
    int          Tree2GLSL           (Node<CalcNodeData>* node_cur, char* str_cur);

    friend class Synth;
};

//------------------------------------------------------------------------------

class Synth : public sf::SoundStream
{
public:

    Puzabrot* puza_ = nullptr;
    Calculator calc_;

    bool audio_reset_;
    bool audio_pause_;
    bool sustain_;
    double volume_;

    sf::Vector2f point_;
    sf::Vector2f c_point_;
    sf::Vector2f new_point_;
    sf::Vector2f prev_point_;

                 Synth      (Puzabrot* puza);
            void updateCalc ();
            void SetPoint   (sf::Vector2f point);
    virtual void onSeek     (sf::Time timeOffset) override {}
    virtual bool onGetData  (Chunk& data)         override;

    int16_t m_samples[AUDIO_BUFF_SIZE] = {};
    int32_t m_audio_time = 0;

    double mean_x = 0;
    double mean_y = 0;

    double dx  = 0;
    double dy  = 0;
    double dpx = 0;
    double dpy = 0;
};

//------------------------------------------------------------------------------

#endif // PUZABROT_H_INCLUDED
