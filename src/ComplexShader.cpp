#include "ComplexShader.h"

#include <cassert>
#include <cstring>

#define COND_RETURN(cond, ret) \
    if (cond)                  \
    {                          \
        return (ret);          \
    } //

ComplexShader::ComplexShader(sf::Vector2u winsizes)
{
    render_texture.create(winsizes.x, winsizes.y);
    sprite = sf::Sprite(render_texture.getTexture());
}

void ComplexShader::updateWinSizes(size_t new_width, size_t new_height)
{
    render_texture.create(static_cast<unsigned int>(new_width), static_cast<unsigned int>(new_height));
    sprite = sf::Sprite(render_texture.getTexture());
}

void ComplexShader::draw(const ComplexHolder& holder, int draw_mode, bool coloring)
{
    shader.setUniform("borders", sf::Glsl::Vec4(static_cast<float>(holder.borders.re_left),
                                                static_cast<float>(holder.borders.re_right),
                                                static_cast<float>(holder.borders.im_bottom),
                                                static_cast<float>(holder.borders.im_top)));

    shader.setUniform("winsizes", sf::Glsl::Ivec2(sf::Vector2i(holder.winsizes)));

    shader.setUniform("itrn_max", static_cast<int>(holder.itrn_max));
    shader.setUniform("limit", static_cast<float>(holder.limit));

    shader.setUniform("drawing_mode", draw_mode);
    shader.setUniform("coloring", coloring);

    shader.setUniform("julia_point", sf::Glsl::Vec2(sf::Vector2f(holder.julia_point)));

    render_texture.draw(sprite, &shader);
}

