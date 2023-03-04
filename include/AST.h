#ifndef AST_H
#define AST_H

#include "Tree.h"

#include <cmath>
#include <complex>
#include <memory>
#include <iostream>
#include <string>
#include <unordered_map>

namespace ast {

template<typename T>
class AST;

template<typename T>
using Variable = std::pair<const std::string, T>;

template<typename T>
using Variables = std::unordered_map<std::string, T>;

template<typename T>
struct ASTNode
{
    enum class Type
    {
        OPERATION,
        FUNCTION,
        VARIABLE,
        NUMBER,
    };

    ASTNode() = default;
    virtual ~ASTNode() = default;

    virtual Type NodeType() const = 0;
    virtual T calc(AST<T>* node, const Variables<T>& vars) = 0;
    virtual void diff(AST<T>* node, const std::string& var_name) = 0;
    virtual void simplify(AST<T>* node) = 0;
};

template<typename T>
struct OperationNode : public ASTNode<T>
{
    enum class Type
    {
        ERROR,
        ADD,
        SUB,
        MUL,
        DIV,
        POW,
    };

    explicit OperationNode(Type op_type);

    ASTNode<T>::Type NodeType() const override;
    T calc(AST<T>* node, const Variables<T>& vars) override;
    void diff(AST<T>* node, const std::string& var_name) override;
    void simplify(AST<T>* node) override;

    Type type;
};

template<typename T>
struct FunctionNode : public ASTNode<T>
{
    enum class Type
    {
        ERROR,
        ABS,
        ARCCOS,
        ARCCOSH,
        ARCCOT,
        ARCCOTH,
        ARCSIN,
        ARCSINH,
        ARCTAN,
        ARCTANH,
        ARG,
        COS,
        COSH,
        COT,
        COTH,
        EXP,
        LOG,
        LOG10,
        SIN,
        SINH,
        SQRT,
        TAN,
        TANH,
    };

    explicit FunctionNode(Type func_type);

    ASTNode<T>::Type NodeType() const override;
    T calc(AST<T>* node, const Variables<T>& vars) override;
    void diff(AST<T>* node, const std::string& var_name) override;
    void simplify(AST<T>* node) override;

    Type type;
};

template<typename T>
struct VariableNode : public ASTNode<T>
{
    explicit VariableNode(const std::string& var_name);

    ASTNode<T>::Type NodeType() const override;
    T calc(AST<T>* node, const Variables<T>& vars) override;
    void diff(AST<T>* node, const std::string& var_name) override;
    void simplify(AST<T>* node) override;

    std::string name;
};

template<typename T>
struct NumberNode : public ASTNode<T>
{
    explicit NumberNode(const T& number_value);

    ASTNode<T>::Type NodeType() const override;
    T calc(AST<T>* node, const Variables<T>& vars) override;
    void diff(AST<T>* node, const std::string& var_name) override;
    void simplify(AST<T>* node) override;

    T number = {};
};

template<typename T = float>
class AST : public Tree<std::shared_ptr<ASTNode<T>>>
{
public:
    enum class Error
    {
        OK,
        SYNTAX_ERROR,
        NO_CLOSE_BRACKET,
        UNIDENTIFIED_OPERATION,
        UNIDENTIFIED_FUNCTION,
        UNIDENTIFIED_VARIABLE,
    };

    AST() = default;
    AST(std::string expression, Error* err = nullptr);
    AST(const char* expression, Error* err = nullptr);
    explicit AST(const T& number);

    T operator()(std::initializer_list<Variable<T>> list);

    AST operator + (const AST& a) const;
    AST operator - (const AST& a) const;
    AST operator * (const AST& a) const;
    AST operator / (const AST& a) const;

    AST operator - () const;
    AST operator + () const;

    AST& operator += (const AST& a);
    AST& operator -= (const AST& a);
    AST& operator *= (const AST& a);
    AST& operator /= (const AST& a);

    AST operator + (const T& number) const;
    AST operator - (const T& number) const;
    AST operator * (const T& number) const;
    AST operator / (const T& number) const;

    AST& operator += (const T& number);
    AST& operator -= (const T& number);
    AST& operator *= (const T& number);
    AST& operator /= (const T& number);
    AST& operator = (const T& number);

    AST& differentiate(const std::string& var_name);
    AST derivative(const std::string& var_name) const;

