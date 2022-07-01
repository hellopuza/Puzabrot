#include "Calculator/Operation.h"

Operation::Operation(char code_, std::string word_) : code(code_), word(std::move(word_)) {}

int compare_OpNames(const void* p1, const void* p2)
{
    return (static_cast<const Operation*>(p1))->word.compare((static_cast<const Operation*>(p2))->word);
}