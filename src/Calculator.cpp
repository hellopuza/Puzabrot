#include "Calculator/Calculator.h"

#include <cassert>
#include <cmath>

namespace puza {

constexpr std::complex<double> kNil = { 0.0, 0.0 };
constexpr std::complex<double> kOne = { 1.0, 0.0 };
constexpr std::complex<double> kTwo = { 2.0, 0.0 };

#define ADD_VAR(variables)                               \
    {                                                    \
        (variables).emplace_back(Variable { PI, "pi" }); \
        (variables).emplace_back(Variable { E, "e" });   \
        (variables).emplace_back(Variable { I, "i" });   \
    } //

std::ostream& operator<<(std::ostream& os, const CalcData& data)
{
    switch (data.node_type)
    {
    case CalcData::FUNCTION:
    case CalcData::OPERATOR:
    case CalcData::VARIABLE: return os << data.word;   break;
    case CalcData::NUMBER:   return os << data.number; break;
    }

    return os;
}

Variable::Variable(std::complex<double> value_, std::string name_) : value(value_), name(std::move(name_)) {}

CalcData::CalcData(std::complex<double> number_, std::string word_, char op_code_, char node_type_) :
    number(number_), word(std::move(word_)), op_code(op_code_), node_type(node_type_)
{}

Calculator::Calculator()
{
    ADD_VAR(variables);
}

void Calculator::clear()
{
    variables.clear();
    ADD_VAR(variables);
}

int Calculator::Calculate(Tree<CalcData>& node)
{
    switch (node.data.node_type)
    {
    case CalcData::FUNCTION:
    {
        int err = Calculate(node.branches[0]);
        if (err)
            return err;

        std::complex<double> number = node.branches[0].data.number;

        switch (node.data.op_code)
        {
        case Operation::ABS:     number = abs(number);              break;
        case Operation::ARCCOS:  number = acos(number);             break;
        case Operation::ARCCOSH: number = acosh(number);            break;
        case Operation::ARCCOT:  number = PI / kTwo - atan(number); break;
        case Operation::ARCCOTH: number = atanh(kOne / number);     break;
        case Operation::ARCSIN:  number = asin(number);             break;
        case Operation::ARCSINH: number = asinh(number);            break;
        case Operation::ARCTAN:  number = atan(number);             break;
        case Operation::ARCTANH: number = atanh(number);            break;
        case Operation::COS:     number = cos(number);              break;
        case Operation::COSH:    number = cosh(number);             break;
        case Operation::COT:     number = kOne / tan(number);       break;
        case Operation::COTH:    number = kOne / tanh(number);      break;
        case Operation::EXP:     number = exp(number);              break;
        case Operation::LG:      number = log10(number);            break;
        case Operation::LN:      number = log(number);              break;
        case Operation::SIN:     number = sin(number);              break;
        case Operation::SINH:    number = sinh(number);             break;
        case Operation::SQRT:    number = sqrt(number);             break;
        case Operation::TAN:     number = tan(number);              break;
        case Operation::TANH:    number = tanh(number);             break;
        default: assert(0);
        }

        node.data = CalcData { number, node.data.word, node.data.op_code, node.data.node_type };
        break;
    }
    case CalcData::OPERATOR:
    {
        std::complex<double> right_num { kNil };
        std::complex<double> left_num { kNil };
        std::complex<double> number { kNil };

        if (node.branches.size() == 2)
        {
            int err = Calculate(node.branches[1]);
            if (err)
                return err;

            left_num = node.branches[1].data.number;
        }
        else
            left_num = kNil;

        int err = Calculate(node.branches[0]);
        if (err)
            return err;

        right_num = node.branches[0].data.number;

        switch (node.data.op_code)
        {
        case Operation::ADD: number = left_num + right_num;     break;
        case Operation::SUB: number = left_num - right_num;     break;
        case Operation::MUL: number = left_num * right_num;     break;
        case Operation::DIV: number = left_num / right_num;     break;
        case Operation::POW: number = pow(left_num, right_num); break;
        default: assert(0);
        }

        node.data = CalcData { number, node.data.word, node.data.op_code, node.data.node_type };
        break;
    }
    case CalcData::VARIABLE:
    {
        auto index = static_cast<size_t>(-1);
        for (size_t i = 0; i < variables.size(); ++i)
            if (variables[i].name == node.data.word)
            {
                index = i;
                break;
            }

        if (index == static_cast<size_t>(-1))
            return CALC_UNIDENTIFIED_VARIABLE;

        node.data = CalcData { variables[index].value, node.data.word, node.data.op_code, node.data.node_type };
        break;
    }
    case CalcData::NUMBER: break;

    default: assert(0);
    }

    return CALC_OK;
}

Expression::Expression(std::string string) : str(std::move(string)) {}

int Expression::getTree(Tree<CalcData>& tree)
{
    size_t to_write = 0;
    for (char symb : str)
    {
        if (!isspace(symb))
        {
            str[to_write++] = symb;
        }
    }
    str.erase(to_write);

    pos = 0;

    tree.clear();

    int err = pass_Plus_Minus(tree);
    if (err)
        return err;

    return CALC_OK;
}

int Expression::pass_Plus_Minus(Tree<CalcData>& node)
{
    if (str[pos] == '-')
    {
        pos++;

        Tree<CalcData> right;
        int            err = pass_Mul_Div(right);
        if (err)
            return err;

        node.clear();
        node.branches.push_back(right);

        node.data = CalcData { kNil, op_names[Operation::SUB].word, op_names[Operation::SUB].code, CalcData::OPERATOR };
    }
    else
    {
        int err = pass_Mul_Div(node);
        if (err)
            return err;
    }

    while ((str[pos] == '+') || (str[pos] == '-'))
    {
        size_t op = (str[pos] == '-') ? Operation::SUB : Operation::ADD;
        pos++;

        Tree<CalcData> left = node;
        Tree<CalcData> right;

        int err = pass_Mul_Div(right);
        if (err)
            return err;

        node.clear();
        node.branches.push_back(right);
        node.branches.push_back(left);

        node.data = CalcData { kNil, op_names[op].word, op_names[op].code, CalcData::OPERATOR };
    }

    if ((str[pos] != '+') && (str[pos] != '-') && (str[pos] != '*') && (str[pos] != '/') && (str[pos] != '^') &&
        (str[pos] != '(') && (str[pos] != ')') && (str[pos] != '\0'))
        return CALC_SYNTAX_ERROR;

    return CALC_OK;
}

int Expression::pass_Mul_Div(Tree<CalcData>& node)
{
    int err = pass_Power(node);
    if (err)
        return err;

    while ((str[pos] == '*') || (str[pos] == '/'))
    {
        size_t op = (str[pos] == '*') ? Operation::MUL : Operation::DIV;
        pos++;

        Tree<CalcData> left = node;
        Tree<CalcData> right;

        err = pass_Power(right);
        if (err)
            return err;

        node.clear();
        node.branches.push_back(right);
        node.branches.push_back(left);

        node.data = CalcData { kNil, op_names[op].word, op_names[op].code, CalcData::OPERATOR };
    }

    return CALC_OK;
}

int Expression::pass_Power(Tree<CalcData>& node)
{
    int err = pass_Brackets(node);
    if (err)
        return err;

    while (str[pos] == '^')
    {
        pos++;

        Tree<CalcData> left = node;
        Tree<CalcData> right;

        err = pass_Power(right);
        if (err)
            return err;

        node.clear();
        node.branches.push_back(right);
        node.branches.push_back(left);

        node.data = CalcData { kNil, op_names[Operation::POW].word, op_names[Operation::POW].code, CalcData::OPERATOR };
    }

    return CALC_OK;
}

int Expression::pass_Brackets(Tree<CalcData>& node)
{
    if (str[pos] == '(')
    {
        pos++;

        int err = pass_Plus_Minus(node);
        if (err)
            return err;

        if (str[pos] != ')')
            return CALC_SYNTAX_NO_CLOSE_BRACKET;
        pos++;

        return CALC_OK;
    }
    return pass_Function(node);
}

int Expression::pass_Function(Tree<CalcData>& node)
{
    if (isdigit(str[pos]))
        return pass_Number(node);

    if (!isalpha(str[pos]))
        return CALC_SYNTAX_ERROR;

    std::string word;
    while (isalpha(str[pos]) || isdigit(str[pos])) { word.push_back(str[pos++]); }

    if (str[pos] == '(')
    {
        int code = findFunc(word);
        if (code == Operation::ERR)
            return CALC_SYNTAX_UNIDENTIFIED_FUNCTION;

        Tree<CalcData> right;
        int err = pass_Brackets(right);
        if (err)
            return err;

        node.clear();
        node.branches.push_back(right);

        node.data = CalcData { kNil, op_names[code].word, op_names[code].code, CalcData::FUNCTION };
    }
    else
        node.data = CalcData { kNil, word, Operation::ERR, CalcData::VARIABLE };

    return CALC_OK;
}

int Expression::pass_Number(Tree<CalcData>& node)
{
    size_t end_pos = 0;
    double value   = std::stod(str.substr(pos), &end_pos);
    pos += end_pos;

    if (str[pos] == 'i')
    {
        pos++;
        node.data = CalcData { { 0, value }, {}, Operation::ERR, CalcData::NUMBER };
    }
    else
        node.data = CalcData { { value, 0 }, {}, Operation::ERR, CalcData::NUMBER };

    return CALC_OK;
}

char Expression::findFunc(std::string word)
{
    Operation func_key { 0, std::move(word) };

    auto* p_func_struct =
        reinterpret_cast<struct Operation*>(bsearch(&func_key, op_names, OP_NUM, sizeof(op_names[0]), compare_OpNames));

    if (p_func_struct != nullptr)
        return p_func_struct->code;

    return Operation::ERR;
}

} // namespace puza
