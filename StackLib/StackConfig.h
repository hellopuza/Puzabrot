/*------------------------------------------------------------------------------
    * File:        StackConfig.h                                               *
    * Description: Stack congigurations which define different stack types,    *
                   canary, hashes and errors                                   *
    * Created:     1 dec 2020                                                  *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef STACK_CONFIG_H_INCLUDED
#define STACK_CONFIG_H_INCLUDED


#include "../Types.h"
#include <stdlib.h>
#include <time.h>


#if defined (__GNUC__) || defined (__clang__) || defined (__clang_major__)
    #define __FUNC_NAME__   __PRETTY_FUNCTION__
    #define PRINT_PTR       "%p"

#elif defined (_MSC_VER)
    #define __FUNC_NAME__   __FUNCSIG__
    #define PRINT_PTR       "0x%p"

#else
    #define __FUNC_NAME__   __FUNCTION__
    #define PRINT_PTR       "%p"

#endif

#define CONSOLE_PRINT  if(1)

#ifdef  NO_DUMP

    #define DUMP_PRINT if(0)

#else

    #define DUMP_PRINT if(1)

#endif // NO_DUMP

#ifndef NO_HASH

    #define HASH_PROTECT

#endif // NO_HASH


char const * const STACK_LOGNAME = "stack.log";

constexpr size_t MAX_STACK_NUM = 100;
constexpr size_t MAX_CAPACITY  = 100000;


enum StackErrors
{
    STACK_NOT_OK = -1                                               ,
    STACK_OK = 0                                                    ,
    STACK_NO_MEMORY                                                 ,

    STACK_CAPACITY_WRONG_VALUE                                      ,
    STACK_DESTRUCTED                                                ,
    STACK_DESTRUCTOR_REPEATED                                       ,
    STACK_EMPTY_STACK                                               ,
    STACK_INCORRECT_HASH                                            ,
    STACK_MEM_ACCESS_VIOLATION                                      ,
    STACK_NOT_CONSTRUCTED                                           ,
    STACK_NULL_DATA_PTR                                             ,
    STACK_NULL_INPUT_STACK_PTR                                      ,
    STACK_NULL_STACK_PTR                                            ,
    STACK_SIZE_BIGGER_CAPACITY                                      ,
    STACK_TOO_MANY_STACKS                                           ,
    STACK_WRONG_CUR_SIZE                                            ,
    STACK_WRONG_INPUT_CAPACITY_VALUE_BIG                            ,
    STACK_WRONG_INPUT_CAPACITY_VALUE_NIL                            ,
    STACK_WRONG_INPUT_STACK_NAME                                    ,
};

char const * const stk_errstr[] =
{
    "ERROR"                                                         ,
    "OK"                                                            ,
    "Failed to allocate memory"                                     ,

    "Bad size stack capacity"                                       ,
    "Stack already destructed"                                      ,
    "Stack destructor repeated"                                     ,
    "Stack is empty"                                                ,
    "Stack cracked, hash corrupted"                                 ,
    "Memory access violation"                                       ,
    "Stack did not constructed, operation is impossible"            ,
    "The pointer to the stack is null, data lost"                   ,
    "The input value of the stack pointer turned out to be zero"    ,
    "The pointer to the stack is null, stack lost"                  ,
    "The size of the stack data is larger than the capacity"        ,
    "Too many stacks created, cannot create a new one"              ,
    "Current size of stack data is wrong"                           ,
    "Wrong capacity value: - is too big"                            ,
    "Wrong capacity value: - is nil"                                ,
    "Wrong input stack name"                                        ,
};


#endif // STACK_CONFIG_H_INCLUDED
