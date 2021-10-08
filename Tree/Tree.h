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

#include <vector>
#include <fstream>

namespace puza {

template <typename TYPE>
struct Tree
{
    TYPE data;
    std::vector<Tree<TYPE>> branches;

    Tree  ();
    Tree  (TYPE value);
    ~Tree ();

    Tree (const Tree& obj);

    Tree& operator = (const Tree& obj);

    bool operator == (const Tree& obj) const;
    bool operator != (const Tree& obj) const;

    void clear  ();
    int  dump   (const char* dump_name) const;
    int  rdump  (const char* dump_name) const;

private:

    void dump  (std::ofstream& dump_file) const;
    void rdump (std::ofstream& dump_file) const;
};

} // namespace puza

#include "Tree.ipp"

#endif // TREE_H_INCLUDED