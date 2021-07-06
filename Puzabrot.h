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
#include "ProgressBar.h"

#include <SFML/Graphics.hpp>
#include <assert.h>
#include <omp.h>

/*
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>
*/

constexpr size_t DEFAULT_WIDTH  = 640;
constexpr size_t DEFAULT_HEIGHT = 480;

constexpr size_t DELTA_ZOOM    = 3000;
constexpr size_t LIMIT         = 100;
constexpr size_t MAX_ITERATION = 3000;

constexpr double UPPER_BORDER = 1.3;

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
    double Re_left  = 0;
    double Re_right = 0;
    double Im_up    = 0;
    double Im_down  = 0;
};

//------------------------------------------------------------------------------

class Puzabrot
{
public:

    Puzabrot ();
   ~Puzabrot ();

    void run ();

private:

    sf::RenderWindow* window_ = nullptr;
    sf::VertexArray*  pointmap_ = nullptr;
    ComplexFrame      borders_;
    sf::Vector2i      winsizes_;

    Calculator calcs_[20] = {};

    size_t itrn_max_ = MAX_ITERATION;
    size_t lim_      = LIMIT;

    void      updateWinSizes   (size_t width, size_t height);
    void      toggleFullScreen ();
    int       DrawSet          ();
    sf::Color getColor         (int32_t itrn);
    int       GetNewScreen     (Screen& newscreen);
    void      changeBorders    (Screen newscreen);
    void      PointTrace       (sf::Vector2i point);
};

//------------------------------------------------------------------------------

#endif // PUZABROT_H_INCLUDED
