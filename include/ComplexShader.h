/*------------------------------------------------------------------------------
 * File:        ComplexShader.h                                                *
 * Description: Shader for drawing escape time sets                            *
 * Created:     3 nov 2021                                                     *
 * Author:      Artem Puzankov                                                 *
 * Email:       puzankov.ao@phystech.edu                                       *
 * GitHub:      https://github.com/hellopuza                                   *
 * Copyright © 2021 Artem Puzankov. All rights reserved.                       *
 *///---------------------------------------------------------------------------

#ifndef COMPLEXSHADER_H
#define COMPLEXSHADER_H

#include "Calculator/Calculator.h"
#include "ComplexSolver.h"

#include <SFML/Graphics.hpp>

namespace puza {

struct ComplexShader final
{
    sf::Shader        shader;
    sf::Sprite        sprite;
    sf::RenderTexture render_texture;

    struct ExprTrees
    {
        Tree<CalcData> x;
        Tree<CalcData> y;
        Tree<CalcData> z;
    };

    enum InputModes
    {
        Z_INPUT,
        XY_INPUT,
    };

    explicit ComplexShader(sf::Vector2u winsizes);

    void         draw(const ComplexSolver& solver, int draw_mode, bool coloring);
    int          make(const ExprTrees& expr_trees, int input_mode);
    void         updateWinSizes(size_t new_width, size_t new_height);
    static char* writeInitialization(int input_mode);
    char*        writeCalculation(const ExprTrees& expr_trees, int input_mode) const;
    static char* writeChecking(int input_mode);
    int          Tree2GLSL(const Tree<CalcData>& node, int input_mode, char* str_cur) const;
};

} // namespace puza

#endif // COMPLEXSHADER_H