int ComplexShader::make(const ExprTrees& expr_trees, int input_mode)
{
    char* str_initialization = writeInitialization(input_mode);

    char* str_calculation = writeCalculation(expr_trees, input_mode);
    COND_RETURN(str_calculation == nullptr, -1);

    char* str_checking = writeChecking(input_mode);

    const size_t str_shader_size = 10000;
    char* str_shader = new char[str_shader_size] {};

    sprintf(str_shader,
        "#version 130\n"
        "\n"
        "const float NIL = 1e-9;\n"
        "const float PI  = atan(1.0) * 4;\n"
        "const float E   = exp(1.0);\n"
        "const vec2  I   = vec2(0, 1);\n"
        "const vec2  ONE = vec2(1, 0);\n"
        "\n"
        "uniform vec4  borders;\n"
        "uniform ivec2 winsizes;\n"
        "uniform int   itrn_max;\n"
        "uniform float limit;\n"
        "uniform int   drawing_mode;\n"
        "uniform bool  coloring;\n"
        "uniform vec2  julia_point;\n"
        "\n"
        "vec2 conj(vec2 a)\n"
        "{\n"
        "    return vec2(a.x, -a.y);\n"
        "}\n"
        "\n"
        "float norm(vec2 a)\n"
        "{\n"
        "    return a.x * a.x + a.y * a.y;\n"
        "}\n"
        "\n"
        "float arg(vec2 a)\n"
        "{\n"
        "    return atan(a.y, a.x);\n"
        "}\n"
        "\n"
        "vec2 cabs(vec2 a)\n"
        "{\n"
        "    return vec2(sqrt(a.x * a.x + a.y * a.y), 0.0);\n"
        "}\n"
        "\n"
        "vec2 carg(vec2 a)\n"
        "{\n"
        "    return vec2(arg(a), 0.0);\n"
        "}\n"
        "\n"
        "vec2 cadd(vec2 a, vec2 b)\n"
        "{\n"
        "    return vec2(a.x + b.x, a.y + b.y);\n"
        "}\n"
        "\n"
        "vec2 csub(vec2 a, vec2 b)\n"
        "{\n"
        "    return vec2(a.x - b.x, a.y - b.y);\n"
        "}\n"
        "\n"
        "vec2 cmul(vec2 a, vec2 b)\n"
        "{\n"
        "    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);\n"
        "}\n"
        "\n"
        "vec2 cdiv(vec2 a, vec2 b)\n"
        "{\n"
        "    vec2 top = cmul(a, conj(b));\n"
        "    float bottom = norm(b);\n"
        "    return vec2(top.x / bottom, top.y / bottom);\n"
        "}\n"
        "\n"
        "vec2 cln(vec2 a)\n"
        "{\n"
        "    if ((a.x < 0) && (abs(a.y) <= NIL))\n"
        "        return vec2(log(a.x * a.x + a.y * a.y)/2, PI);\n"
        "    else\n"
        "        return vec2(log(a.x * a.x + a.y * a.y)/2, arg(a));\n"
        "}\n"
        "\n"
        "vec2 clg(vec2 a)\n"
        "{\n"
        "    vec2 ln = cln(a);\n"
        "    return vec2(ln.x/log(10.0), ln.y/log(10.0));\n"
        "}\n"
        "\n"
        "vec2 cexp(vec2 a)\n"
        "{\n"
        "    float e = exp(a.x);\n"
        "    return vec2(e * cos(a.y), e * sin(a.y));\n"
        "}\n"
        "\n"
        "vec2 cpow(vec2 a, vec2 b)\n"
        "{\n"
        "    return cexp(cmul(b, cln(a)));\n"
        "}\n"
        "\n"
        "vec2 csqrt(vec2 a)\n"
        "{\n"
        "    return cpow(a, vec2(0.5, 0));\n"
        "}\n"
        "\n"
        "vec2 csin(vec2 a)\n"
        "{\n"
        "    return vec2(sin(a.x) * cosh(a.y), cos(a.x) * sinh(a.y));\n"
        "}\n"
        "\n"
        "vec2 ccos(vec2 a)\n"
        "{\n"
        "    return vec2(cos(a.x) * cosh(a.y), -sin(a.x) * sinh(a.y));\n"
        "}\n"
        "\n"
        "vec2 ctan(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2, a.y * 2);\n"
        "    float bottom = cos(a_2.x) + cosh(a_2.y);\n"
        "    return vec2(sin(a_2.x) / bottom, sinh(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 ccot(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2, a.y * 2);\n"
        "    float bottom = cos(a_2.x) - cosh(a_2.y);\n"
        "    return vec2(-sin(a_2.x) / bottom, sinh(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 carcsin(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0, -1), cln(cadd(cmul(I, a), csqrt(csub(ONE, cmul(a, a))))));\n"
        "}\n"
        "\n"
        "vec2 carccos(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0, -1), cln(cadd(a, csqrt(csub(cmul(a, a), ONE)))));\n"
        "}\n"
        "\n"
        "vec2 carctan(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0, 0.5), csub(cln(cadd(I, a)), cln(csub(I, a))));\n"
        "}\n"
        "\n"
        "vec2 carccot(vec2 a)\n"
        "{\n"
        "    return csub(vec2(PI/2, 0), cmul(vec2(0, 0.5), csub(cln(cadd(I, a)), cln(csub(I, a)))));\n"
        "}\n"
        "\n"
        "vec2 csinh(vec2 a)\n"
        "{\n"
        "    return vec2(sinh(a.x) * cos(a.y), cosh(a.x) * sin(a.y));\n"
        "}\n"
        "\n"
        "vec2 ccosh(vec2 a)\n"
        "{\n"
        "    return vec2(cosh(a.x) * cos(a.y), sinh(a.x) * sin(a.y));\n"
        "}\n"
        "\n"
        "vec2 ctanh(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2, a.y * 2);\n"
        "    float bottom = cosh(a_2.x) + cos(a_2.y);\n"
        "    return vec2(sinh(a_2.x) / bottom, sin(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 ccoth(vec2 a)\n"
        "{\n"
        "    vec2 a_2 = vec2(a.x * 2, a.y * 2);\n"
        "    float bottom = cos(a_2.y) - cosh(a_2.x);\n"
        "    return vec2(-sinh(a_2.x) / bottom, sin(a_2.y) / bottom);\n"
        "}\n"
        "\n"
        "vec2 carcsinh(vec2 a)\n"
        "{\n"
        "    return cln(cadd(a, csqrt(cadd(cmul(a, a), ONE))));\n"
        "}\n"
        "\n"
        "vec2 carccosh(vec2 a)\n"
        "{\n"
        "    return cln(cadd(a, csqrt(csub(cmul(a, a), ONE))));\n"
        "}\n"
        "\n"
        "vec2 carctanh(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.5, 0), csub(cln(cadd(ONE, a)), cln(csub(ONE, a))));\n"
        "}\n"
        "\n"
        "vec2 carccoth(vec2 a)\n"
        "{\n"
        "    return cmul(vec2(0.5, 0), csub(cln(cadd(ONE, a)), cln(csub(ONE, a))));\n"
        "}\n"
        "\n"
        "vec3 getColor(int itrn, vec3 sumz)\n"
        "{\n"
        "    if (itrn < itrn_max)\n"
        "    {\n"
        "        itrn = itrn * 4 %% 1530;\n"
        "        if (itrn < 256)  return vec3(255, itrn, 0)        / 255 * (1.0 - float(coloring)*0.85);\n"
        "        if (itrn < 511)  return vec3(510 - itrn, 255, 0)  / 255 * (1.0 - float(coloring)*0.85);\n"
        "        if (itrn < 766)  return vec3(0, 255, itrn - 510)  / 255 * (1.0 - float(coloring)*0.85);\n"
        "        if (itrn < 1021) return vec3(0, 1020 - itrn, 255) / 255 * (1.0 - float(coloring)*0.85);\n"
        "        if (itrn < 1276) return vec3(itrn - 1020, 0, 255) / 255 * (1.0 - float(coloring)*0.85);\n"
        "        if (itrn < 1530) return vec3(255, 0, 1529 - itrn) / 255 * (1.0 - float(coloring)*0.85);\n"
        "    }\n"
        "    if (coloring) return sin(abs(abs(sumz) / itrn_max * 5.0)) * 0.45 + 0.5;\n"
        "    return vec3( 0, 0, 0 );\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    float re0 = borders.x + (borders.y - borders.x) * gl_FragCoord.x / winsizes.x;\n"
        "    float im0 = borders.w - (borders.w - borders.z) * gl_FragCoord.y / winsizes.y;\n"
        "\n"
        "    %s\n"
        "\n"
        "    vec3 sumz = vec3(0.0, 0.0, 0.0);\n"
        "    int itrn  = 0;\n"
        "    for (itrn = 0; itrn < itrn_max; ++itrn)\n"
        "    {\n"
        "        %s\n"
        "        \n"
        "        %s\n"
        "    }\n"
        "\n"
        "    vec3 col = getColor(itrn, sumz);\n"
        "    gl_FragColor = vec4(col, 1.0);\n"
        "}", str_initialization, str_calculation, str_checking
    );

    delete[] str_initialization;
    delete[] str_calculation;
    delete[] str_checking;

    shader.loadFromMemory(str_shader, sf::Shader::Fragment);

    delete[] str_shader;

    return 0;
}

