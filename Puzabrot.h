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
#include <omp.h>


constexpr size_t DEFAULT_WIDTH  = 640;
constexpr size_t DEFAULT_HEIGHT = 480;

constexpr size_t DELTA_ZOOM    = 3000;
constexpr size_t LIMIT         = 100;
constexpr size_t MAX_ITERATION = 3000;

constexpr float UPPER_BORDER = 1.3f;

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
    MAIN  = 0,
    JULIA = 1,
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
    ComplexFrame      borders_;
    sf::Vector2u      winsizes_;

    InputBox           input_box_;
    Tree<CalcNodeData> expr_tree_;
    sf::Shader         shader_;
    sf::RenderTexture  render_texture_;
    sf::Sprite         sprite_;

    size_t itrn_max_ = MAX_ITERATION;
    size_t lim_      = LIMIT;

    void   updateWinSizes   (size_t new_width, size_t new_height);
    void   toggleFullScreen ();
    void   DrawSet          ();
    void   DrawJulia        (sf::Vector2f point);
    int    GetNewScreen     (Screen& newscreen);
    void   changeBorders    (Screen newscreen);
    void   PointTrace       (sf::Vector2i point, sf::Vector2f julia_point);
    void   savePict         ();
    int    makeShader       ();
    char*  writeShader      ();
    int    Tree2GLSL        (Node<CalcNodeData>* node_cur, char* str_cur);
};

//------------------------------------------------------------------------------

#endif // PUZABROT_H_INCLUDED