    AST& simplify();
    AST simplified() const;

private:
    Error parsePlusMinus(const std::string& str, size_t* pos);
    Error parseMulDiv(const std::string& str, size_t* pos);
    Error parsePower(const std::string& str, size_t* pos);
    Error parseBrackets(const std::string& str, size_t* pos);
    Error parseFunction(const std::string& str, size_t* pos);
    Error parseNumber(const std::string& str, size_t* pos);

public:
    static OperationNode<T>::Type OperationType(const std::string& word);
    static const char* OperationName(OperationNode<T>::Type op_type);
    static FunctionNode<T>::Type FunctionType(const std::string& word);
    static const char* FunctionName(FunctionNode<T>::Type func_type);
};

#define LBRANCH(node_ptr)    static_cast<AST<T>*>(&((*node_ptr)[0]))
#define RBRANCH(node_ptr)    static_cast<AST<T>*>(&((*node_ptr)[1]))
#define COPY_LEFT(node_ptr)  (+(*LBRANCH(node_ptr)))
#define COPY_RIGHT(node_ptr) (+(*RBRANCH(node_ptr)))
#define NODE_TYPE(node_ptr)  (node_ptr)->value().get()->NodeType()
#define IS_OP(node_ptr)      (NODE_TYPE(node_ptr) == ASTNode<T>::Type::OPERATION)
#define IS_FUNC(node_ptr)    (NODE_TYPE(node_ptr) == ASTNode<T>::Type::FUNCTION)
#define IS_VAR(node_ptr)     (NODE_TYPE(node_ptr) == ASTNode<T>::Type::VARIABLE)
#define IS_NUM(node_ptr)     (NODE_TYPE(node_ptr) == ASTNode<T>::Type::NUMBER)
#define TO_OP(node_ptr)      static_cast<OperationNode<T>*>((node_ptr)->value().get())
#define TO_FUNC(node_ptr)    static_cast<FunctionNode<T>*>((node_ptr)->value().get())
#define TO_VAR(node_ptr)     static_cast<VariableNode<T>*>((node_ptr)->value().get())
#define TO_NUM(node_ptr)     static_cast<NumberNode<T>*>((node_ptr)->value().get())
#define CALC(node_ptr)       (node_ptr)->value().get()->calc((node_ptr), vars)
#define DIFF(node_ptr)       (node_ptr)->value().get()->diff((node_ptr), var_name)
#define SIMPLIFY(node_ptr)   (node_ptr)->value().get()->simplify(node_ptr)

template<typename T>
OperationNode<T>::OperationNode(Type op_type) : type(op_type) {}

template<typename T>
ASTNode<T>::Type OperationNode<T>::NodeType() const
{
    return ASTNode<T>::Type::OPERATION;
}

template<typename T>
T OperationNode<T>::calc(AST<T>* node, const Variables<T>& vars)
{
    T right_num = {};
    T left_num = {};
    T number = {};

    if (node->branches_num() == 2)
    {
        left_num = CALC(LBRANCH(node));
        right_num = CALC(RBRANCH(node));
    }
    else
    {
        right_num = CALC(LBRANCH(node));
    }

    switch (this->type)
    {
    case OperationNode<T>::Type::ADD: number = left_num + right_num;          break;
    case OperationNode<T>::Type::SUB: number = left_num - right_num;          break;
    case OperationNode<T>::Type::MUL: number = left_num * right_num;          break;
    case OperationNode<T>::Type::DIV: number = left_num / right_num;          break;
    case OperationNode<T>::Type::POW: number = std::pow(left_num, right_num); break;
    default: break;
    }

    return number;
}

template<typename T>
void OperationNode<T>::diff(AST<T>* node, const std::string& var_name)
{
    if (node->branches_num() == 1)
    {
        DIFF(LBRANCH(node));
        return;
    }

    switch (this->type)
    {
    case OperationNode<T>::Type::ADD:
    case OperationNode<T>::Type::SUB:
    {
        DIFF(LBRANCH(node));
        DIFF(RBRANCH(node));
        break;
    }
    default:
    {
        auto L = COPY_LEFT(node);
        auto R = COPY_RIGHT(node);
        auto Ld = COPY_LEFT(node);
        auto Rd = COPY_RIGHT(node);
        DIFF(&Ld);
        DIFF(&Rd);

        switch (this->type)
        {
            case OperationNode<T>::Type::MUL: *node = Ld * R + Rd * L;                        break;
            case OperationNode<T>::Type::DIV: *node = (Ld * R - Rd * L) / pow(R, T(2));       break;
            case OperationNode<T>::Type::POW: *node = pow(L, R) * (Rd * log(L) + R * L / Ld); break;
            default: break;
        }
        break;
    }
    }
}

template<typename T>
void OperationNode<T>::simplify(AST<T>* node)
{
    if ((node->branches_num() == 1) && IS_NUM(LBRANCH(node)))
    {
        *node = COPY_LEFT(node);
        TO_NUM(node)->number *= T(-1);
        return;
    }
    if (node->branches_num() == 1)
    {
        return;
    }

    switch (this->type)
    {
    case OperationNode<T>::Type::ADD:
    {
        if (IS_NUM(LBRANCH(node)) && IS_NUM(RBRANCH(node)))
        {
            *node = TO_NUM(LBRANCH(node))->number + TO_NUM(RBRANCH(node))->number;
            break;
        }
        if (IS_NUM(LBRANCH(node)) && (TO_NUM(LBRANCH(node))->number == T()))
        {
            *node = COPY_RIGHT(node);
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T()))
        {
            *node = COPY_LEFT(node);
            break;
        }
        if (COPY_LEFT(node) == COPY_RIGHT(node))
        {
            *node = T(2) * COPY_LEFT(node);
            break;
        }
        break;
    }
    case OperationNode<T>::Type::SUB:
    {
        if (IS_NUM(LBRANCH(node)) && IS_NUM(RBRANCH(node)))
        {
            *node = TO_NUM(LBRANCH(node))->number - TO_NUM(RBRANCH(node))->number;
            break;
        }
        if (IS_NUM(LBRANCH(node)) && (TO_NUM(LBRANCH(node))->number == T()))
        {
            *node = -COPY_RIGHT(node);
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T()))
        {
            *node = COPY_LEFT(node);
            break;
        }
        if (COPY_LEFT(node) == COPY_RIGHT(node))
        {
            *node = T();
            break;
        }
        break;
    }
    case OperationNode<T>::Type::MUL:
    {
        if (IS_NUM(LBRANCH(node)) && IS_NUM(RBRANCH(node)))
        {
            *node = TO_NUM(LBRANCH(node))->number * TO_NUM(RBRANCH(node))->number;
            break;
        }
        if (IS_NUM(LBRANCH(node)) && (TO_NUM(LBRANCH(node))->number == T()))
        {
            *node = T();
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T()))
        {
            *node = T();
            break;
        }
        if (IS_NUM(LBRANCH(node)) && (TO_NUM(LBRANCH(node))->number == T(1)))
        {
            *node = COPY_RIGHT(node);
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T(1)))
        {
            *node = COPY_LEFT(node);
            break;
        }
        if (IS_NUM(LBRANCH(node)) && (TO_NUM(LBRANCH(node))->number == T(-1)))
        {
            *node = -COPY_RIGHT(node);
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T(-1)))
        {
            *node = -COPY_LEFT(node);
            break;
        }
        if (COPY_LEFT(node) == COPY_RIGHT(node))
        {
            *node = pow(COPY_LEFT(node), T(2));
            break;
        }
        break;
    }
    case OperationNode<T>::Type::DIV:
    {
        if (IS_NUM(LBRANCH(node)) && IS_NUM(RBRANCH(node)))
        {
            *node = TO_NUM(LBRANCH(node))->number / TO_NUM(RBRANCH(node))->number;
            break;
        }
        if (IS_NUM(LBRANCH(node)) && (TO_NUM(LBRANCH(node))->number == T()))
        {
            *node = T();
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T(1)))
        {
            *node = COPY_LEFT(node);
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T(-1)))
        {
            *node = -COPY_LEFT(node);
            break;
        }
        if (COPY_LEFT(node) == COPY_RIGHT(node))
        {
            *node = T(1);
            break;
        }
        break;
    }
    case OperationNode<T>::Type::POW:
    {
        if (IS_NUM(LBRANCH(node)) && IS_NUM(RBRANCH(node)))
        {
            *node = std::pow(TO_NUM(LBRANCH(node))->number, TO_NUM(RBRANCH(node))->number);
            break;
        }
        if (IS_NUM(LBRANCH(node)) && (TO_NUM(LBRANCH(node))->number == T()))
        {
            *node = T();
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T()))
        {
            *node = T(1);
            break;
        }
        if (IS_NUM(LBRANCH(node)) && (TO_NUM(LBRANCH(node))->number == T(1)))
        {
            *node = T(1);
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T(1)))
        {
            *node = COPY_LEFT(node);
            break;
        }
        if (IS_NUM(RBRANCH(node)) && (TO_NUM(RBRANCH(node))->number == T(-1)))
        {
            *node = T(1) / COPY_LEFT(node);
            break;
        }
        if (IS_OP(LBRANCH(node)) && (TO_OP(LBRANCH(node))->type == OperationNode<T>::Type::POW))
        {
            *node = pow(COPY_LEFT(LBRANCH(node)), (COPY_RIGHT(LBRANCH(node)) * COPY_RIGHT(node)));
            SIMPLIFY(RBRANCH(node));
            break;
        }
        break;
    }
    default: break;
    }
}

