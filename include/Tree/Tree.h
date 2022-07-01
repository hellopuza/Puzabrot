#ifndef TREE_TREE_H
#define TREE_TREE_H

#include <fstream>
#include <vector>

template<typename T>
class Tree
{
public:
    Tree() = default;
    explicit Tree(const T& value);
    explicit Tree(T&& value);
    Tree(const Tree& obj);
    Tree(Tree&& obj) noexcept;
    virtual ~Tree() = default;

    Tree& operator=(const Tree& obj);
    Tree& operator=(Tree&& obj) noexcept;
    Tree& operator[](size_t branch_ind);
    const Tree& operator[](size_t branch_ind) const;

    size_t size() const;
    size_t branches_num() const;
    void clear_branches();

    void push_branch(const Tree& tree);
    void emplace_branch(Tree&& tree);
    void push_branch(const T& value);
    void emplace_branch(T&& value);
    void pop_branch();

    T& value();
    const T& value() const;

    int dot_dump(const char* dump_name) const;

protected:
    void dot_dump(std::ofstream& dump_file) const;

    T value_;
    std::vector<Tree<T>> branches_;
};

#endif // TREE_TREE_H