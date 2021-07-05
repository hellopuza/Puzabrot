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


#include "../StringLib/StringLib.h"
#include "../TreeLib/Tree.h"
#include "Operations.h"
#include <complex>
#include <math.h>
#include <omp.h>


#define NUM_TYPE std::complex<double>

const size_t NUM_TYPE_SIZE = sizeof(NUM_TYPE);

const NUM_TYPE PI = {atan(1) * 4, 0};
const NUM_TYPE E  = {exp(1),      0};
const NUM_TYPE I  = {0,           1};

template<> constexpr NUM_TYPE POISON<NUM_TYPE> = {NAN, NAN};

constexpr double NIL = 1e-9;

#define ADD_VAR(variables)                \
        {                                 \
            variables.Push({ PI, "pi" }); \
            variables.Push({ E,  "e"  }); \
            variables.Push({ I,  "i"  }); \
        } //


//==============================================================================
/*------------------------------------------------------------------------------
                   Calculator errors                                           *
*///----------------------------------------------------------------------------
//==============================================================================


enum CalculatorErrors
{
    CALC_NOT_OK = -1                                                       ,
    CALC_OK = 0                                                            ,
    CALC_NO_MEMORY                                                         ,

    CALC_DESTRUCTED                                                        ,
    CALC_NULL_INPUT_CALCULATOR_PTR                                         ,
    CALC_SYNTAX_ERROR                                                      ,
    CALC_SYNTAX_NO_CLOSE_BRACKET                                           ,
    CALC_SYNTAX_NUMBER_ERROR                                               ,
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
    "Failed to allocate memory"                                            ,

    "Calculator has already destructed"                                    ,
    "The input value of the calculator pointer turned out to be zero"      ,
    "Syntax error"                                                         ,
    "Close bracket \')\' required here"                                    ,
    "Wrong number"                                                         ,
    "Unidentified function"                                                ,
    "Function node must have one children on the right branch"             ,
    "Number node must not have any children"                               ,
    "Operator node must have two children"                                 ,
    "Variable node must not have any children"                             ,
    "I do not solve equations"                                             ,
    "Wrong variable detected"                                              ,
};

char const * const CALCULATOR_LOGNAME = "calculator.log";

#define CHECK_SYNTAX(cond, errcode, expr, len) if (cond)                                                                            \
                                               {                                                                                    \
                                                 CalcPrintError(CALCULATOR_LOGNAME, __FILE__, __LINE__, __FUNC_NAME__, errcode, 0); \
                                                 PrintBadExpr(CALCULATOR_LOGNAME, expr, len);                                       \
                                                 expr.err = errcode;                                                                \
                                                 return nullptr;                                                                    \
                                               } //

#define CALC_ASSERTOK(cond, err) if (cond)                                                                        \
                                 {                                                                                \
                                   CalcPrintError(CALCULATOR_LOGNAME, __FILE__, __LINE__, __FUNC_NAME__, err, 1); \
                                   exit(err);                                                                     \
                                 } //


//==============================================================================
/*------------------------------------------------------------------------------
                   Calculator constants and types                              *
*///----------------------------------------------------------------------------
//==============================================================================


char const * const GRAPH_FILENAME = "Equation.dot";
const size_t       MAX_STR_LEN    = 4096;

enum NODE_TYPE 
{
    NODE_FUNCTION = 1,
    NODE_OPERATOR = 2,
    NODE_VARIABLE = 3,
    NODE_NUMBER   = 4,
};

struct Expression 
{
    char* str      = nullptr;
    char* symb_cur = nullptr;
    int   err      = CALC_OK;
};

struct CalcNodeData
{
    NUM_TYPE number    = POISON<NUM_TYPE>;
    char*    word      = nullptr;
    char     op_code   = 0;
    char     node_type = 0;
};

template<> const char* const      PRINT_TYPE<CalcNodeData> = "CalcNodeData";
template<> constexpr CalcNodeData POISON    <CalcNodeData> = {};

bool isPOISON  (CalcNodeData value);
void TypePrint (FILE* fp, const CalcNodeData& node_data);