template<typename T>
FunctionNode<T>::FunctionNode(Type op_type) : type(op_type) {}

template<typename T>
ASTNode<T>::Type FunctionNode<T>::NodeType() const
{
    return ASTNode<T>::Type::FUNCTION;
}

template<typename T>
T FunctionNode<T>::calc(AST<T>* node, const Variables<T>& vars)
{
    T number = CALC(LBRANCH(node));

    static const T PI_2 = static_cast<T>(std::atan(1.0F) * 2.0F);

    switch (this->type)
    {
    case FunctionNode<T>::Type::ABS:     number = std::abs(number);            break;
    case FunctionNode<T>::Type::ARCCOS:  number = std::acos(number);           break;
    case FunctionNode<T>::Type::ARCCOSH: number = std::acosh(number);          break;
    case FunctionNode<T>::Type::ARCCOT:  number = PI_2 - std::atan(number);    break;
    case FunctionNode<T>::Type::ARCCOTH: number = std::atanh(T{ 1 } / number); break;
    case FunctionNode<T>::Type::ARCSIN:  number = std::asin(number);           break;
    case FunctionNode<T>::Type::ARCSINH: number = std::asinh(number);          break;
    case FunctionNode<T>::Type::ARCTAN:  number = std::atan(number);           break;
    case FunctionNode<T>::Type::ARCTANH: number = std::atanh(number);          break;
    case FunctionNode<T>::Type::ARG:     number = std::arg(number);            break;
    case FunctionNode<T>::Type::COS:     number = std::cos(number);            break;
    case FunctionNode<T>::Type::COSH:    number = std::cosh(number);           break;
    case FunctionNode<T>::Type::COT:     number = T{ 1 } / std::tan(number);   break;
    case FunctionNode<T>::Type::COTH:    number = T{ 1 } / std::tanh(number);  break;
    case FunctionNode<T>::Type::EXP:     number = std::exp(number);            break;
    case FunctionNode<T>::Type::LOG:     number = std::log(number);            break;
    case FunctionNode<T>::Type::LOG10:   number = std::log10(number);          break;
    case FunctionNode<T>::Type::SIN:     number = std::sin(number);            break;
    case FunctionNode<T>::Type::SINH:    number = std::sinh(number);           break;
    case FunctionNode<T>::Type::SQRT:    number = std::sqrt(number);           break;
    case FunctionNode<T>::Type::TAN:     number = std::tan(number);            break;
    case FunctionNode<T>::Type::TANH:    number = std::tanh(number);           break;
    default: break;
    }

    return number;
}

