/*------------------------------------------------------------------------------
    * File:        Types.h                                                     *
    * Description: Functions and constants of different types.                 *
    * Created:     1 mar 2021                                                  *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef TYPES_H
#define TYPES_H

#include <type_traits>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


template<typename TYPE> const TYPE POISON;

    template<> constexpr double             POISON<double>             = NAN;
    template<> constexpr float              POISON<float>              = NAN;
    template<> constexpr unsigned long long POISON<unsigned long long> = ULLONG_MAX;
    template<> constexpr long long          POISON<long long>          = LLONG_MAX;
    template<> constexpr long unsigned int  POISON<long unsigned int>  = ULONG_MAX;
    template<> constexpr unsigned int       POISON<unsigned int>       = UINT_MAX;
    template<> constexpr int                POISON<int>                = INT_MAX;
    template<> constexpr unsigned short     POISON<unsigned short>     = USHRT_MAX;
    template<> constexpr short              POISON<short>              = SHRT_MAX;
    template<> constexpr unsigned char      POISON<unsigned char>      = '\0';
    template<> constexpr char               POISON<char>               = '\0';
    template<> constexpr char*              POISON<char*>              = nullptr;


template<typename TYPE> const char* PRINT_TYPE;

    template<> const char* const PRINT_TYPE<double>             = "double";
    template<> const char* const PRINT_TYPE<float>              = "float";
    template<> const char* const PRINT_TYPE<unsigned long long> = "unsigned long long";
    template<> const char* const PRINT_TYPE<long long>          = "long long";
    template<> const char* const PRINT_TYPE<long unsigned int>  = "long unsigned int";
    template<> const char* const PRINT_TYPE<unsigned int>       = "unsigned int";
    template<> const char* const PRINT_TYPE<int>                = "int";
    template<> const char* const PRINT_TYPE<unsigned short>     = "unsigned short";
    template<> const char* const PRINT_TYPE<short>              = "short";
    template<> const char* const PRINT_TYPE<unsigned char>      = "unsigned char";
    template<> const char* const PRINT_TYPE<char>               = "char";
    template<> const char* const PRINT_TYPE<char*>              = "char*";


template<typename TYPE> const char* const PRINT_FORMAT;

    template<> const char* const PRINT_FORMAT<double>             = "%lf";
    template<> const char* const PRINT_FORMAT<float>              = "%f";
    template<> const char* const PRINT_FORMAT<unsigned long long> = "%llu";
    template<> const char* const PRINT_FORMAT<long long>          = "%lld";
    template<> const char* const PRINT_FORMAT<long unsigned int>  = "%lu";
    template<> const char* const PRINT_FORMAT<unsigned int>       = "%u";
    template<> const char* const PRINT_FORMAT<int>                = "%d";
    template<> const char* const PRINT_FORMAT<unsigned short>     = "%hu";
    template<> const char* const PRINT_FORMAT<short>              = "%hi";
    template<> const char* const PRINT_FORMAT<unsigned char>      = "%c";
    template<> const char* const PRINT_FORMAT<char>               = "%c";
    template<> const char* const PRINT_FORMAT<char*>              = "%s";


//------------------------------------------------------------------------------
/*! @brief   Check if value is POISON.
 *
 *  @param   value       Value to be checked
 *
 *  @return 1 if value is POISON, else 0
 */

template <typename TYPE>
bool isPOISON (TYPE value)
{
    if (value == POISON<TYPE>) return 1;

    if (isnan(*(double*)&POISON<TYPE>))
        if (isnan(*(double*)&value))
            return 1;
        else
            return 0;

    else return (value == POISON<TYPE>);
}

//------------------------------------------------------------------------------
/*! @brief   Print values of any type.
 *
 *  @param   fp          Pointer to output
 *  @param   value       Value to print
 */

template <typename TYPE>
void TypePrint (FILE* fp, const TYPE& value)
{
    fprintf(fp, PRINT_FORMAT<TYPE>, value);
}


#endif // TYPES_H
