/*------------------------------------------------------------------------------
    * File:        Calculator.h                                                *
    * Description: Declaration of functions and data types used for            *
    *              calculating math expressions.                               *
    * Created:     15 may 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef CALCULATOR_H_INCLUDED
#define CALCULATOR_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS


#if defined (__GNUC__) || defined (__clang__) || defined (__clang_major__)
    #define __FUNC_NAME__   __PRETTY_FUNCTION__

#elif defined (_MSC_VER)
    #define __FUNC_NAME__   __FUNCSIG__

#else
    #define __FUNC_NAME__   __FUNCTION__

#endif


#include "../Tree/Tree.h"
#include "Operations.h"
#include <complex>

namespace puza {

const std::complex<double> PI = {atan(1.0) * 4.0, 0.0};
const std::complex<double> E  = {exp(1.0),        0.0};
const std::complex<double> I  = {0.0,             1.0};

#define ADD_VAR(variables)                     \
        {                                      \
            variables.push_back({ PI, "pi" }); \
            variables.push_back({ E,  "e"  }); \
            variables.push_back({ I,  "i"  }); \
        } //

enum CalculatorErrors
{
    CALC_NOT_OK = -1                                                       ,
    CALC_OK = 0                                                            ,
    CALC_DESTRUCTED                                                        ,
    CALC_NULL_INPUT_CALCULATOR_PTR                                         ,
    CALC_SYNTAX_ERROR                                                      ,
    CALC_SYNTAX_NO_CLOSE_BRACKET                                           ,
    CALC_SYNTAX_UNIDENTIFIED_FUNCTION                                      ,
    CALC_TREE_FUNC_WRONG_ARGUMENT                                          ,
    CALC_TREE_NUM_WRONG_ARGUMENT                                           ,
    CALC_TREE_OPER_WRONG_ARGUMENTS                                         ,
    CALC_TREE_VAR_WRONG_ARGUMENT                                           ,
    CALC_UNIDENTIFIED_VARIABLE                                             ,
    CALC_WRONG_VARIABLE                                                    ,
};

char const * const calc_errstr[] =
{
    "ERROR"                                                                ,
    "OK"                                                                   ,
    "Calculator has already destructed"                                    ,
    "The input value of the calculator pointer turned out to be zero"      ,
    "Syntax error"                                                         ,
    "Close bracket \')\' required here"                                    ,
    "Unidentified function"                                                ,
    "Function node must have one children on the right branch"             ,
    "Number node must not have any children"                               ,
    "Operator node must have two children"                                 ,
    "Variable node must not have any children"                             ,
    "I do not solve equations"                                             ,
    "Wrong variable detected"                                              ,
};


enum NODE_TYPE
{
    NODE_FUNCTION = 1,
    NODE_OPERATOR = 2,
    NODE_VARIABLE = 3,
    NODE_NUMBER   = 4,
};

struct CalcData
{
    std::complex<double> number {0.0, 0.0};

    std::string word;
    char  op_code   = 0;
    char  node_type = 0;

    friend std::ostream& operator << (std::ostream& os, const CalcData& data);
};

struct Variable
{
    std::complex<double> value {0.0, 0.0};
    std::string name;
};

struct Expression
{
    std::string str;

    Expression  ();
    Expression  (std::string string);
    ~Expression ();

    int getTree (Tree<CalcData>& tree);

private:

    size_t pos = 0;

    int pass_Plus_Minus (Tree<CalcData>& node);
    int pass_Mul_Div    (Tree<CalcData>& node);
    int pass_Power      (Tree<CalcData>& node);
    int pass_Brackets   (Tree<CalcData>& node);
    int pass_Function   (Tree<CalcData>& node);
    int pass_Number     (Tree<CalcData>& node);

    char findFunc (std::string word);
};

struct Calculator
{
    std::vector<Variable> variables;

    Calculator  ();
    ~Calculator ();

    int  Calculate (Tree<CalcData>& node);
    void clear ();
};

} // namespace puza

#endif // CALCULATOR_H_INCLUDED