template<typename T>
void FunctionNode<T>::diff(AST<T>* node, const std::string& var_name)
{
    auto L = COPY_LEFT(node);
    auto Ld = COPY_LEFT(node);
    DIFF(&Ld);

    switch (this->type)
    {
    case FunctionNode<T>::Type::ABS:     *node = L * Ld / abs(L);                 break;
    case FunctionNode<T>::Type::ARCCOS:  *node = -Ld / sqrt(T(1) - pow(L, T(2))); break;
    case FunctionNode<T>::Type::ARCCOSH: *node = Ld / sqrt(pow(L, T(2)) - T(1));  break;
    case FunctionNode<T>::Type::ARCCOT:  *node = -Ld / (T(1) + pow(L, T(2)));     break;
    case FunctionNode<T>::Type::ARCCOTH: *node = Ld / (T(1) - pow(L, T(2)));      break;
    case FunctionNode<T>::Type::ARCSIN:  *node = Ld / sqrt(T(1) - pow(L, T(2)));  break;
    case FunctionNode<T>::Type::ARCSINH: *node = Ld / sqrt(T(1) + pow(L, T(2)));  break;
    case FunctionNode<T>::Type::ARCTAN:  *node = Ld / (T(1) + pow(L, T(2)));      break;
    case FunctionNode<T>::Type::ARCTANH: *node = Ld / (T(1) - pow(L, T(2)));      break;
    case FunctionNode<T>::Type::ARG:     *node = AST(T());                        break;
    case FunctionNode<T>::Type::COS:     *node = -Ld * sin(L);                    break;
    case FunctionNode<T>::Type::COSH:    *node = Ld * sinh(L);                    break;
    case FunctionNode<T>::Type::COT:     *node = -Ld / pow(sin(L), T(2));         break;
    case FunctionNode<T>::Type::COTH:    *node = -Ld / pow(sinh(L), T(2));        break;
    case FunctionNode<T>::Type::EXP:     *node = Ld * exp(L);                     break;
    case FunctionNode<T>::Type::LOG:     *node = Ld / L;                          break;
    case FunctionNode<T>::Type::LOG10:   *node = Ld / (L * log(AST(T(10))));      break;
    case FunctionNode<T>::Type::SIN:     *node = Ld * cos(L);                     break;
    case FunctionNode<T>::Type::SINH:    *node = Ld * cosh(L);                    break;
    case FunctionNode<T>::Type::SQRT:    *node = Ld / (T(2) * sqrt(L));           break;
    case FunctionNode<T>::Type::TAN:     *node = Ld / pow(cos(L), T(2));          break;
    case FunctionNode<T>::Type::TANH:    *node = Ld / pow(cosh(L), T(2));         break;
    default: break;
    }
}

template<typename T>
void FunctionNode<T>::simplify(AST<T>*) {}

template<typename T>
VariableNode<T>::VariableNode(const std::string& var_name) : name(var_name) {}

template<typename T>
ASTNode<T>::Type VariableNode<T>::NodeType() const
{
    return ASTNode<T>::Type::VARIABLE;
}

template<typename T>
T VariableNode<T>::calc(AST<T>*, const Variables<T>& vars)
{
    return vars.contains(this->name) ? vars.at(this->name) : T{};
}

