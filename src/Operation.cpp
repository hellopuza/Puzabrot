/*------------------------------------------------------------------------------
 * File:        Operation.cpp                                              *
 * Description: Operations implementation.                                  *
 * Created:     29 oct 2021                                                 *
 * Author:      Artem Puzankov                                              *
 * Email:       puzankov.ao@phystech.edu                                    *
 * GitHub:      https://github.com/hellopuza                                *
 * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
 *///------------------------------------------------------------------------

#include "Calculator/Operation.h"

namespace puza {

Operation::Operation(char code_, std::string word_) : code(code_), word(std::move(word_)) {}

int compare_OpNames(const void* p1, const void* p2)
{
    return (static_cast<const Operation*>(p1))->word.compare((static_cast<const Operation*>(p2))->word);
}

} // namespace puza