/*------------------------------------------------------------------------------
 * File:        Operation.h                                                *
 * Description: Names of operators, functions and their codes.              *
 * Created:     15 may 2021                                                 *
 * Author:      Artem Puzankov                                              *
 * Email:       puzankov.ao@phystech.edu                                    *
 * GitHub:      https://github.com/hellopuza                                *
 * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
 *///------------------------------------------------------------------------

#ifndef CALCULATOR_OPERATIONS_H_INCLUDED
#define CALCULATOR_OPERATIONS_H_INCLUDED

#include <string>

namespace puza {

enum OperationsCodes
{
    OP_ERR     = 0x00,
    OP_ADD     = 0x01,
    OP_SUB     = 0x02,
    OP_MUL     = 0x03,
    OP_DIV     = 0x04,
    OP_POW     = 0x05,
    OP_ABS     = 0x06,
    OP_ARCCOS  = 0x07,
    OP_ARCCOSH = 0x08,
    OP_ARCCOT  = 0x09,
    OP_ARCCOTH = 0x0A,
    OP_ARCSIN  = 0x0B,
    OP_ARCSINH = 0x0C,
    OP_ARCTAN  = 0x0D,
    OP_ARCTANH = 0x0E,
    OP_COS     = 0x0F,
    OP_COSH    = 0x10,
    OP_COT     = 0x11,
    OP_COTH    = 0x12,
    OP_EXP     = 0x13,
    OP_LG      = 0x14,
    OP_LN      = 0x15,
    OP_SIN     = 0x16,
    OP_SINH    = 0x17,
    OP_SQRT    = 0x18,
    OP_TAN     = 0x19,
    OP_TANH    = 0x1A,
};

struct Operation
{
    char        code = 0;
    std::string word = 0;

    Operation(char code_, std::string word_);
};

static Operation op_names[] = {
    { OP_ERR, "#ERR#" },       { OP_ADD, "+" },         { OP_SUB, "-" },           { OP_MUL, "*" },
    { OP_DIV, "/" },           { OP_POW, "^" },         { OP_ABS, "abs" },         { OP_ARCCOS, "arccos" },
    { OP_ARCCOSH, "arccosh" }, { OP_ARCCOT, "arccot" }, { OP_ARCCOTH, "arccoth" }, { OP_ARCSIN, "arcsin" },
    { OP_ARCSINH, "arcsinh" }, { OP_ARCTAN, "arctan" }, { OP_ARCTANH, "arctanh" }, { OP_COS, "cos" },
    { OP_COSH, "cosh" },       { OP_COT, "cot" },       { OP_COTH, "coth" },       { OP_EXP, "exp" },
    { OP_LG, "lg" },           { OP_LN, "ln" },         { OP_SIN, "sin" },         { OP_SINH, "sinh" },
    { OP_SQRT, "sqrt" },       { OP_TAN, "tan" },       { OP_TANH, "tanh" },
};

const size_t OP_NUM = sizeof(op_names) / sizeof(op_names[0]);

int compare_OpNames(const void* p1, const void* p2);

} // namespace puza

#endif // CALCULATOR_OPERATIONS_H_INCLUDED