char* ComplexShader::writeInitialization(int input_mode)
{
    switch (input_mode)
    {
    case Z_INPUT:
    {
        const size_t str_shader_size = 1000;
        char* str_initialization = new char[str_shader_size] {};

        sprintf(str_initialization,
            "vec2 z = vec2(re0, im0);\n"
            "vec2 pz = z;\n"
            "vec2 c = vec2(0, 0);\n"
            "if (drawing_mode == 0)\n"
            "    c = vec2(re0, im0);\n"
            "else if (drawing_mode == 1)\n"
            "    c = vec2(julia_point.x, julia_point.y);"
        );

        return str_initialization;
    }
    case XY_INPUT:
    {
        const size_t str_shader_size = 1000;
        char* str_initialization = new char[str_shader_size] {};

        sprintf(str_initialization,
            "float x = re0;\n"
            "float y = im0;\n"
            "vec2 pz = vec2(x, y);\n"
            "float cx = 0;\n"
            "float cy = 0;\n"
            "if (drawing_mode == 0)\n"
            "{\n"
            "    cx = re0;\n"
            "    cy = im0;\n"
            "}\n"
            "else if (drawing_mode == 1)\n"
            "{\n"
            "    cx = julia_point.x;\n"
            "    cy = julia_point.y;\n"
            "}"
        );

        return str_initialization;
    }
    default: return nullptr;
    }
}

char* ComplexShader::writeCalculation(const ExprTrees& expr_trees, int input_mode) const
{
    switch (input_mode)
    {
    case Z_INPUT:
    {
        const size_t str_shader_size = 1000;
        char* str_calculation = new char[str_shader_size] {};
        sprintf(str_calculation, "vec2 ppz = pz;\n"
                                 "pz = z;\n");

        sprintf(str_calculation + strlen(str_calculation), "z = ");

        int err = Tree2GLSL(expr_trees.z, input_mode, str_calculation + strlen(str_calculation));
        if (err)
        {
            delete[] str_calculation;
            return nullptr;
        }

        sprintf(str_calculation + strlen(str_calculation), ";");

        return str_calculation;
    }
    case XY_INPUT:
    {
        const size_t str_shader_size = 1000;
        char* str_calculation = new char[str_shader_size] {};
        sprintf(str_calculation, "vec2 ppz = pz;\n"
                                 "pz = vec2(x, y);\n");

        sprintf(str_calculation + strlen(str_calculation), "vec2 x1 = ");

        int err = Tree2GLSL(expr_trees.x, input_mode, str_calculation + strlen(str_calculation));
        if (err)
        {
            delete[] str_calculation;
            return nullptr;
        }

        sprintf(str_calculation + strlen(str_calculation), ";\nvec2 y1 = ");

        err = Tree2GLSL(expr_trees.y, input_mode, str_calculation + strlen(str_calculation));
        if (err)
        {
            delete[] str_calculation;
            return nullptr;
        }

        sprintf(str_calculation + strlen(str_calculation), ";\n");

        sprintf(str_calculation + strlen(str_calculation), "x = x1.x;\ny = y1.x;");

        return str_calculation;
    }
    default: return nullptr;
    }
}

