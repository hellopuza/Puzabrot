#ifndef CALCULATOR_OPERATIONS_H_INCLUDED
#define CALCULATOR_OPERATIONS_H_INCLUDED

#include <string>

namespace puza {

struct Operation
{
    char        code = 0;
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
        COS     = 0x0F,
        COSH    = 0x10,
        COT     = 0x11,
        COTH    = 0x12,
        EXP     = 0x13,
        LG      = 0x14,
        LN      = 0x15,
        SIN     = 0x16,
        SINH    = 0x17,
        SQRT    = 0x18,
        TAN     = 0x19,
        TANH    = 0x1A,
    };

    Operation(char code_, std::string word_);
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

} // namespace puza

#endif // CALCULATOR_OPERATIONS_H_INCLUDED