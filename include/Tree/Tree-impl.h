/*------------------------------------------------------------------------------
 * File:        Tree-impl.h                                                    *
 * Description: Tree library implementation.                                   *
 * Created:     18 apr 2021                                                    *
 * Author:      Artem Puzankov                                                 *
 * Email:       puzankov.ao@phystech.edu                                       *
 * GitHub:      https://github.com/hellopuza                                   *
 * Copyright © 2021 Artem Puzankov. All rights reserved.                       *
 *///---------------------------------------------------------------------------

#ifndef TREE_TREE_IMPL_H
#define TREE_TREE_IMPL_H

#include "Tree/Tree.h"

namespace puza {

template<typename TYPE>
Tree<TYPE>::Tree(TYPE value) : data(value)
{}

template<typename TYPE>
Tree<TYPE>::Tree(const Tree& obj) : data(obj.data), branches(obj.branches)
{}

template<typename TYPE>
Tree<TYPE>::Tree(Tree&& obj) noexcept : data(std::move(obj.data)), branches(std::move(obj.branches))
{}

template<typename TYPE>
Tree<TYPE>& Tree<TYPE>::operator=(const Tree& obj)
{
    data     = obj.data;
    branches = obj.branches;
    return *this;
}

template<typename TYPE>
Tree<TYPE>& Tree<TYPE>::operator=(Tree&& obj) noexcept
{
    data     = std::move(obj.data);
    branches = std::move(obj.branches);
    return *this;
}

template<typename TYPE>
bool Tree<TYPE>::operator==(const Tree& obj) const
{
    return (data == obj.data) && (branches == obj.branches);
}

template<typename TYPE>
bool Tree<TYPE>::operator!=(const Tree& obj) const
{
    return !(*this == obj);
}

template<typename TYPE>
void Tree<TYPE>::clear()
{
    branches.clear();
}

template<typename TYPE>
int Tree<TYPE>::dump(const char* dump_name) const
{
    if (dump_name == nullptr)
        return -1;
    const char* const DOT_FILE_NAME = "graph.dot";

    std::ofstream dump_file(DOT_FILE_NAME);
    if (!dump_file.is_open())
        return -1;

    dump_file << "digraph G{\n"
                 " rankdir = HR;\n"
                 " node[shape=box];\n";

    dump(dump_file);

    dump_file << "\tlabelloc=\"t\";"
                 "\tlabel=\""
              << dump_name << "\"; }\n";

    dump_file.close();

    char command[1024] = "";

    sprintf(command, "dot -Tpng -o %s.png %s", dump_name, DOT_FILE_NAME);
    return system(command);
}

template<typename TYPE>
int Tree<TYPE>::rdump(const char* dump_name) const
{
    if (dump_name == nullptr)
        return -1;
    const char* const DOT_FILE_NAME = "graph.dot";

    std::ofstream dump_file(DOT_FILE_NAME);
    if (!dump_file.is_open())
        return -1;

    dump_file << "digraph G{\n"
                 " rankdir = HR;\n"
                 " node[shape=box];\n";

    rdump(dump_file);

    dump_file << "\tlabelloc=\"t\";"
                 "\tlabel=\""
              << dump_name << "\"; }\n";

    dump_file.close();

    char command[1024] = "";

    sprintf(command, "dot -Tpng -o %s.png %s", dump_name, DOT_FILE_NAME);
    return system(command);
}

template<typename TYPE>
void Tree<TYPE>::dump(std::ofstream& dump_file) const
{
    dump_file << "\t\"" << this << "\"[shape = box, style = filled, color = black, fillcolor = lightskyblue, label = \""
              << data << "\"]\n";

    for (auto node = branches.begin(); node != branches.end(); ++node)
    {
        dump_file << "\t\"" << this << "\" -> \"" << &*node << "\"\n";
    }

    for (auto node = branches.begin(); node != branches.end(); ++node) { node->dump(dump_file); }
}

template<typename TYPE>
void Tree<TYPE>::rdump(std::ofstream& dump_file) const
{
    dump_file << "\t\"" << this << "\"[shape = box, style = filled, color = black, fillcolor = lightskyblue, label = \""
              << data << "\"]\n";

    for (auto node = branches.rbegin(); node != branches.rend(); ++node)
    {
        dump_file << "\t\"" << this << "\" -> \"" << &*node << "\"\n";
    }

    for (auto node = branches.rbegin(); node != branches.rend(); ++node) { node->rdump(dump_file); }
}

} // namespace puza

#endif // TREE_TREE_IMPL_H