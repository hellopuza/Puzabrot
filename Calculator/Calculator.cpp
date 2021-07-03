/*------------------------------------------------------------------------------
    * File:        Calculator.cpp                                              *
    * Description: Functions for calculating math expressions.                 *
    * Created:     15 may 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#include "Calculator.h"

//------------------------------------------------------------------------------

Calculator::Calculator () :
    filename_     (nullptr),
    trees_        ((char*)"trees of expression"),
    variables_    ((char*)"variables"),
    state_        (CALC_OK)
{
    Tree<CalcNodeData> tree((char*)"expression");
    trees_.Push(tree);

    ADD_VAR(variables_);
}

//------------------------------------------------------------------------------

Calculator::Calculator (char* filename) :
    filename_     (filename),
    trees_        ((char*)"trees of expression"),
    variables_    ((char*)"variables"),
    state_        (CALC_OK)
{
    Tree<CalcNodeData> tree(GetTrueFileName(filename));
    trees_.Push(tree);

    ADD_VAR(variables_);
}

//------------------------------------------------------------------------------

Calculator::~Calculator ()
{
    CALC_ASSERTOK((this == nullptr),           CALC_NULL_INPUT_CALCULATOR_PTR);
    CALC_ASSERTOK((state_ == CALC_DESTRUCTED), CALC_DESTRUCTED               );

    filename_ = nullptr;

    state_ = CALC_DESTRUCTED;
}

//------------------------------------------------------------------------------

int Calculator::Run ()
{
    CALC_ASSERTOK((this == nullptr), CALC_NULL_INPUT_CALCULATOR_PTR);

    if (filename_ == nullptr)
    {
        bool running = true;
        while (running)
        {
            printf("\nEnter expression: ");

            char* expr = ScanExpr();
            Expression expression = { expr, expr };

            int err = Expr2Tree(expression, trees_[0]);
            delete [] expr;
            if (!err)
            {
                printExprGraph(trees_[0]);

                err = Calculate(trees_[0].root_);
                if (err)
                    printf("%s\n", calc_errstr[err + 1]);
                else
                    Write();
            }
            char* tree_name = trees_[0].name_;

            trees_.Clean();
            variables_.Clean();

            Tree<CalcNodeData> tree(GetTrueFileName(tree_name));
            trees_.Push(tree);

            ADD_VAR(variables_);

            printf("Continue [Y/n]? ");
            running = scanAns();
        }
    }
    else
    {
        Text text(filename_);
        char* expr = text.text_;
        Expression expression = { expr, expr };

        int err = Expr2Tree(expression, trees_[0]);
        delete [] expr;
        if (err) return err;

        printExprGraph(trees_[0]);

        err = Calculate(trees_[0].root_);
        if (err)
            printf("%s\n", calc_errstr[err + 1]);
        else
            Write();
    }
    
    return CALC_OK;
}

//------------------------------------------------------------------------------

int Calculator::Calculate (Node<CalcNodeData>* node_cur)
{
    assert(node_cur != nullptr);

    int err = 0;
    int index = -1;

    NUM_TYPE number    = 0;
    NUM_TYPE right_num = 0;
    NUM_TYPE left_num  = 0;

    switch (node_cur->getData().node_type)
    {
    case NODE_FUNCTION:
    {
        assert((node_cur->right_ != nullptr) && (node_cur->left_ == nullptr));
        int err = Calculate(node_cur->right_);
        if (err) return err;

        number = node_cur->right_->getData().number;

        #define ONE static_cast<NUM_TYPE>(1)
        #define TWO static_cast<NUM_TYPE>(2)

        switch (node_cur->getData().op_code)
        {
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

        #undef ONE
        #undef TWO

        node_cur->setData({ number, node_cur->getData().word, node_cur->getData().op_code, node_cur->getData().node_type });
        break;
    }
    case NODE_OPERATOR:
    {
        if (node_cur->left_ != nullptr)
        {
            int err = Calculate(node_cur->left_);
            if (err) return err;

            left_num = node_cur->left_->getData().number;
        }
        else left_num = 0;

        int err = Calculate(node_cur->right_);
        if (err) return err;

        right_num = node_cur->right_->getData().number;

        switch (node_cur->getData().op_code)
        {
        case OP_MUL:  number = left_num * right_num;     break;
        case OP_ADD:  number = left_num + right_num;     break;
        case OP_DIV:  number = left_num / right_num;     break;
        case OP_SUB:  number = left_num - right_num;     break;
        case OP_POW:  number = pow(left_num, right_num); break;
        default: assert(0);
        }

        node_cur->setData({ number, node_cur->getData().word, node_cur->getData().op_code, node_cur->getData().node_type });
        break;
    }
    case NODE_VARIABLE:
    {
        assert((node_cur->right_ == nullptr) && (node_cur->left_ == nullptr));

        index = -1;
        for (int i = 0; i < variables_.getSize(); ++i)
            if (strcmp(variables_[i].name, node_cur->getData().word) == 0)
            {
                index = i;
                break;
            }
        
        if (index == -1)
        {
            variables_.Push({ POISON<NUM_TYPE>, node_cur->getData().word });
            size_t size = variables_.getSize();

            number = scanVar(*this, node_cur->getData().word);
            variables_[size - 1] = { number, node_cur->getData().word };
        }
        else number = variables_[index].value;
        
        if (isPOISON(number))
        {
            return CALC_UNIDENTIFIED_VARIABLE;
        }

        node_cur->setData({ number, node_cur->getData().word, node_cur->getData().op_code, node_cur->getData().node_type });
        break;
    }
    case NODE_NUMBER:
        break;

    default: assert(0);
    }

    return CALC_OK;
}

//------------------------------------------------------------------------------

void Calculator::Write ()
{
    assert(trees_[0].root_->getData().node_type == NODE_NUMBER);

    char* strnum = Num2Str(trees_[0].root_->getData().number);
    if (filename_ == nullptr)
    {
        printf("result: %s\n", strnum);
    }
    else
    {
        FILE* output = fopen(filename_, "w");
        assert(output != nullptr);

        fprintf(output, "%s", strnum);
        fclose(output);
    }
    delete [] strnum;
}

//------------------------------------------------------------------------------

void CalcPrintError (const char* logname, const char* file, int line, const char* function, int err, bool console_err)
{
    assert(function != nullptr);
    assert(logname  != nullptr);
    assert(file     != nullptr);

    FILE* log = fopen(logname, "a");
    assert(log != nullptr);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(log, "###############################################################################\n");
    fprintf(log, "TIME: %d-%02d-%02d %02d:%02d:%02d\n\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(log, "ERROR: file %s  line %d  function %s\n\n", file, line, function);
    fprintf(log, "%s\n", calc_errstr[err + 1]);

    if (console_err)
        printf(  "ERROR: file %s  line %d  function %s\n\n", file, line, function);

    printf (     "ERROR: %s\n\n", calc_errstr[err + 1]);
}

//------------------------------------------------------------------------------

void TypePrint (FILE* fp, const CalcNodeData& node_data)
{
    assert (fp != nullptr);

    switch (node_data.node_type)
    {
    case NODE_FUNCTION:
    {
        fprintf(fp, "func: %s", node_data.word);
        break;
    }
    case NODE_OPERATOR:
    {
        fprintf(fp, "oper: %s", node_data.word);
        break;
    }
    case NODE_VARIABLE:
    {
        fprintf(fp, "var: %s", node_data.word);
        break;
    }
    case NODE_NUMBER:
    {
        char* strnum = Num2Str(node_data.number);
        fprintf(fp, "num: %s", strnum);
        delete [] strnum;
        break;
    }
    default: fprintf(fp, "err: %s", node_data.word); break;
    }
}

//------------------------------------------------------------------------------

void TypePrint (FILE* fp, const Variable& var)
{
    assert (fp != nullptr);

    char* strnum = Num2Str(var.value);
    fprintf(fp, "%s: %s", var.name, strnum);
    delete [] strnum;
}

//------------------------------------------------------------------------------

bool isPOISON (CalcNodeData value)
{
    return ( (isPOISON(real(value.number))) &&
             (isPOISON(imag(value.number))) &&
             (value.word      == nullptr)   &&
             (value.op_code   == 0)         &&
             (value.node_type == 0) );
}

//------------------------------------------------------------------------------

bool isPOISON (Variable var)
{
    return ( (isPOISON(real(var.value))) &&
             (isPOISON(imag(var.value))) &&
             (var.name  == nullptr) );
}

//------------------------------------------------------------------------------

bool scanAns ()
{
    char ans[MAX_STR_LEN] = "";

    char* err = fgets(ans, MAX_STR_LEN - 2, stdin);
    ans[0] = toupper(ans[0]);

    while ((ans[0] != 'Y') && (ans[0] != 'N') || (ans[1] != '\n') || !err)
    {
        printf("Try again [Y/n]? ");
        err = fgets(ans, MAX_STR_LEN - 2, stdin);
        ans[0] = toupper(ans[0]);
    }

    return ((ans[0] == 'Y') ? 1 : 0);
}

//------------------------------------------------------------------------------

NUM_TYPE scanVar (Calculator& calc, char* varname)
{
    printf("Enter value of variable %s: ", varname);

    char* expr = ScanExpr();
    Expression expression = { expr, expr };

    Tree<CalcNodeData> vartree(varname);
    while (Expr2Tree(expression, vartree))
    {
        delete [] expr;
        printf("Try again: ");
        expr = ScanExpr();
        Expression expression = { expr, expr };
    }
    delete [] expr;

    printExprGraph(vartree);
    calc.trees_.Push(vartree);
    size_t trees_size = calc.trees_.getSize();
    int err = calc.Calculate(calc.trees_[trees_size - 1].root_);
    if (err == CALC_UNIDENTIFIED_VARIABLE)
        return POISON<NUM_TYPE>;

    return calc.trees_[trees_size - 1].root_->getData().number;
}

//------------------------------------------------------------------------------

char* Num2Str (NUM_TYPE number)
{
    char* str = new char[64] {};

    if (isnan(real(number)) || isnan(imag(number)))
    {
        sprintf(str, "Not a number (Nan)");
        return str;
    }

    char* word = new char[64] {};
    char* begin = str;

    bool was_real = 0;
    if (abs(real(number)) > NIL)
    {
        sprintf(word, "%lf", real(number));

        if (abs(real(number) - (int)real(number)) <= NIL)
        {
            char* s = strchr(word, '.');
            if (s != nullptr) *s = '\0';
        }

        sprintf(str, "%s", word);
        str += strlen(word);
        was_real = 1;
    }

    bool was_imag = 0;
    if (abs(imag(number)) > NIL)
    {
        sprintf(word, "%lf", imag(number));

        if (abs(imag(number) - (int)imag(number)) <= NIL)
        {
            char* s = strchr(word, '.');
            if (s != nullptr) *s = '\0';
        }

        if (was_real && (imag(number) > NIL))
            sprintf(str++, "+");

        if ( (abs(imag(number) - static_cast<NUM_TYPE>(1)) <= NIL) ||
             (abs(imag(number) + static_cast<NUM_TYPE>(1)) <= NIL) )
        {
            sprintf(str, "i");
        }
        else
        {
            sprintf(str, "%s", word);
            str += strlen(word);
            sprintf(str, "i");
        }
        
        was_imag = 1;
    }

    if (not was_real && not was_imag)
        sprintf(str, "0");

    delete [] word;

    return begin;
}

//------------------------------------------------------------------------------

char* ScanExpr ()
{
    char* expr = new char [MAX_STR_LEN] {};
    char err   = 0;

    do err = (not fgets(expr, MAX_STR_LEN - 2, stdin)) || (*expr == '\n');
    while (err && printf("Try again: "));
    
    return expr;
}

//------------------------------------------------------------------------------

int Tree2Expr (const Tree<CalcNodeData>& tree, Expression& expr)
{
    assert(expr.str != nullptr);
    expr.symb_cur = expr.str;
    int err = Node2Str(tree.root_, &expr.symb_cur);
    CALC_ASSERTOK(err, err);
    expr.symb_cur = expr.str;

    return err;
}

//------------------------------------------------------------------------------

int Node2Str (Node<CalcNodeData>* node_cur, char** str)
{
    assert(node_cur != nullptr);
    assert(*str     != nullptr);

    int err = 0;

    switch (node_cur->getData().node_type)
    {
    case NODE_FUNCTION:
    {
        if ((node_cur->right_ == nullptr) || (node_cur->left_ != nullptr))
            return CALC_TREE_FUNC_WRONG_ARGUMENT;

        sprintf(*str, "%s", node_cur->getData().word);
        *str += strlen(node_cur->getData().word);

        sprintf(*str, "(");
        *str += 1;

        err = Node2Str(node_cur->right_, str);
        if (err) return err;

        sprintf(*str, ")");
        *str += 1;
        break;
    }
    case NODE_OPERATOR:
    {
        if ((node_cur->right_ == nullptr) ||
            (node_cur->left_  == nullptr) && (node_cur->getData().op_code != OP_SUB))
            return CALC_TREE_OPER_WRONG_ARGUMENTS;

        if (needBrackets(node_cur, node_cur->left_))
        {
            sprintf(*str, "(");
            *str += 1;

            if (node_cur->left_ != nullptr)
            {
                err = Node2Str(node_cur->left_, str);
                if (err) return err;
            }

            sprintf(*str, ")");
            *str += 1;
        }
        else
        if (node_cur->left_ != nullptr)
        {
            err = Node2Str(node_cur->left_, str);
            if (err) return err;
        }

        sprintf(*str, "%s", node_cur->getData().word);
        *str += 1;

        if (needBrackets(node_cur, node_cur->right_))
        {
            sprintf(*str, "(");
            *str += 1;

            err = Node2Str(node_cur->right_, str);
            if (err) return err;

            sprintf(*str, ")");
            *str += 1;
        }
        else
        {
            err = Node2Str(node_cur->right_, str);
            if (err) return err;
        }
        break;
    }
    case NODE_VARIABLE:
    {
        if ((node_cur->right_ != nullptr) || (node_cur->left_ != nullptr))
            return CALC_TREE_VAR_WRONG_ARGUMENT;

        sprintf(*str, "%s", node_cur->getData().word);
        *str += strlen(node_cur->getData().word);

        break;
    }
    case NODE_NUMBER:
    {
        if ((node_cur->right_ != nullptr) || (node_cur->left_ != nullptr))
            return CALC_TREE_NUM_WRONG_ARGUMENT;

        char* number = Num2Str(node_cur->getData().number);
        sprintf(*str, "%s", number);
        *str += strlen(number);
        delete [] number;

        break;
    }
    default: assert(0);
    }

    return CALC_OK;
}

//------------------------------------------------------------------------------

int Expr2Tree (Expression& expr, Tree<CalcNodeData>& tree)
{
    assert(expr.str != nullptr);

    del_spaces(expr.str);

    tree.root_ = pass_Plus_Minus(expr);
    if (tree.root_ == nullptr) return -1;

    tree.root_->recountPrev();
    tree.root_->recountDepth();

    return 0;
}

//------------------------------------------------------------------------------

Node<CalcNodeData>* pass_Plus_Minus (Expression& expr)
{
    Node<CalcNodeData>* node_cur = nullptr;

    if (*expr.symb_cur == '-')
    {   
        ++expr.symb_cur;

        Node<CalcNodeData>* right = pass_Mul_Div(expr);

        node_cur = new Node<CalcNodeData>;
        node_cur->setData({ POISON<NUM_TYPE>, op_names[OP_SUB].word, op_names[OP_SUB].code, NODE_OPERATOR });
        node_cur->right_ = right;
    }
    else node_cur = pass_Mul_Div(expr);
    
    while ( (*expr.symb_cur == '+') ||
            (*expr.symb_cur == '-')   )
    {
        char* symb_cur = expr.symb_cur;
        ++expr.symb_cur;

        Node<CalcNodeData>* left  = node_cur;
        Node<CalcNodeData>* right = pass_Mul_Div(expr);

        node_cur = new Node<CalcNodeData>;
        char op = (*symb_cur == '-') ? OP_SUB : OP_ADD;
        node_cur->setData({ POISON<NUM_TYPE>, op_names[op].word, op_names[op].code, NODE_OPERATOR });

        node_cur->right_ = right;
        node_cur->left_  = left;
    }

    CHECK_SYNTAX(( (*expr.symb_cur != '+') &&
                   (*expr.symb_cur != '-') &&
                   (*expr.symb_cur != '*') &&
                   (*expr.symb_cur != '/') &&
                   (*expr.symb_cur != '^') &&
                   (*expr.symb_cur != '(') &&
                   (*expr.symb_cur != ')') &&
                   (*expr.symb_cur != '\0') ), CALC_SYNTAX_ERROR, expr, 1);

    return node_cur;
}

//------------------------------------------------------------------------------

Node<CalcNodeData>* pass_Mul_Div (Expression& expr)
{
    Node<CalcNodeData>* node_cur = pass_Power(expr);

    while ( (*expr.symb_cur == '*') ||
            (*expr.symb_cur == '/') )
    {
        char* symb_cur = expr.symb_cur;
        ++expr.symb_cur;

        Node<CalcNodeData>* left  = node_cur;
        Node<CalcNodeData>* right = pass_Power(expr);

        node_cur = new Node<CalcNodeData>;
        char op = (*symb_cur == '*') ? OP_MUL : OP_DIV;
        node_cur->setData({ POISON<NUM_TYPE>, op_names[op].word, op_names[op].code, NODE_OPERATOR });

        node_cur->right_ = right;
        node_cur->left_  = left;
    }

    return node_cur;
}

//------------------------------------------------------------------------------

Node<CalcNodeData>* pass_Power (Expression& expr)
{
    Node<CalcNodeData>* node_cur = pass_Brackets(expr);

    while (*expr.symb_cur == '^')
    {
        ++expr.symb_cur;

        Node<CalcNodeData>* left  = node_cur;
        Node<CalcNodeData>* right = pass_Power(expr);

        node_cur = new Node<CalcNodeData>;
        node_cur->setData({ POISON<NUM_TYPE>, op_names[OP_POW].word, op_names[OP_POW].code, NODE_OPERATOR });

        node_cur->right_ = right;
        node_cur->left_  = left;
    }
    
    return node_cur;
}

//------------------------------------------------------------------------------

Node<CalcNodeData>* pass_Brackets (Expression& expr)
{
    if (*expr.symb_cur == '(')
    {
        ++expr.symb_cur;

        Node<CalcNodeData>* node_cur = pass_Plus_Minus(expr);

        CHECK_SYNTAX((*expr.symb_cur != ')'), CALC_SYNTAX_NO_CLOSE_BRACKET, expr, 1);
        ++expr.symb_cur;

        return node_cur;
    }

    else return pass_Function(expr);
}

//------------------------------------------------------------------------------

Node<CalcNodeData>* pass_Function (Expression& expr)
{
    if (isdigit(*expr.symb_cur)) return pass_Number(expr);

    else
    {
        CHECK_SYNTAX((not isalpha(*expr.symb_cur)), CALC_SYNTAX_ERROR, expr, 1);

        char* word = new char [MAX_STR_LEN] {};
        size_t index = 0;

        while (isalpha(*expr.symb_cur) || isdigit(*expr.symb_cur))
        {
            word[index++] = *expr.symb_cur;

            ++expr.symb_cur;
        }

        word[index] = '\0';

        if (*expr.symb_cur == '(')
        {
            int code = findFunc(word);
            delete [] word;

            Expression old = { expr.str, expr.symb_cur - index };
            CHECK_SYNTAX((code == 0), CALC_SYNTAX_UNIDENTIFIED_FUNCTION, old, index);

            Node<CalcNodeData>* arg = pass_Brackets(expr);

            Node<CalcNodeData>* node_cur = new Node<CalcNodeData>;
            node_cur->setData({ POISON<NUM_TYPE>, op_names[code].word, op_names[code].code, NODE_FUNCTION });
            node_cur->right_ = arg;

            return node_cur;
        }
        else
        {
            Node<CalcNodeData>* node_cur = new Node<CalcNodeData>;
            node_cur->setData({ POISON<NUM_TYPE>, word, 0, NODE_VARIABLE });

            return node_cur;
        }   
    }
}

//------------------------------------------------------------------------------

Node<CalcNodeData>* pass_Number (Expression& expr)
{
    double value = 0;
    char* begin = expr.symb_cur;

    value = strtod(expr.symb_cur, &expr.symb_cur);
    CHECK_SYNTAX((expr.symb_cur == begin), CALC_SYNTAX_NUMBER_ERROR, expr, 1);

    Node<CalcNodeData>* node_cur = new Node<CalcNodeData>;

    if (*expr.symb_cur == 'i')
    {
        ++expr.symb_cur;
        node_cur->setData({ {0, value}, nullptr, 0, NODE_NUMBER });
    }
    else node_cur->setData({ {value, 0}, nullptr, 0, NODE_NUMBER });

    return node_cur;
}

//------------------------------------------------------------------------------

bool needBrackets (Node<CalcNodeData>* node, Node<CalcNodeData>* child)
{
    if  ( ((node-> getData().op_code == OP_MUL) || (node-> getData().op_code == OP_DIV)) &&
          ((child->getData().op_code == OP_ADD) || (child->getData().op_code == OP_SUB))   )
        return true;
    else
        return ( (node-> getData().op_code   == OP_POW)        &&
                 (child->getData().node_type == NODE_OPERATOR) &&
                 (child->getData().op_code   != OP_POW)          );
}

//------------------------------------------------------------------------------

char findFunc (char* word)
{
    assert(word != nullptr);

    operation func_key = { 0, word };

    operation* p_func_struct = (operation*)bsearch(&func_key, op_names, OP_NUM, sizeof(op_names[0]), CompareOP_Names);

    if (p_func_struct != nullptr) return p_func_struct->code;

    return 0;
}

//------------------------------------------------------------------------------

void Optimize (Tree<CalcNodeData>& tree)
{
    bool running = true;
    while (running)
    {
        running = Optimize(tree, tree.root_);
    }
    tree.root_->recountDepth();
}

//------------------------------------------------------------------------------

#define OPTIMIZE_ACTION(node_to_place)                   \
        {                                                \
            if (node_cur->prev_ == nullptr)              \
                tree.root_ = node_to_place;              \
            else                                         \
            if (node_cur->prev_->left_ == node_cur)      \
                node_cur->prev_->left_ = node_to_place;  \
            else                                         \
                node_cur->prev_->right_ = node_to_place; \
                                                         \
            node_to_place->prev_ = node_cur->prev_;      \
            node_to_place = nullptr;                     \
                                                         \
            delete node_cur;                             \
            return true;                                 \
        } //

//------------------------------------------------------------------------------

#define CALCULATE_ACTION(node, operation)                           \
        {                                                           \
            NUM_TYPE number1 = node->left_ ->getData().number;      \
            NUM_TYPE number2 = node->right_->getData().number;      \
                                                                    \
            Node<CalcNodeData>* newnode = new Node<CalcNodeData>;   \
            number1 = number1 operation number2;                    \
                                                                    \
            newnode->setData({ number1, nullptr, 0, NODE_NUMBER }); \
            OPTIMIZE_ACTION(newnode);                               \
        } //

//------------------------------------------------------------------------------

bool Optimize (Tree<CalcNodeData>& tree, Node<CalcNodeData>* node_cur)
{
    assert(node_cur != nullptr);

    int err = 0;

    switch (node_cur->getData().node_type)
    {
    case NODE_FUNCTION:

        return Optimize(tree, node_cur->right_);
        break;

    case NODE_OPERATOR:

        switch (node_cur->getData().op_code)
        {
        case OP_ADD:
        case OP_SUB:

            if (node_cur->left_ == nullptr)
            {
                if (abs(node_cur->right_->getData().number) <= NIL)
                {
                    OPTIMIZE_ACTION(node_cur->right_);
                }
                else return Optimize(tree, node_cur->right_);
            }
            else
            if (abs(node_cur->left_->getData().number) <= NIL)
            {
                OPTIMIZE_ACTION(node_cur->right_);
            }
            else
            if (abs(node_cur->right_->getData().number) <= NIL)
            {
                OPTIMIZE_ACTION(node_cur->left_);
            }
            else
            if ( (node_cur->left_ ->getData().node_type == NODE_NUMBER) &&
                 (node_cur->right_->getData().node_type == NODE_NUMBER)   )
            {
                if (node_cur->getData().op_code == OP_ADD)
                {
                    CALCULATE_ACTION(node_cur, +);
                }
                else
                {
                    CALCULATE_ACTION(node_cur, -);
                }
            }
            else
            {
                if (Optimize(tree, node_cur->left_)) return true;

                return Optimize(tree, node_cur->right_);
            }
            break;

        case OP_MUL:

            if (abs(node_cur->left_->getData().number) <= NIL)
            {
                OPTIMIZE_ACTION(node_cur->left_);
            }
            else
            if (abs(node_cur->right_->getData().number) <= NIL)
            {
                OPTIMIZE_ACTION(node_cur->right_);
            }
            else
            if (abs(node_cur->left_->getData().number - static_cast<NUM_TYPE>(1)) <= NIL)
            {
                OPTIMIZE_ACTION(node_cur->right_);
            }
            else
            if (abs(node_cur->right_->getData().number - static_cast<NUM_TYPE>(1)) <= NIL)
            {
                OPTIMIZE_ACTION(node_cur->left_);
            }
            else
            if ( (node_cur->left_ ->getData().node_type == NODE_NUMBER) &&
                 (node_cur->right_->getData().node_type == NODE_NUMBER)   )
            {
                CALCULATE_ACTION(node_cur, *);
            }
            else
            {
                if (Optimize(tree, node_cur->left_)) return true;

                return Optimize(tree, node_cur->right_);
            }
            break;

        case OP_DIV:

            if (abs(node_cur->left_->getData().number) <= NIL)
            {
                OPTIMIZE_ACTION(node_cur->left_);
            }
            else
            if (abs(node_cur->right_->getData().number - static_cast<NUM_TYPE>(1)) <= NIL)
            {
                OPTIMIZE_ACTION(node_cur->left_);
            }
            else
            if ( (abs(node_cur->left_->getData().number - node_cur->right_->getData().number) <= NIL) &&
                 ((node_cur->left_->getData().node_type == NODE_VARIABLE) || (node_cur->left_->getData().node_type == NODE_NUMBER)) )
            {
                Node<CalcNodeData>* newnode = new Node<CalcNodeData>;
                newnode->setData({ {1, 0}, nullptr, 0, NODE_NUMBER });

                OPTIMIZE_ACTION(newnode);
            }
            else
            {
                if (Optimize(tree, node_cur->left_)) return true;

                return Optimize(tree, node_cur->right_);
            }
            break;
        }
        break;

    case NODE_VARIABLE:
        break;
    
    case NODE_NUMBER:
        break;

    default: assert(0);
    }

    return false;
}

//------------------------------------------------------------------------------

bool isPOISON (NUM_TYPE value)
{
    if (isnan(real(value)) || isnan(imag(value)))
        return 1;
    else
        return 0;
}

//------------------------------------------------------------------------------

void printExprGraph (Tree<CalcNodeData> tree)
{
    char graphname[128] = "";
    sprintf(graphname, "%s.dot", tree.name_);
    
    FILE* graph = fopen(graphname, "w");
    assert(graph != nullptr);

    fprintf(graph, "digraph G{\n" "rankdir = HR;\n node[shape=box];\n");

    printExprGraphNode(graph, tree.root_);

    fprintf(graph, "\tlabelloc=\"t\";"
                   "\tlabel=\"Expression: %s\";"
                   "}\n", tree.name_);

    fclose(graph);

    char command[512] = "";

#if defined(WIN32)

    sprintf(command, "win_iconv -f 1251 -t UTF8 \"%s\" > \"new%s\"", graphname, graphname);

    int err = system(command);

    sprintf(command, "dot -Tpng -o %s.png new%s", tree.name_, graphname);
    if (!err) err = system(command);

    sprintf(command, "del new%s", graphname);
    if (!err) err = system(command);

    sprintf(command, "del %s", graphname);
    if (!err) err = system(command);

#elif defined(__linux__)

    sprintf(command, "iconv -f UTF8 -t UTF8 \"%s\" -o \"new%s\"", graphname, graphname);

    int err = system(command);

    sprintf(command, "dot -Tpng -o %s.png new%s", tree.name_, graphname);
    if (!err) err = system(command);

    sprintf(command, "rm new%s", graphname);
    if (!err) err = system(command);

    sprintf(command, "rm %s", graphname);
    if (!err) err = system(command);

#else
#error Program is only supported by linux or windows platforms
#endif
}

//------------------------------------------------------------------------------

void printExprGraphNode (FILE* graph, Node<CalcNodeData>* node_cur)
{
    assert(graph != nullptr);

    char* data      = new char[64] {};
    char* fillcolor = new char[64] {};

    getDataAndColor(node_cur, &data, &fillcolor);

    fprintf(graph, "\t %lu [shape = box, style = filled, color = black, fillcolor = %s, label = \"%s\"]\n", (size_t)node_cur, fillcolor, data);

    if (node_cur->left_ != nullptr)
    {
        char* leftdata = new char[64] {};
        getDataAndColor(node_cur->left_, &leftdata, &fillcolor);
        fprintf(graph, "\t %lu -> %lu [label=\"left\"]\n", (size_t)node_cur, (size_t)node_cur->left_);
        delete [] leftdata;
    }

    if (node_cur->right_ != nullptr)
    {
        char* rightdata = new char[64] {};
        getDataAndColor(node_cur->right_, &rightdata, &fillcolor);
        fprintf(graph, "\t %lu -> %lu [label=\"right\"]\n", (size_t)node_cur, (size_t)node_cur->right_);
        delete [] rightdata;
    }

    delete [] data;
    delete [] fillcolor;

    if (node_cur->left_  != nullptr) printExprGraphNode(graph, node_cur->left_);
    if (node_cur->right_ != nullptr) printExprGraphNode(graph, node_cur->right_);
}

//------------------------------------------------------------------------------

void getDataAndColor (Node<CalcNodeData>* node_cur, char** data, char** fillcolor)
{
    switch (node_cur->getData().node_type)
    {
    case NODE_FUNCTION:
    {
        sprintf(*fillcolor, "tomato");
        sprintf(*data, "%s()", node_cur->getData().word);
        break;
    }
    case NODE_OPERATOR:
    {
        sprintf(*fillcolor, "springgreen2");
        sprintf(*data, "%s", node_cur->getData().word);
        break;
    }
    case NODE_VARIABLE:
    {
        sprintf(*fillcolor, "deepskyblue2");
        sprintf(*data, "%s", node_cur->getData().word);
        break;
    }
    case NODE_NUMBER:
    {
        sprintf(*fillcolor, "darkgoldenrod1");
        char* strnum = Num2Str(node_cur->getData().number);
        sprintf(*data, "%s", strnum);
        delete [] strnum;
        break;
    }
    default: sprintf(*data, "err: %s", node_cur->getData().word); break;
    }
}

//------------------------------------------------------------------------------

void PrintBadExpr (const char* logname, const Expression& expr, size_t len)
{
    assert(logname != nullptr);

    FILE* log = fopen(logname, "a");
    assert(log != nullptr);

    fprintf(log, "\t %s\n", expr.str);
    printf (     "\t %s\n", expr.str);

    fprintf(log, "\t ");
    printf (     "\t ");

    for (int i = 0; i < expr.symb_cur - expr.str; ++i)
    {
        fprintf(log, " ");
        printf (     " ");
    }

    fprintf(log, "^");
    printf (     "^");

    for (int i = 0; i < len - 1; ++i)
    {
        fprintf(log, "~");
        printf (     "~");
    }

    fprintf(log, "\n");
    printf (     "\n");
}

//------------------------------------------------------------------------------