template<typename T>
void VariableNode<T>::diff(AST<T>* node, const std::string& var_name)
{
    *node = (var_name == this->name) ? AST(T(1)) : AST(T());
}

template<typename T>
void VariableNode<T>::simplify(AST<T>*) {}

template<typename T>
NumberNode<T>::NumberNode(const T& number_value) : number(number_value) {}

template<typename T>
ASTNode<T>::Type NumberNode<T>::NodeType() const
{
    return ASTNode<T>::Type::NUMBER;
}

template<typename T>
T NumberNode<T>::calc(AST<T>*, const Variables<T>&)
{
    return this->number;
}

template<typename T>
void NumberNode<T>::diff(AST<T>* node, const std::string&)
{
    *node = AST(T());
}

template<typename T>
void NumberNode<T>::simplify(AST<T>*) {}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ASTNode<T>>& obj)
{
    switch (obj.get()->NodeType())
    {
    case ASTNode<T>::Type::OPERATION: return os << AST<T>::OperationName(static_cast<OperationNode<T>*>(obj.get())->type);
    case ASTNode<T>::Type::FUNCTION:  return os << AST<T>::FunctionName(static_cast<FunctionNode<T>*>(obj.get())->type);
    case ASTNode<T>::Type::VARIABLE:  return os << static_cast<VariableNode<T>*>(obj.get())->name;
    case ASTNode<T>::Type::NUMBER:    return os << static_cast<NumberNode<T>*>(obj.get())->number;
    }

    return os;
}

template<typename T>
AST<T>::AST(std::string expression, Error* err)
{
    size_t to_write = 0;
    for (char symb : expression)
    {
        if (!isspace(symb))
        {
            expression[to_write++] = symb;
        }
    }
    expression.erase(to_write);

    size_t pos = 0;
    Error error = parsePlusMinus(expression, &pos);

    if (err != nullptr)
    {
        *err = error;
    }
}

template<typename T>
AST<T>::AST(const char* expression, Error* err) : AST(std::string(expression), err) {}

template<typename T>
AST<T>::AST(const T& number) : Tree<std::shared_ptr<ASTNode<T>>>(std::make_shared<NumberNode<T>>(NumberNode<T>(number))) {}

template<typename T>
T AST<T>::operator()(std::initializer_list<Variable<T>> list)
{
    Variables<T> vars(list);
    return CALC(this);
}

template<typename T>
AST<T> AST<T>::operator + (const AST& a) const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::ADD));
    ast.push_branch(*this);
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> AST<T>::operator - (const AST& a) const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::SUB));
    ast.push_branch(*this);
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> AST<T>::operator * (const AST& a) const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::MUL));
    ast.push_branch(*this);
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> AST<T>::operator / (const AST& a) const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::DIV));
    ast.push_branch(*this);
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> AST<T>::operator - () const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::SUB));
    ast.push_branch(*this);
    return ast;
}

template<typename T>
AST<T> AST<T>::operator + () const
{
    return *this;
}

template<typename T>
AST<T>& AST<T>::operator += (const AST& a)
{
    AST<T> left = *this;
    this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::ADD));
    this->clear_branches();
    this->push_branch(left);
    this->push_branch(a);
    return *this;
}

template<typename T>
AST<T>& AST<T>::operator -= (const AST& a)
{
    AST<T> left = *this;
    this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::SUB));
    this->clear_branches();
    this->push_branch(left);
    this->push_branch(a);
    return *this;
}

template<typename T>
AST<T>& AST<T>::operator *= (const AST& a)
{
    AST<T> left = *this;
    this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::MUL));
    this->clear_branches();
    this->push_branch(left);
    this->push_branch(a);
    return *this;
}

template<typename T>
AST<T>& AST<T>::operator /= (const AST& a)
{
    AST<T> left = *this;
    this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::DIV));
    this->clear_branches();
    this->push_branch(left);
    this->push_branch(a);
    return *this;
}

template<typename T>
AST<T> AST<T>::operator + (const T& number) const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::ADD));
    ast.push_branch(*this);
    ast.push_branch(AST<T>(number));
    return ast;
}

template<typename T>
AST<T> AST<T>::operator - (const T& number) const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::SUB));
    ast.push_branch(*this);
    ast.push_branch(AST<T>(number));
    return ast;
}

template<typename T>
AST<T> AST<T>::operator * (const T& number) const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::MUL));
    ast.push_branch(*this);
    ast.push_branch(AST<T>(number));
    return ast;
}

template<typename T>
AST<T> AST<T>::operator / (const T& number) const
{
    AST<T> ast;
    ast.value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::DIV));
    ast.push_branch(*this);
    ast.push_branch(AST<T>(number));
    return ast;
}

template<typename T>
AST<T>& AST<T>::operator += (const T& number)
{
    AST<T> left = *this;
    this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::ADD));
    this->clear_branches();
    this->push_branch(left);
    this->push_branch(AST<T>(number));
    return *this;
}

