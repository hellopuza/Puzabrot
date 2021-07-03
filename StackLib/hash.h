/*------------------------------------------------------------------------------
    * File:        hash.h                                                      *
    * Description: Declaration of functions and data types used for hash sum   *
                   computing library functions.                                *
    * Created:     1 dec 2020                                                  *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS
//#define NDEBUG

#include <assert.h>
#include <limits.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


typedef unsigned long long hash_t;

#define HASH_SIZE sizeof(hash_t)
#define MAX_HASH  ULLONG_MAX

#define HASH_PRINT_FORMAT "0x%016llX"

static const size_t BLOCK_SIZE = 64;
static const size_t KEYS_NUM   = 16;

static const size_t Keys[KEYS_NUM] =
{
    0x26964da6, 0x69b25a6d, 0x9b4d9693, 0x64d26d2c,
    0x4b65a6c9, 0x9a592d36, 0xa4da6cb4, 0x4b2696c9,
    0xd36934b6, 0x369b2d92, 0x6cb4da59, 0x4b65a6d2,
    0xda592d93, 0x2696c964, 0xb26d365b, 0x25936934,
};

//------------------------------------------------------------------------------
/*! @brief   Circular shift of bits anywhere in any length.
 *
 *  @param   buf  Start of memory for turning round
 *  @param   size Size of memory for turning round
 *  @param   dir  Direction of turning, if >0 - right, if <0 - left
 * 
 *  @return 0 if error, 1 if ok
 */

int bit_rotate (void* buf, size_t size, int dir);

//------------------------------------------------------------------------------
/*! @brief   Hash counting.
 *
 *  @param   buf  Start of memory to be hashable
 *  @param   size Size of memory to be hashable
 *
 *  @return  0 if error, else hash
 */

hash_t hash (void* buf, size_t size);

//------------------------------------------------------------------------------

#endif // HASH_H_INCLUDED
