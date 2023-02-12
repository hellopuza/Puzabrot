#ifndef TREE_H
#define TREE_H

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

    bool operator==(const Tree& obj) const;
    bool operator!=(const Tree& obj) const;

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

template<typename T>
Tree<T>::Tree(const T& value) : value_(value) {}

template<typename T>
Tree<T>::Tree(T&& value) : value_(std::move(value)) {}

template<typename T>
Tree<T>::Tree(const Tree& obj) : value_(obj.value_), branches_(obj.branches_) {}

template<typename T>
Tree<T>::Tree(Tree&& obj) noexcept : value_(std::move(obj.value_)), branches_(std::move(obj.branches_)) {}

template<typename T>
Tree<T>& Tree<T>::operator=(const Tree& obj)
{
    value_ = obj.value_;
    branches_ = obj.branches_;
    return *this;
}

template<typename T>
Tree<T>& Tree<T>::operator=(Tree&& obj) noexcept
{
    value_ = std::move(obj.value_);
    branches_ = std::move(obj.branches_);
    return *this;
}

template<typename T>
Tree<T>& Tree<T>::operator[](size_t branch_ind)
{
    return branches_[branch_ind];
}

template<typename T>
const Tree<T>& Tree<T>::operator[](size_t branch_ind) const
{
    return branches_[branch_ind];
}

template<typename T>
bool Tree<T>::operator==(const Tree& obj) const
{
    if (value_ != obj.value_)
    {
        return false;
    }

    if (branches_num() != obj.branches_num())
    {
        return false;
    }

    for (size_t i = 0; i < branches_num(); i++)
    {
        if (branches_[i] != obj.branches_[i])
        {
            return false;
        }
    }

    return true;
}

template<typename T>
bool Tree<T>::operator!=(const Tree& obj) const
{
    return !(*this == obj);
}

template<typename T>
size_t Tree<T>::size() const
{
    size_t size = 1;
    for (const auto& node : branches_)
    {
        size += node.size();
    }
    return size;
}

template<typename T>
size_t Tree<T>::branches_num() const
{
    return branches_.size();
}

template<typename T>
void Tree<T>::clear_branches()
{
    branches_.clear();
}

template<typename T>
void Tree<T>::push_branch(const Tree& tree)
{
    branches_.push_back(tree);
}

template<typename T>
void Tree<T>::emplace_branch(Tree&& tree)
{
    branches_.emplace_back(tree);
}

template<typename T>
void Tree<T>::push_branch(const T& value)
{
    branches_.emplace_back(Tree<T>(value));
}

template<typename T>
void Tree<T>::emplace_branch(T&& value)
{
    branches_.emplace_back(Tree<T>(std::move(value)));
}

template<typename T>
void Tree<T>::pop_branch()
{
    branches_.pop_back();
}

template<typename T>
T& Tree<T>::value()
{
    return value_;
}

template<typename T>
const T& Tree<T>::value() const
{
    return value_;
}

template<typename T>
int Tree<T>::dot_dump(const char* dump_name) const
{
    const char* dot_file_name = "graph.dot";

    std::ofstream dump_file(dot_file_name);
    if (!dump_file.is_open())
    {
        return -1;
    }

    dump_file << "digraph G{\n"
        " rankdir = HR;\n"
        " node[shape=box];\n";

    dot_dump(dump_file);

    dump_file << "\tlabelloc=\"t\";"
        "\tlabel=\""
        << dump_name << "\"; }\n";

    dump_file.close();

    char command[1024] = "";
    sprintf(command, "dot -Tpng -o %s.png %s", dump_name, dot_file_name);
    return system(command);
}

template<typename T>
void Tree<T>::dot_dump(std::ofstream& dump_file) const
{
    dump_file << "\t\"" << this << "\"[shape = box, style = filled, color = black, fillcolor = lightskyblue, label = \""
        << value_ << "\"]\n";

    for (const auto& node : branches_)
    {
        dump_file << "\t\"" << this << "\" -> \"" << &node << "\"\n";
    }

    for (const auto& node : branches_)
    {
        node.dot_dump(dump_file);
    }
}

#endif // TREE_H