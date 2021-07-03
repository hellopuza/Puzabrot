/*------------------------------------------------------------------------------
    * File:        Tree.h                                                      *
    * Description: Declaration of functions and data types used for binary     *
                   trees.                                                      *
    * Created:     18 apr 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS


#include "../StringLib/StringLib.h"

#define NO_DUMP
#define NO_HASH
#include "../StackLib/Stack.h"
#undef NO_HASH
#undef NO_DUMP

#include "TreeConfig.h"
#include <type_traits>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <new>


#define TREE_CHECK if (Check ())                            \
                   {                                        \
                     Dump(DUMP_NAME);                       \
                     TREE_ASSERTOK(errCode_, errCode_, -1); \
                   } //


#define TREE_ASSERTOK(cond, err, line) if (cond)                                                                  \
                                       {                                                                          \
                                         PrintError(TREE_LOGNAME , __FILE__, __LINE__, __FUNC_NAME__, err, line); \
                                         exit(err);                                                               \
                                       } //

#define CHECK_BRACKET(lines_arr, line, bracket)          \
        (                                                \
          (lines_arr[line].str[0] != bracket) ||         \
          (                                              \
              (not isspace(base.lines_[line].str[1])) && \
              (base.lines_[line].str[1] != '\0')         \
          )                                              \
        ) //

static int tree_id = 0;

#define newTree(NAME, STK_TYPE) \
        Tree<STK_TYPE> NAME ((char*)#NAME);

#define newTree_root(NAME, root, STK_TYPE) \
        Tree<STK_TYPE> NAME ((char*)#NAME, root);

#define newTree_base(NAME, base, STK_TYPE) \
        Tree<STK_TYPE> NAME ((char*)#NAME, base);


template <typename TYPE>
class Tree;

template<typename TYPE> const char* const PRINT_TYPE<Tree<TYPE>> = "Tree";
template<typename TYPE> const Tree<TYPE>  POISON    <Tree<TYPE>> = {};

template<typename TYPE> bool isPOISON  (Tree<TYPE> tree);
template<typename TYPE> void TypePrint (FILE* fp, const Tree<TYPE>& tree);


template <typename TYPE>
struct Node
{
private:

    friend class Tree<TYPE>;

    TYPE data_       = POISON<TYPE>;
    bool is_dynamic_ = false;

public:

    Node* left_  = nullptr;
    Node* right_ = nullptr;
    Node* prev_  = nullptr;

    size_t depth_ = 0;

//------------------------------------------------------------------------------
/*! @brief   Node default constructor.
*/

    Node ();

//------------------------------------------------------------------------------
/*! @brief   Recursive node destruction.
 *
 *  @note    All nodes must be created by operator new!!!
 */

    ~Node ();

//------------------------------------------------------------------------------
/*! @brief   Safe change node data.
 *
 *  @param   data        Data to change
 */

    void setData (TYPE data);

//------------------------------------------------------------------------------
/*! @brief   Get node data.
 *
 *  @return  node data
 */

    const TYPE& getData ();

//------------------------------------------------------------------------------
/*! @brief   Recursive depth recount.
 */

    void recountDepth ();

//------------------------------------------------------------------------------
/*! @brief   Recursive previous node pointers recount.
 */

    void recountPrev ();

//------------------------------------------------------------------------------
/*! @brief   Node copy constructor.
 *
 *  @param   obj         Source node
 */

    Node (const Node& obj);

    Node& operator = (const Node& obj);

private:

//------------------------------------------------------------------------------
/*! @brief   Create a tree from the base text.
 *
 *  @param   base        Base text
 *  @param   line_cur    Current line in the base text
 * 
 *  @return  error code
 */

    int AddFromBase (const Text& base, size_t& line_cur);

//------------------------------------------------------------------------------
/*! @brief   Recursive tree writing to file.
 *
 *  @param   base        Base file
 */

    void Write (FILE* base);

