#ifndef CALCULATOR_OPERATION_H
#define CALCULATOR_OPERATION_H

#include <string>

struct Operation
{
    uint8_t code = 0;
    std::string word = 0;

    enum OperationsCodes
    {
        ERR     = 0x00,
        ADD     = 0x01,
        SUB     = 0x02,
        MUL     = 0x03,
        DIV     = 0x04,
        POW     = 0x05,
        ABS     = 0x06,
        ARCCOS  = 0x07,
        ARCCOSH = 0x08,
        ARCCOT  = 0x09,
        ARCCOTH = 0x0A,
        ARCSIN  = 0x0B,
        ARCSINH = 0x0C,
        ARCTAN  = 0x0D,
        ARCTANH = 0x0E,
        ARG     = 0x0F,
        COS     = 0x10,
        COSH    = 0x11,
        COT     = 0x12,
        COTH    = 0x13,
        EXP     = 0x14,
        LG      = 0x15,
        LN      = 0x16,
        SIN     = 0x17,
        SINH    = 0x18,
        SQRT    = 0x19,
        TAN     = 0x1A,
        TANH    = 0x1B,
    };

    Operation(uint8_t code_, std::string word_);
};

static Operation op_names[] = {
    { Operation::ERR,     "#ERR#"   },
    { Operation::ADD,     "+"       },
    { Operation::SUB,     "-"       },
    { Operation::MUL,     "*"       },
    { Operation::DIV,     "/"       },
    { Operation::POW,     "^"       },
    { Operation::ABS,     "abs"     },
    { Operation::ARCCOS,  "arccos"  },
    { Operation::ARCCOSH, "arccosh" },
    { Operation::ARCCOT,  "arccot"  },
    { Operation::ARCCOTH, "arccoth" },
    { Operation::ARCSIN,  "arcsin"  },
    { Operation::ARCSINH, "arcsinh" },
    { Operation::ARCTAN,  "arctan"  },
    { Operation::ARCTANH, "arctanh" },
    { Operation::ARG,     "arg"     },
    { Operation::COS,     "cos"     },
    { Operation::COSH,    "cosh"    },
    { Operation::COT,     "cot"     },
    { Operation::COTH,    "coth"    },
    { Operation::EXP,     "exp"     },
    { Operation::LG,      "lg"      },
    { Operation::LN,      "ln"      },
    { Operation::SIN,     "sin"     },
    { Operation::SINH,    "sinh"    },
    { Operation::SQRT,    "sqrt"    },
    { Operation::TAN,     "tan"     },
    { Operation::TANH,    "tanh"    },
};

const size_t OP_NUM = sizeof(op_names) / sizeof(op_names[0]);

int compare_OpNames(const void* p1, const void* p2);

#endif // CALCULATOR_OPERATION_H