template<typename T>
AST<T>& AST<T>::operator -= (const T& number)
{
    AST<T> left = *this;
    this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::SUB));
    this->clear_branches();
    this->push_branch(left);
    this->push_branch(AST<T>(number));
    return *this;
}

template<typename T>
AST<T>& AST<T>::operator *= (const T& number)
{
    AST<T> left = *this;
    this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::MUL));
    this->clear_branches();
    this->push_branch(left);
    this->push_branch(AST<T>(number));
    return *this;
}

template<typename T>
AST<T>& AST<T>::operator /= (const T& number)
{
    AST<T> left = *this;
    this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::DIV));
    this->clear_branches();
    this->push_branch(left);
    this->push_branch(AST<T>(number));
    return *this;
}

template<typename T>
AST<T>& AST<T>::operator = (const T& number)
{
    return *this = AST<T>(number);
}

template<typename T>
AST<T> operator + (const T& number, const AST<T>& right)
{
    AST<T> ast;
    ast.value() = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::ADD));
    ast.push_branch(AST<T>(number));
    ast.push_branch(right);
    return ast;
}

template<typename T>
AST<T> operator - (const T& number, const AST<T>& right)
{
    AST<T> ast;
    ast.value() = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::SUB));
    ast.push_branch(AST<T>(number));
    ast.push_branch(right);
    return ast;
}

template<typename T>
AST<T> operator * (const T& number, const AST<T>& right)
{
    AST<T> ast;
    ast.value() = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::MUL));
    ast.push_branch(AST<T>(number));
    ast.push_branch(right);
    return ast;
}

template<typename T>
AST<T> operator / (const T& number, const AST<T>& right)
{
    AST<T> ast;
    ast.value() = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::DIV));
    ast.push_branch(AST<T>(number));
    ast.push_branch(right);
    return ast;
}

template<typename T>
AST<T> pow(const AST<T>& left, const AST<T>& right)
{
    AST<T> ast;
    ast.value() = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::POW));
    ast.push_branch(left);
    ast.push_branch(right);
    return ast;
}

template<typename T>
AST<T> pow(const AST<T>& left, const T& number)
{
    AST<T> ast;
    ast.value() = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::POW));
    ast.push_branch(left);
    ast.push_branch(AST<T>(number));
    return ast;
}

template<typename T>
AST<T> pow(const T& number, const AST<T>& right)
{
    AST<T> ast;
    ast.value() = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::POW));
    ast.push_branch(AST<T>(number));
    ast.push_branch(right);
    return ast;
}

template<typename T>
AST<T> abs(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ABS));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arccos(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARCCOS));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arccosh(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARCCOSH));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arccot(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARCCOT));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arccoth(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARCCOTH));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arcsin(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARCSIN));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arcsinh(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARCSINH));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arctan(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARCTAN));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arctanh(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARCTANH));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> arg(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::ARG));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> cos(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::COS));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> cosh(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::COSH));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> cot(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::COT));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> coth(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::COTH));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> exp(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::EXP));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> log(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::LOG));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> log10(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::LOG10));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> sin(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::SIN));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> sinh(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::SINH));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> sqrt(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::SQRT));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> tan(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::TAN));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T> tanh(const AST<T>& a)
{
    AST<T> ast;
    ast.value() = std::make_shared<FunctionNode<T>>(FunctionNode<T>(FunctionNode<T>::Type::TANH));
    ast.push_branch(a);
    return ast;
}

template<typename T>
AST<T>& AST<T>::differentiate(const std::string& var_name)
{
    DIFF(this);
    return *this;
}

template<typename T>
AST<T> AST<T>::derivative(const std::string& var_name) const
{
    AST<T> ast = *this;
    return ast.differentiate(var_name);
}

template<typename T>
AST<T>& AST<T>::simplify()
{
    if (this->branches_num() != 0)
    {
        if (this->branches_num() == 2)
        {
            RBRANCH(this)->simplify();
            SIMPLIFY(RBRANCH(this));
        }

        LBRANCH(this)->simplify();
        SIMPLIFY(LBRANCH(this));
    }
    return *this;
}

template<typename T>
AST<T> AST<T>::simplified() const
{
    AST<T> ast = *this;
    return ast.simplify();
}

#define COND_RETURN(cond, ret) \
    if (cond)                  \
    {                          \
        return (ret);          \
    } //

