/*------------------------------------------------------------------------------
    * File:        Puzabrot.h                                                  *
    * Description: Declaration of functions and data types for application     *
    * Created:     30 jun 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#define _CRT_SECURE_NO_WARNINGS

#include <SFML/Graphics.hpp>
#include <assert.h>
#include <string.h>
#include <omp.h>

#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>

const int DEFAULT_WIDTH  = 640;
const int DEFAULT_HEIGHT = 480;

const sf::String title_string = "Puzabrot";

//------------------------------------------------------------------------------

class Puzabrot
{
public:

    Puzabrot ();
   ~Puzabrot ();

    void run ();

private:

    /*

    sf::RenderWindow* window_ = nullptr;
    sf::VertexArray*  pointmap_ = nullptr;
    cmplxborders      borders_;
    sf::Vector2i      winsizes_;

    size_t delta_zoom_ = 3000;
    double lim_        = 100;
    size_t itrn_max_   = 3000;

    void initBorders ();

    int       GetNewScreen   (screen& newscreen);
    void      DrawMandelbrot ();
    void      changeBorders  (screen newscreen);
    sf::Color getColor       (int32_t itrn);
    void      savePict       ();
    void      PointTrace     (sf::Vector2i point);
    */
};

//------------------------------------------------------------------------------
