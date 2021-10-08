/*------------------------------------------------------------------------------
    * File:        Calculator.cpp                                              *
    * Description: Functions for calculating math expressions.                 *
    * Created:     15 may 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#include <cmath>
#include <cassert>
#include "Calculator.h"

namespace puza {

constexpr std::complex<double> NIL = { 0.0, 0.0 };
constexpr std::complex<double> ONE = { 1.0, 0.0 };
constexpr std::complex<double> TWO = { 2.0, 0.0 };

#define ADD_VAR(variables)                     \
        {                                      \
            variables.push_back({ PI, "pi" }); \
            variables.push_back({ E,  "e"  }); \
            variables.push_back({ I,  "i"  }); \
        } //

std::ostream& operator << (std::ostream& os, const CalcData& data)
{
    switch (data.node_type)
    {
    case NODE_FUNCTION: return os << data.word;   break;
    case NODE_OPERATOR: return os << data.word;   break;
    case NODE_VARIABLE: return os << data.word;   break;
    case NODE_NUMBER:   return os << data.number; break;
    }

    return os;
}

//------------------------------------------------------------------------------

Calculator::Calculator ()
{
    ADD_VAR(variables);
}

Calculator::~Calculator () {}

void Calculator::clear ()
{
    variables.clear();
    ADD_VAR(variables);
}

//------------------------------------------------------------------------------

int Calculator::Calculate (Tree<CalcData>& node)
{
    switch (node.data.node_type)
    {
    case NODE_FUNCTION:
    {
        int err = Calculate(node.branches[0]);
        if (err) return err;

        std::complex<double> number = node.branches[0].data.number;

        switch (node.data.op_code)
        {
        case OP_ABS:        number = abs(number);           break;
        case OP_ARCCOS:     number = acos(number);          break;
        case OP_ARCCOSH:    number = acosh(number);         break;
        case OP_ARCCOT:     number = PI/TWO - atan(number); break;
        case OP_ARCCOTH:    number = atanh(ONE / number);   break;
        case OP_ARCSIN:     number = asin(number);          break;
        case OP_ARCSINH:    number = asinh(number);         break;
        case OP_ARCTAN:     number = atan(number);          break;
        case OP_ARCTANH:    number = atanh(number);         break;
        case OP_COS:        number = cos(number);           break;
        case OP_COSH:       number = cosh(number);          break;
        case OP_COT:        number = ONE / tan(number);     break;
        case OP_COTH:       number = ONE / tanh(number);    break;
        case OP_EXP:        number = exp(number);           break;
        case OP_LG:         number = log10(number);         break;
        case OP_LN:         number = log(number);           break;
        case OP_SIN:        number = sin(number);           break;
        case OP_SINH:       number = sinh(number);          break;
        case OP_SQRT:       number = sqrt(number);          break;
        case OP_TAN:        number = tan(number);           break;
        case OP_TANH:       number = tanh(number);          break;
        default: assert(0);
        }

        node.data = { number, node.data.word, node.data.op_code, node.data.node_type };
        break;
    }
    case NODE_OPERATOR:
    {
        std::complex<double> right_num { NIL };
        std::complex<double> left_num  { NIL };
        std::complex<double> number    { NIL };

        if (node.branches.size() == 2)
        {
            int err = Calculate(node.branches[1]);
            if (err) return err;

            left_num = node.branches[1].data.number;
        }
        else left_num = NIL;

        int err = Calculate(node.branches[0]);
        if (err) return err;

        right_num = node.branches[0].data.number;

        switch (node.data.op_code)
        {
        case OP_ADD:  number = left_num + right_num;     break;
        case OP_SUB:  number = left_num - right_num;     break;
        case OP_MUL:  number = left_num * right_num;     break;
        case OP_DIV:  number = left_num / right_num;     break;
        case OP_POW:  number = pow(left_num, right_num); break;
        default: assert(0);
        }

        node.data = { number, node.data.word, node.data.op_code, node.data.node_type };
        break;
    }
    case NODE_VARIABLE:
    {
        int index = -1;
        for (int i = 0; i < variables.size(); ++i)
            if (variables[i].name == node.data.word)
            {
                index = i;
                break;
            }
        
        if (index == -1) return CALC_UNIDENTIFIED_VARIABLE;
        
        node.data = { variables[index].value, node.data.word, node.data.op_code, node.data.node_type };
        break;
    }
    case NODE_NUMBER:
        break;

    default: assert(0);
    }

    return CALC_OK;
}

//------------------------------------------------------------------------------

Expression::Expression  () {}
Expression::~Expression () {}

Expression::Expression (std::string string) : str(string) {}

int Expression::getTree (Tree<CalcData>& tree)
{
    size_t to_write = 0;
    for (char symb : str)
    {
        if (! isspace(symb))
        {
            str[to_write++] = symb;
        }
    }
    str.erase(to_write);

    pos = 0;

    tree.clear();

    int err = pass_Plus_Minus(tree);
    if (err) return err;

    return CALC_OK;
}

//------------------------------------------------------------------------------

int Expression::pass_Plus_Minus (Tree<CalcData>& node)
{
    if (str[pos] == '-')
    {   
        pos++;

        Tree<CalcData> right;
        int err = pass_Mul_Div(right);
        if (err) return err;

        node.clear();
        node.branches.push_back(right);

        node.data = { NIL, op_names[OP_SUB].word, op_names[OP_SUB].code, NODE_OPERATOR };
    }
    else
    {
        int err = pass_Mul_Div(node);
        if (err) return err;
    }
    
    while ( (str[pos] == '+') ||
            (str[pos] == '-')   )
    {
        char op = (str[pos] == '-') ? OP_SUB : OP_ADD;
        pos++;

        Tree<CalcData> left = node;
        Tree<CalcData> right;

        int err = pass_Mul_Div(right);
        if (err) return err;

        node.clear();
        node.branches.push_back(right);
        node.branches.push_back(left);

        node.data = { NIL, op_names[op].word, op_names[op].code, NODE_OPERATOR };
    }

    if ( (str[pos] != '+') && (str[pos] != '-') && (str[pos] != '*') && (str[pos] != '/') &&
         (str[pos] != '^') && (str[pos] != '(') && (str[pos] != ')') && (str[pos] != '\0') )
        return CALC_SYNTAX_ERROR;

    return CALC_OK;
}

//------------------------------------------------------------------------------

int Expression::pass_Mul_Div (Tree<CalcData>& node)
{
    int err = pass_Power(node);
    if (err) return err;

    while ( (str[pos] == '*') ||
            (str[pos] == '/') )
    {
        char op = (str[pos] == '*') ? OP_MUL : OP_DIV;
        pos++;

        Tree<CalcData> left = node;
        Tree<CalcData> right;

        err = pass_Power(right);
        if (err) return err;

        node.clear();
        node.branches.push_back(right);
        node.branches.push_back(left);

        node.data = { NIL, op_names[op].word, op_names[op].code, NODE_OPERATOR };
    }

    return CALC_OK;
}

//------------------------------------------------------------------------------

int Expression::pass_Power (Tree<CalcData>& node)
{
    int err = pass_Brackets(node);
    if (err) return err;

    while (str[pos] == '^')
    {
        pos++;

        Tree<CalcData> left = node;
        Tree<CalcData> right;

        err = pass_Power(right);
        if (err) return err;

        node.clear();
        node.branches.push_back(right);
        node.branches.push_back(left);

        node.data = { NIL, op_names[OP_POW].word, op_names[OP_POW].code, NODE_OPERATOR };
    }
    
    return CALC_OK;
}

//------------------------------------------------------------------------------

int Expression::pass_Brackets (Tree<CalcData>& node)
{
    if (str[pos] == '(')
    {
        pos++;

        int err = pass_Plus_Minus(node);
        if (err) return err;

        if (str[pos] != ')') return CALC_SYNTAX_NO_CLOSE_BRACKET;
        pos++;

        return CALC_OK;
    }
    else return pass_Function(node);
}

//------------------------------------------------------------------------------

int Expression::pass_Function (Tree<CalcData>& node)
{
    if (isdigit(str[pos])) return pass_Number(node);

    if (! isalpha(str[pos])) return CALC_SYNTAX_ERROR;

    std::string word;
    while (isalpha(str[pos]) || isdigit(str[pos]))
    {
        word.push_back(str[pos++]);
    }

    if (str[pos] == '(')
    {
        int code = findFunc(word);
        if (code == OP_ERR) return CALC_SYNTAX_UNIDENTIFIED_FUNCTION;

        Tree<CalcData> right;
        int err = pass_Brackets(right);
        if (err) return err;

        node.clear();
        node.branches.push_back(right);

        node.data = { NIL, op_names[code].word, op_names[code].code, NODE_FUNCTION };
    }
    else node.data = { NIL, word, OP_ERR, NODE_VARIABLE };

    return CALC_OK;
}

//------------------------------------------------------------------------------

int Expression::pass_Number (Tree<CalcData>& node)
{
    size_t end_pos = 0;
    double value = std::stod(str.substr(pos), &end_pos);
    pos += end_pos;

    if (str[pos] == 'i')
    {
        pos++;
        node.data = { {0, value}, {}, OP_ERR, NODE_NUMBER };
    }
    else node.data = { {value, 0}, {}, OP_ERR, NODE_NUMBER };

    return CALC_OK;
}

//------------------------------------------------------------------------------

char Expression::findFunc (std::string word)
{
    operation func_key = { 0, word };

    operation* p_func_struct = (operation*)bsearch(&func_key, op_names, OP_NUM, sizeof(op_names[0]), compare_OpNames);

    if (p_func_struct != nullptr) return p_func_struct->code;

    return OP_ERR;
}

} // namespace puza
