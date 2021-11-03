/*------------------------------------------------------------------------------
 * File:        Tree.h                                                         *
 * Description: Tree library.                                                  *
 * Created:     18 apr 2021                                                    *
 * Author:      Artem Puzankov                                                 *
 * Email:       puzankov.ao@phystech.edu                                       *
 * GitHub:      https://github.com/hellopuza                                   *
 * Copyright © 2021 Artem Puzankov. All rights reserved.                       *
 *///---------------------------------------------------------------------------

#ifndef TREE_TREE_H
#define TREE_TREE_H

#include <fstream>
#include <vector>

namespace puza {

template<typename TYPE>
struct Tree final
{
    TYPE                    data;
    std::vector<Tree<TYPE>> branches;

    Tree() = default;
    Tree(TYPE value);
    Tree(const Tree& obj);
    Tree(Tree&& obj) noexcept;
    ~Tree() = default;

    Tree& operator=(const Tree& obj);
    Tree& operator=(Tree&& obj) noexcept;

    bool operator==(const Tree& obj) const;
    bool operator!=(const Tree& obj) const;

    void clear();
    int  dump(const char* dump_name) const;
    int  rdump(const char* dump_name) const;

private:
    void dump(std::ofstream& dump_file) const;
    void rdump(std::ofstream& dump_file) const;
};

} // namespace puza

#endif // TREE_TREE_H