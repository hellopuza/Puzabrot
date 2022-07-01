#ifndef COMPLEXSHADER_H
#define COMPLEXSHADER_H

#include "Calculator/Calculator.h"
#include "ComplexHolder.h"

#include <SFML/Graphics.hpp>

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

    void         draw(const ComplexHolder& holder, int draw_mode, bool coloring);
    int          make(const ExprTrees& expr_trees, int input_mode);
    void         updateWinSizes(size_t new_width, size_t new_height);
    static char* writeInitialization(int input_mode);
    char*        writeCalculation(const ExprTrees& expr_trees, int input_mode) const;
    static char* writeChecking(int input_mode);
    int          Tree2GLSL(const Tree<CalcData>& node, int input_mode, char* str_cur) const;
};

#endif // COMPLEXSHADER_H