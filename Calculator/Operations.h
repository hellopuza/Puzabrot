/*------------------------------------------------------------------------------
    * File:        Operations.h                                                *
    * Description: Names of operators, functions and their codes.              *
    * Created:     15 may 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef OPERATIONS_H_INCLUDED
#define OPERATIONS_H_INCLUDED

#include <assert.h>
#include <string.h>

/*------------------------------------------------------------------------------
                   Operations codes                                            *
*///----------------------------------------------------------------------------


enum OperationsCodes
{
    OP_ERR      = 0x00,
    OP_ADD      = 0x01,
    OP_SUB      = 0x02,
    OP_MUL      = 0x03,
    OP_DIV      = 0x04,
    OP_POW      = 0x05,
    OP_ARCCOS   = 0x06,
    OP_ARCCOSH  = 0x07,
    OP_ARCCOT   = 0x08,
    OP_ARCCOTH  = 0x09,
    OP_ARCSIN   = 0x0A,
    OP_ARCSINH  = 0x0B,
    OP_ARCTAN   = 0x0C,
    OP_ARCTANH  = 0x0D,
    OP_COS      = 0x0E,
    OP_COSH     = 0x0F,
    OP_COT      = 0x10,
    OP_COTH     = 0x11,
    OP_EXP      = 0x12,
    OP_LG       = 0x13,
    OP_LN       = 0x14,
    OP_SIN      = 0x15,
    OP_SINH     = 0x16,
    OP_SQRT     = 0x17,
    OP_TAN      = 0x18,
    OP_TANH     = 0x19,
};

struct operation
{
    char code  = 0;
    char* word = 0;
};

static operation op_names[] =
{
    { OP_ERR      , (char*) "#ERR#"   },
    { OP_ADD      , (char*) "+"       },
    { OP_SUB      , (char*) "-"       },
    { OP_MUL      , (char*) "*"       },
    { OP_DIV      , (char*) "/"       },
    { OP_POW      , (char*) "^"       },
    { OP_ARCCOS   , (char*) "arccos"  },
    { OP_ARCCOSH  , (char*) "arccosh" },
    { OP_ARCCOT   , (char*) "arccot"  },
    { OP_ARCCOTH  , (char*) "arccoth" },
    { OP_ARCSIN   , (char*) "arcsin"  },
    { OP_ARCSINH  , (char*) "arcsinh" },
    { OP_ARCTAN   , (char*) "arctan"  },
    { OP_ARCTANH  , (char*) "arctanh" },
    { OP_COS      , (char*) "cos"     },
    { OP_COSH     , (char*) "cosh"    },
    { OP_COT      , (char*) "cot"     },
    { OP_COTH     , (char*) "coth"    },
    { OP_EXP      , (char*) "exp"     },
    { OP_LG       , (char*) "lg"      },
    { OP_LN       , (char*) "ln"      },
    { OP_SIN      , (char*) "sin"     },
    { OP_SINH     , (char*) "sinh"    },
    { OP_SQRT     , (char*) "sqrt"    },
    { OP_TAN      , (char*) "tan"     },
    { OP_TANH     , (char*) "tanh"    },
};

const int OP_NUM = sizeof(op_names) / sizeof(op_names[0]);

//------------------------------------------------------------------------------

inline int CompareOP_Names (const void* p1, const void* p2)
{
    assert(p1 != nullptr);
    assert(p2 != nullptr);
    assert(p1 != p2);

    // printf("CompareOP_Names: s1(%p): %s, s2(%p): %s\n", p1, ((struct operation*)p1)->word, p2, ((struct operation*)p2)->word);

    return strcmp(((struct operation*)p1)->word, ((struct operation*)p2)->word);
}

//------------------------------------------------------------------------------

#endif // OPERATIONS_H_INCLUDED