template<typename T>
AST<T>::Error AST<T>::parsePlusMinus(const std::string& str, size_t* pos)
{
    if (str[*pos] == '-')
    {
        (*pos)++;

        AST<T> right;
        Error err = right.parseMulDiv(str, pos);
        COND_RETURN(err != Error::OK, err)

        this->clear_branches();
        this->push_branch(right);
        this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::SUB));
    }
    else
    {
        Error err = parseMulDiv(str, pos);
        COND_RETURN(err != Error::OK, err)
    }

    while ((str[*pos] == '+') || (str[*pos] == '-'))
    {
        typename OperationNode<T>::Type op_type = (str[*pos] == '-') ? OperationNode<T>::Type::SUB : OperationNode<T>::Type::ADD;
        (*pos)++;

        AST<T> left = *this;
        AST<T> right;

        Error err = right.parseMulDiv(str, pos);
        COND_RETURN(err != Error::OK, err)

        this->clear_branches();
        this->push_branch(left);
        this->push_branch(right);

        this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(op_type));
    }

    COND_RETURN((str[*pos] != '+') && (str[*pos] != '-') && (str[*pos] != '*') && (str[*pos] != '/') && (str[*pos] != '^') &&
        (str[*pos] != '(') && (str[*pos] != ')') && (str[*pos] != '\0'), Error::UNIDENTIFIED_OPERATION)

    return Error::OK;
}

template<typename T>
AST<T>::Error AST<T>::parseMulDiv(const std::string& str, size_t* pos)
{
    Error err = parsePower(str, pos);
    COND_RETURN(err != Error::OK, err)

    while ((str[*pos] == '*') || (str[*pos] == '/'))
    {
        typename OperationNode<T>::Type op_type = (str[*pos] == '*') ? OperationNode<T>::Type::MUL : OperationNode<T>::Type::DIV;
        (*pos)++;

        AST<T> left = *this;
        AST<T> right;

        err = right.parsePower(str, pos);
        COND_RETURN(err != Error::OK, err)

        this->clear_branches();
        this->push_branch(left);
        this->push_branch(right);

        this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(op_type));
    }

    return Error::OK;
}

template<typename T>
AST<T>::Error AST<T>::parsePower(const std::string& str, size_t* pos)
{
    Error err = parseBrackets(str, pos);
    COND_RETURN(err != Error::OK, err)

    while (str[*pos] == '^')
    {
        (*pos)++;

        AST<T> left = *this;
        AST<T> right;

        err = right.parseBrackets(str, pos);
        COND_RETURN(err != Error::OK, err)

        this->clear_branches();
        this->push_branch(left);
        this->push_branch(right);

        this->value_ = std::make_shared<OperationNode<T>>(OperationNode<T>(OperationNode<T>::Type::POW));
    }

    return Error::OK;
}

template<typename T>
AST<T>::Error AST<T>::parseBrackets(const std::string& str, size_t* pos)
{
    if (str[*pos] == '(')
    {
        (*pos)++;

        Error err = parsePlusMinus(str, pos);
        COND_RETURN(err != Error::OK, err)

        COND_RETURN(str[*pos] != ')', Error::NO_CLOSE_BRACKET)
        (*pos)++;

        return Error::OK;
    }

    return parseFunction(str, pos);
}

template<typename T>
AST<T>::Error AST<T>::parseFunction(const std::string& str, size_t* pos)
{
    COND_RETURN(isdigit(str[*pos]), parseNumber(str, pos))
    COND_RETURN(!isalpha(str[*pos]), Error::SYNTAX_ERROR)

    std::string word;
    while (isalpha(str[*pos]) || isdigit(str[*pos]))
    {
        word.push_back(str[(*pos)++]);
    }

    if (str[*pos] == '(')
    {
        typename FunctionNode<T>::Type func_type = FunctionType(word);
        COND_RETURN(func_type == FunctionNode<T>::Type::ERROR, Error::UNIDENTIFIED_FUNCTION)

        AST<T> right;
        Error err = right.parseBrackets(str, pos);
        COND_RETURN(err != Error::OK, err)

        this->clear_branches();
        this->push_branch(right);

        this->value_ = std::make_shared<FunctionNode<T>>(FunctionNode<T>(func_type));
    }
    else
    {
        this->value_ = std::make_shared<VariableNode<T>>(VariableNode<T>(word));
    }

    return Error::OK;
}

namespace {

template<typename T>
struct is_complex : public std::false_type {};

template<typename T>
struct is_complex<std::complex<T>> : public std::true_type {};

constexpr std::uint32_t hash(char const* str)
{
    return ((*str ? hash(str + 1) : 2166136261U) ^ static_cast<std::uint32_t>(*str)) * 16777619U;
}

} // namespace

template<typename T>
AST<T>::Error AST<T>::parseNumber(const std::string& str, size_t* pos)
{
    size_t end_pos = 0;
    auto value = static_cast<T>(std::stof(str.substr(*pos), &end_pos));
    *pos += end_pos;

    if constexpr (is_complex<T>())
    {
        if (str[*pos] == 'i')
        {
            (*pos)++;
            this->value_ = std::make_shared<NumberNode<T>>(NumberNode<T>({ 0, std::real(value) }));
        }
        else
        {
            this->value_ = std::make_shared<NumberNode<T>>(NumberNode<T>({ std::real(value), 0 }));
        }
    }
    else
    {
        this->value_ = std::make_shared<NumberNode<T>>(NumberNode<T>(value));
    }

    return Error::OK;
}

