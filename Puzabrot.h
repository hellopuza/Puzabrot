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

};

//------------------------------------------------------------------------------
