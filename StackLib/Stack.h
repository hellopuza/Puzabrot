/*------------------------------------------------------------------------------
    * File:        Stack.h                                                     *
    * Description: Stack library.                                              *
    * Created:     1 dec 2020                                                  *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS


#include "StackConfig.h"
#include <assert.h>
#include <limits.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <new>

#ifdef HASH_PROTECT
#include "hash.h"
#endif // HASH_PROTECT


#define STACK_CHECK if (Check ())                                                                                      \
                    {                                                                                                  \
                      FILE* log = fopen(STACK_LOGNAME, "a");                                                           \
                      assert (log != nullptr);                                                                         \
                      fprintf(log, "ERROR: file %s  line %d  function \"%s\"\n\n", __FILE__, __LINE__, __FUNC_NAME__); \
                      printf (     "ERROR: file %s  line %d  function \"%s\"\n",   __FILE__, __LINE__, __FUNC_NAME__); \
                      fclose(log);                                                                                     \
                      Dump( __FUNC_NAME__, STACK_LOGNAME);                                                             \
                      exit(errCode_);                                                                                  \
                    } //


#define STACK_ASSERTOK(cond, err) if (cond)                                                              \
                                  {                                                                      \
                                    printError (STACK_LOGNAME , __FILE__, __LINE__, __FUNC_NAME__, err); \
                                    exit(err);                                                           \
                                  } //

const size_t DEFAULT_STACK_CAPACITY = 8;
static int   stack_id   = 0;

#define newStack_size(NAME, capacity, STK_TYPE) \
        Stack<STK_TYPE> NAME ((char*)#NAME, capacity);

#define newStack(NAME, STK_TYPE) \
        Stack<STK_TYPE> NAME ((char*)#NAME);


template <typename TYPE>
class Stack
{
private:

    char*   name_     = nullptr;
    size_t  capacity_ = 0;
    size_t  size_cur_ = 0;

    TYPE* data_ = nullptr;

    int id_ = 0;
    int errCode_;

#ifdef HASH_PROTECT
    hash_t stackhash_ = 0;
    hash_t datahash_  = 0;
#endif // HASH_PROTECT

public:

//------------------------------------------------------------------------------
/*! @brief   Stack default constructor.
 */

    Stack ();

//------------------------------------------------------------------------------
/*! @brief   Stack constructor.
 *
 *  @param   stack_name  Stack variable name
 *  @param   capacity    Capacity of the stack
 */

    Stack (char* stack_name, size_t capacity = DEFAULT_STACK_CAPACITY);

//------------------------------------------------------------------------------
/*! @brief   Stack copy constructor.
 *
 *  @param   obj         Source stack
 */

    Stack (const Stack& obj);

    Stack& operator = (const Stack& obj);

//------------------------------------------------------------------------------
/*! @brief   Stack destructor.
 */

   ~Stack ();

//------------------------------------------------------------------------------
/*! @brief   Pushing a value onto the stack.
 *
 *  @param   value       Value to push
 *
 *  @return  error code
 */

    int Push (TYPE value);

//------------------------------------------------------------------------------
/*! @brief   Popping from stack.
 *
 *  @return  value from the stack if present, otherwise POISON
 */

    TYPE Pop ();

//------------------------------------------------------------------------------
/*! @brief   Get size of the stack data.
 *
 *  @return  stack data size
 */

    size_t getSize () const;

//------------------------------------------------------------------------------
/*! @brief   Get name of the stack.
 *
 *  @return  stack name
 */

    const char* getName () const;

//------------------------------------------------------------------------------
/*! @brief   Get name of the stack.
 *
 *  @param   name        Stack name
 */

    void setName (char* name);

    TYPE& operator [] (size_t n);

    const TYPE& operator [] (size_t n) const;

//------------------------------------------------------------------------------
/*! @brief   Clean stack.
 */

    void Clean ();

//------------------------------------------------------------------------------
/*! @brief   Print the contents of the stack and its data to the logfile.
 *
 *  @param   funcname    Name of the function from which the StackDump was called
 *  @param   logname     Name of the logfile
 *
 *  @return  error code
 */

    int Dump (const char* funcname = nullptr, const char* logfile = STACK_LOGNAME);

/*------------------------------------------------------------------------------
                   Private functions                                           *
*///----------------------------------------------------------------------------

private:

//------------------------------------------------------------------------------
/*! @brief   Filling the stack data with POISON.
 */

    void fillPoison ();

//------------------------------------------------------------------------------
/*! @brief   Increase the stack by 2 times.
 *
 *  @return  error code
 */

    int Expand ();

//------------------------------------------------------------------------------
/*! @brief   Check stack for problems and hash (if enabled).
 *
 *  @return  error code
 */

    int Check ();

//------------------------------------------------------------------------------
/*! @brief   Print information and error summary to log file and to console.
 *
 *  @param   fp          Pointer to the logfile
 *
 *  @return  error code
 */

    void ErrorPrint (FILE * fp);

//------------------------------------------------------------------------------
/*! @brief   Calculates the size of the structure stack without hash and second canary.
 *
 *  @return  stack size for hash
 */

#ifdef HASH_PROTECT

    size_t SizeForHash ();

#endif // HASH_PROTECT

//------------------------------------------------------------------------------
};

//------------------------------------------------------------------------------
/*! @brief   Print error explanations to log file and to console.
 *
 *  @param   logname     Name of the log file
 *  @param   file        Name of the file from which this function was called
 *  @param   line        Line of the code from which this function was called
 *  @param   function    Name of the function from which this function was called
 *  @param   err         Error code
 */

static void printError (const char* logname, const char* file, int line, const char* function, int err);

//------------------------------------------------------------------------------

#include "Stack.ipp"

#endif // STACK_H_INCLUDED