struct Variable
{
    NUM_TYPE    value = POISON<NUM_TYPE>;
    const char* name  = nullptr;
};

template<> const char* const  PRINT_TYPE<Variable> = "Variable";
template<> constexpr Variable POISON    <Variable> = {};

bool isPOISON  (Variable value);
void TypePrint (FILE* fp, const Variable& var);


class Calculator
{
private:

    int state_;
    char* filename_;

public:

    Stack<Tree<CalcNodeData>> trees_;
    Stack<Variable>           variables_;

//------------------------------------------------------------------------------
/*! @brief   Calculator default constructor.
*/

    Calculator ();

//------------------------------------------------------------------------------
/*! @brief   Calculator constructor.
 *
 *  @param   filename    Name of input file
 */

    Calculator (char* filename);

//------------------------------------------------------------------------------
/*! @brief   Calculator copy constructor (deleted).
 *
 *  @param   obj         Source calculator
 */

    Calculator (const Calculator& obj);

    Calculator& operator = (const Calculator& obj); // deleted

//------------------------------------------------------------------------------
/*! @brief   Calculator destructor.
 */

   ~Calculator ();

//------------------------------------------------------------------------------
/*! @brief   Execution process.
 *
 *  @return  error code
 */

    int Run ();

//------------------------------------------------------------------------------
/*! @brief   Calculating process.
 *
 *  @param   node_cur      Current node
 *  @param   with_new_var  If not all required variables are defined on the stack
 *
 *  @return  error code
 */

    int Calculate (Node<CalcNodeData>* node_cur, bool with_new_var);

/*------------------------------------------------------------------------------
                   Private functions                                           *
*///----------------------------------------------------------------------------

private:

//------------------------------------------------------------------------------
/*! @brief   Write calculated result to console or to file.
 */

    void Write ();

//------------------------------------------------------------------------------
};

//------------------------------------------------------------------------------
/*! @brief   Prints an error wih description to the console and to the log file.
 *
 *  @param   logname      Name of the log file
 *  @param   file         Name of the program file
 *  @param   line         Number of line with an error
 *  @param   function     Name of the function with an error
 *  @param   err          Error code
 *  @param   console_err  Print error to console or not
 */

void CalcPrintError (const char* logname, const char* file, int line, const char* function, int err, bool console_err);

//------------------------------------------------------------------------------
/*! @brief   Get an answer from stdin (yes or no).
 *
 *  @return  true or false
 */

bool scanAns ();

//------------------------------------------------------------------------------
/*! @brief   Get a variable value from stdin.
 *
 *  @param   calc        Calculator for counting
 *  @param   varname     Variable name
 *
 *  @return  number
 */

NUM_TYPE scanVar (Calculator& calc, char* varname);

//------------------------------------------------------------------------------
/*! @brief   Convert complex number to c string.
 *
 *  @param   number      Complex number
 *
 *  @return  string
 */

char* Num2Str (NUM_TYPE number);

//------------------------------------------------------------------------------
/*! @brief   Get string equation from stdin.
 * 
 *  @return  string expression
 */

char* ScanExpr ();

//------------------------------------------------------------------------------
/*! @brief   Convert tree to string expression.
 * 
 *  @param   tree        Equation tree
 *  @param   expr        String expression
 *
 *  @return  error code
 */

int Tree2Expr (const Tree<CalcNodeData>& tree, Expression& expr);

//------------------------------------------------------------------------------
/*! @brief   Convert tree node to string expression.
 * 
 *  @param   node_cur    Current node
 *  @param   str         C string
 *
 *  @return  error code
 */

int Node2Str (Node<CalcNodeData>* node_cur, char** str);

//------------------------------------------------------------------------------
/*! @brief   Convert string expression to tree.
 * 
 *  @param   expr        String expression
 *  @param   tree        Equation tree
 *
 *  @return  -1 if error, 0 if ok
 */

int Expr2Tree (Expression& expr, Tree<CalcNodeData>& tree);