char* ComplexShader::writeChecking(int input_mode)
{
    const size_t str_shader_size = 1000;
    char* str_checking = new char[str_shader_size] {};

    switch (input_mode)
    {
    case Z_INPUT: sprintf(str_checking, "if (cabs(z).x > limit) break;\n"); break;
    case XY_INPUT: sprintf(str_checking, "if (cabs(vec2(x, y)).x > limit) break;\nvec2 z = vec2(x, y);\n"); break;
    default: return nullptr;
    }

    sprintf(str_checking + strlen(str_checking), "sumz.x += dot(z - pz, pz - ppz);\n"
                                                 "sumz.y += dot(z - pz,  z - pz);\n"
                                                 "sumz.z += dot(z - ppz, z - ppz);");

    return str_checking;
}

int ComplexShader::Tree2GLSL(const Tree<CalcData>& node, int input_mode, char* str_cur) const
{
    switch (node.value().node_type)
    {
    case CalcData::FUNCTION:
    {
        sprintf(str_cur, "c%s(", node.value().word.c_str());

        int err = Tree2GLSL(node[0], input_mode, str_cur + strlen(str_cur));
        COND_RETURN(err, err);

        sprintf(str_cur + strlen(str_cur), ")");

        break;
    }
    case CalcData::OPERATOR:
    {
        switch (node.value().op_code)
        {
        case Operation::ADD: sprintf(str_cur, "cadd("); break;
        case Operation::SUB: sprintf(str_cur, "csub("); break;
        case Operation::MUL: sprintf(str_cur, "cmul("); break;
        case Operation::DIV: sprintf(str_cur, "cdiv("); break;
        case Operation::POW: sprintf(str_cur, "cpow("); break;
        default: assert(0);
        }

        if (node.branches_num() < 2)
        {
            sprintf(str_cur + strlen(str_cur), "vec2(0, 0), ");
        }
        else
        {
            int err = Tree2GLSL(node[1], input_mode, str_cur + strlen(str_cur));
            COND_RETURN(err, err);

            sprintf(str_cur + strlen(str_cur), ", ");
        }

        int err = Tree2GLSL(node[0], input_mode, str_cur + strlen(str_cur));
        COND_RETURN(err, err);

        sprintf(str_cur + strlen(str_cur), ")");

        break;
    }
    case CalcData::VARIABLE:
    {
        switch (input_mode)
        {
        case Z_INPUT:
        {
            COND_RETURN((node.value().word != "z") && (node.value().word != "c") && (node.value().word != "pi") &&
                        (node.value().word != "e") && (node.value().word != "i"), -1);

            break;
        }
        case XY_INPUT:
        {
            COND_RETURN((node.value().word != "x") && (node.value().word != "y") && (node.value().word != "cx") &&
                        (node.value().word != "cy") && (node.value().word != "pi") && (node.value().word != "e") &&
                        (node.value().word != "i"), -1);

            break;
        }
        }

        if (node.value().word == "i")
        {
            sprintf(str_cur, "I");
        }
        else
        {
            switch (input_mode)
            {
            case Z_INPUT: sprintf(str_cur, "%s", node.value().word.c_str()); break;
            case XY_INPUT: sprintf(str_cur, "vec2(%s, 0)", node.value().word.c_str()); break;
            }
        }

        break;
    }
    case CalcData::NUMBER:
    {
        switch (input_mode)
        {
        case Z_INPUT: sprintf(str_cur, "vec2(%f, %f)", real(node.value().number), imag(node.value().number)); break;
        case XY_INPUT: sprintf(str_cur, "vec2(%f,  0)", real(node.value().number)); break;
        }

        break;
    }
    default: assert(0);
    }

    return 0;
}