template<typename T>
OperationNode<T>::Type AST<T>::OperationType(const std::string& word)
{
    switch (word[0])
    {
    case '+': return OperationNode<T>::Type::ADD;
    case '-': return OperationNode<T>::Type::SUB;
    case '*': return OperationNode<T>::Type::MUL;
    case '/': return OperationNode<T>::Type::DIV;
    case '^': return OperationNode<T>::Type::POW;
    default:  return OperationNode<T>::Type::ERROR;
    }
}

template<typename T>
const char* AST<T>::OperationName(OperationNode<T>::Type op_type)
{
    switch (op_type)
    {
    case OperationNode<T>::Type::ADD: return "+";
    case OperationNode<T>::Type::SUB: return "-";
    case OperationNode<T>::Type::MUL: return "*";
    case OperationNode<T>::Type::DIV: return "/";
    case OperationNode<T>::Type::POW: return "^";
    default: return nullptr;
    }
}

template<typename T>
FunctionNode<T>::Type AST<T>::FunctionType(const std::string& word)
{
    switch (hash(word.c_str()))
    {
    case hash("abs"):     return FunctionNode<T>::Type::ABS;
    case hash("arccos"):  return FunctionNode<T>::Type::ARCCOS;
    case hash("arccosh"): return FunctionNode<T>::Type::ARCCOSH;
    case hash("arccot"):  return FunctionNode<T>::Type::ARCCOT;
    case hash("arccoth"): return FunctionNode<T>::Type::ARCCOTH;
    case hash("arcsin"):  return FunctionNode<T>::Type::ARCSIN;
    case hash("arcsinh"): return FunctionNode<T>::Type::ARCSINH;
    case hash("arctan"):  return FunctionNode<T>::Type::ARCTAN;
    case hash("arctanh"): return FunctionNode<T>::Type::ARCTANH;
    case hash("arg"):     return FunctionNode<T>::Type::ARG;
    case hash("cos"):     return FunctionNode<T>::Type::COS;
    case hash("cosh"):    return FunctionNode<T>::Type::COSH;
    case hash("cot"):     return FunctionNode<T>::Type::COT;
    case hash("coth"):    return FunctionNode<T>::Type::COTH;
    case hash("exp"):     return FunctionNode<T>::Type::EXP;
    case hash("log"):     return FunctionNode<T>::Type::LOG;
    case hash("log10"):   return FunctionNode<T>::Type::LOG10;
    case hash("sin"):     return FunctionNode<T>::Type::SIN;
    case hash("sinh"):    return FunctionNode<T>::Type::SINH;
    case hash("sqrt"):    return FunctionNode<T>::Type::SQRT;
    case hash("tan"):     return FunctionNode<T>::Type::TAN;
    case hash("tanh"):    return FunctionNode<T>::Type::TANH;
    default:              return FunctionNode<T>::Type::ERROR;
    }
}

template<typename T>
const char* AST<T>::FunctionName(FunctionNode<T>::Type func_type)
{
    switch (func_type)
    {
    case FunctionNode<T>::Type::ABS:     return "abs";
    case FunctionNode<T>::Type::ARCCOS:  return "arccos";
    case FunctionNode<T>::Type::ARCCOSH: return "arccosh";
    case FunctionNode<T>::Type::ARCCOT:  return "arccot";
    case FunctionNode<T>::Type::ARCCOTH: return "arccoth";
    case FunctionNode<T>::Type::ARCSIN:  return "arcsin";
    case FunctionNode<T>::Type::ARCSINH: return "arcsinh";
    case FunctionNode<T>::Type::ARCTAN:  return "arctan";
    case FunctionNode<T>::Type::ARCTANH: return "arctanh";
    case FunctionNode<T>::Type::ARG:     return "arg";
    case FunctionNode<T>::Type::COS:     return "cos";
    case FunctionNode<T>::Type::COSH:    return "cosh";
    case FunctionNode<T>::Type::COT:     return "cot";
    case FunctionNode<T>::Type::COTH:    return "coth";
    case FunctionNode<T>::Type::EXP:     return "exp";
    case FunctionNode<T>::Type::LOG:     return "log";
    case FunctionNode<T>::Type::LOG10:   return "log10";
    case FunctionNode<T>::Type::SIN:     return "sin";
    case FunctionNode<T>::Type::SINH:    return "sinh";
    case FunctionNode<T>::Type::SQRT:    return "sqrt";
    case FunctionNode<T>::Type::TAN:     return "tan";
    case FunctionNode<T>::Type::TANH:    return "tanh";
    default: break;
    }

    return nullptr;
}

#undef LBRANCH
#undef RBRANCH
#undef NODE_TYPE
#undef TO_OP
#undef TO_FUNC
#undef TO_VAR
#undef TO_NUM
#undef CALC
#undef DIFF
#undef SIMPLIFY
#undef COND_RETURN

} // namespace ast

#endif // AST_H