//------------------------------------------------------------------------------
/*! @brief   Parsing of expression beginning with plus and minus signs.
 * 
 *  @param   expr        String expression
 *
 *  @return  error code
 */

Node<CalcNodeData>* pass_Plus_Minus (Expression& expr);

//------------------------------------------------------------------------------
/*! @brief   Parsing of expression with mulptiply and division signs.
 * 
 *  @param   expr        String expression
 *
 *  @return  error code
 */

Node<CalcNodeData>* pass_Mul_Div (Expression& expr);

//------------------------------------------------------------------------------
/*! @brief   Parsing of expression with power signs.
 * 
 *  @param   expr        String expression
 *
 *  @return  error code
 */

Node<CalcNodeData>* pass_Power (Expression& expr);

//------------------------------------------------------------------------------
/*! @brief   Parsing of expression with brackets.
 * 
 *  @param   expr        String expression
 *
 *  @return  error code
 */

Node<CalcNodeData>* pass_Brackets (Expression& expr);

//------------------------------------------------------------------------------
/*! @brief   Parsing of expression with function.
 * 
 *  @param   expr        String expression
 *
 *  @return  error code
 */

Node<CalcNodeData>* pass_Function (Expression& expr);

//------------------------------------------------------------------------------
/*! @brief   Parsing of expression with number.
 * 
 *  @param   expr        String expression
 *
 *  @return  error code
 */

Node<CalcNodeData>* pass_Number (Expression& expr);

//------------------------------------------------------------------------------
/*! @brief   Check if there are need brackets for operator.
 *
 *  @param   node_cur    Current node
 *  @param   child       Current node's child
 *
 *  @return  true if need, else false
 */

bool needBrackets (Node<CalcNodeData>* node, Node<CalcNodeData>* child);

//------------------------------------------------------------------------------
/*! @brief   Function identifier.
 *
 *  @param   word        C string to be recognized
 *
 *  @return  function code if found else NOT_OK
 */

char findFunc (char* word);

//------------------------------------------------------------------------------
/*! @brief   Optimize expression process.
 *
 *  @param   tree        Tree to optimize
 *
 *  @return  error code
 */

void Optimize (Tree<CalcNodeData>& tree);

//------------------------------------------------------------------------------
/*! @brief   Optimize expression process.
 *
 *  @param   tree        Tree to optimize
 *  @param   node_cur    Node to optimize
 *
 *  @return  return 0 if optimized, else 1
 */

bool Optimize (Tree<CalcNodeData>& tree, Node<CalcNodeData>* node_cur);

//------------------------------------------------------------------------------
/*! @brief   Check if value is POISON.
 *
 *  @param   value       Value to be checked
 *
 *  @return 1 if value is POISON, else 0
 */

bool isPOISON (NUM_TYPE value);

//------------------------------------------------------------------------------
/*! @brief   Print the contents of the tree like a graphviz dot file.
 *
 *  @param   tree        Tree to visualize
 */

void printExprGraph (Tree<CalcNodeData> tree);

//------------------------------------------------------------------------------
/*! @brief   Recursive print the contents of the tree like a graphviz dot file.
 *
 *  @param   graph       Dump graphviz dot file
 *  @param   node_cur    Node to visualize
 */

void printExprGraphNode (FILE* graph, Node<CalcNodeData>* node_cur);

//------------------------------------------------------------------------------
/*! @brief   Get data and fillcolor of node for printing the contents of the tree like a graphviz dot file.
 *
 *  @param   node_cur    Node to visualize
 *  @param   data        Pointer to string node data
 *  @param   node_cur    Pointer to string color name
 */

void getDataAndColor (Node<CalcNodeData>* node_cur, char** data, char** fillcolor);

//------------------------------------------------------------------------------
/*! @brief   Prints an expression indicating an error.
 *
 *  @param   logname     Name of the log file
 *  @param   expr        Bad expression
 *  @param   len         Length of string error
 */

void PrintBadExpr (const char* logname, const Expression& expr, size_t len);

//------------------------------------------------------------------------------

#endif // CALCULATOR_H_INCLUDED