//------------------------------------------------------------------------------
/*! @brief   Recursively find path to the element.
 *
 *  @param   path        Path to the element
 *  @param   elem        Data of node
 *
 *  @return  1 if found, 0 if not
 */

    int findPath (Stack<size_t>& path, TYPE elem);

//------------------------------------------------------------------------------
/*! @brief   Recursive node checker.
 *
 *  @param   tree        Tree of the node
 *
 *  @return  error code
 */

    int Check (Tree<TYPE>& tree);

//------------------------------------------------------------------------------
/*! @brief   Recursive print the contents of the tree like a graphviz dot file.
 *
 *  @param   dump        Dump graphviz dot file
 */

    void Dump (FILE* dump);

//------------------------------------------------------------------------------
};


template <typename TYPE>
class Tree
{
public:

    char* name_ = nullptr;
    Node<TYPE>* root_ = nullptr;

    int id_ = 0;
    int errCode_ = 0;

    Stack<TYPE> path2badnode_;

public:

//------------------------------------------------------------------------------
/*! @brief   Tree default constructor.
*/

    Tree ();

//------------------------------------------------------------------------------
/*! @brief   Tree constructor with one node.
 *
 *  @param   tree_name   Tree variable name
 */

    Tree (char* tree_name);

//------------------------------------------------------------------------------
/*! @brief   Tree constructor with root.
 *
 *  @param   tree_name   Tree variable name
 *  @param   root        Tree root
 */

    Tree (char* tree_name, Node<TYPE>* root);

//------------------------------------------------------------------------------
/*! @brief   Tree constructor with base.
 *
 *  @param   tree_name   Tree variable name
 *  @param   base_name   Base filename
 */

    Tree (char* tree_name, char* base_name);

//------------------------------------------------------------------------------
/*! @brief   Stack destructor.
 */

   ~Tree ();

//------------------------------------------------------------------------------
/*! @brief   Tree copy constructor.
 *
 *  @param   obj         Source tree
 */

    Tree (const Tree& obj);

    Tree& operator = (const Tree& obj);

//------------------------------------------------------------------------------
/*! @brief   Clean tree.
 */

    void Clean ();

//------------------------------------------------------------------------------
/*! @brief   Print the contents of the tree like a graphviz dot file.
 *
 *  @param   dumpname    Name of the dump file
 */

    void Dump (const char* dumpname = DUMP_NAME);

//------------------------------------------------------------------------------
/*! @brief   Write the tree data to the base file.
 *
 *  @param   basename    Base file name
 */

    void Write (const char* basename = DEFAULT_BASE_NAME);

//------------------------------------------------------------------------------
/*! @brief   Find path in the tree to the element.
 *
 *  @param   path        Path to the element
 *  @param   elem        Data of node
 *
 *  @return  1 if found, 0 if not
 */

    int findPath (Stack<size_t>& path, TYPE elem);

//------------------------------------------------------------------------------
/*! @brief   Check tree for problems.
 *
 *  @return  error code
 */

    int Check ();

//------------------------------------------------------------------------------
/*! @brief   Print error explanations to log file and to console.
 *
 *  @param   logname     Name of the log file
 *  @param   file        Name of the file from which this function was called
 *  @param   line        Line of the code from which this function was called
 *  @param   function    Name of the function from which this function was called
 *  @param   err         Error code
 *  @param   errline     Number of base line with error
 */

    void PrintError (const char* logname, const char* file, int line, const char* function, int err, int errline);

//------------------------------------------------------------------------------
/*! @brief   Prints a section of base text with an error to the console and to the log file.
 *
 *  @param   base        Text base
 *  @param   line        Number of line with an error
 *  @param   logname     Name of the log file
 */

    void PrintBase (Text& base, size_t line, const char* logname);

//------------------------------------------------------------------------------
};

//------------------------------------------------------------------------------

#include "Tree.ipp"

#endif // TREE_H_INCLUDED