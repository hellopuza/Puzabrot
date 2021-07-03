/*------------------------------------------------------------------------------
    * File:        hash.cpp                                                    *
    * Description: Functions to compute hash message digest of files or memory *
                   blocks.                                                     *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#include "hash.h"

//------------------------------------------------------------------------------

int bit_rotate(void* buf, size_t size, int dir)
{
    assert(buf != nullptr);

    if ((size == 0) || (dir == 0))
        return 0;

    char* carry_bits = (char*)calloc(size, 1);
    if (carry_bits == nullptr)
        return 0;

    if (dir > 0)
    {
        dir = dir % (size * 8);
        for (int i = 0; i < dir; ++i)
        {
            for (int byte_i = 0; byte_i < size; ++byte_i)
            {
                carry_bits[byte_i] = *((char*)buf + byte_i) & 1;
            }

            for (int byte_i = 0; byte_i < size - 1; ++byte_i)
            {
                char byte = *((char*)buf + byte_i);
                byte = byte >> 1;
                byte = (byte & 127) | (128 * carry_bits[byte_i + 1]);

                ((char*)buf)[byte_i] = byte;
            }

            char byte = *((char*)buf + size - 1);
            byte = byte >> 1;
            byte = (byte & 127) | (128 * carry_bits[0]);

            ((char*)buf)[size - 1] = byte;
        }
    }
    else
    {
        dir = -dir % (size * 8);

        for (int i = 0; i < dir; ++i)
        {
            for (int byte_i = 0; byte_i < size; ++byte_i)
            {
                carry_bits[byte_i] = *((char*)buf + byte_i) & 128;
            }

            for (int byte_i = size - 1; byte_i > 0; --byte_i)
            {
                char byte = *((char*)buf + byte_i);
                byte = byte << 1;
                byte = (byte & 254) | (carry_bits[byte_i - 1] / 128);

                ((char*)buf)[byte_i] = byte;
            }

            char byte = *((char*)buf);
            byte = byte << 1;
            byte = (byte & 254) | (carry_bits[size - 1] / 128);

            ((char*)buf)[0] = byte;
        }
    }

    free(carry_bits);

    return 1;
}

//------------------------------------------------------------------------------

hash_t hash(void* buf, size_t size)
{
    assert(buf != nullptr);

    size_t bm_size = (size / BLOCK_SIZE) * BLOCK_SIZE + BLOCK_SIZE;
    size_t mid_size = bm_size / 2;

    char* buf_main = (char*)calloc(bm_size, 1);
    if (buf_main == nullptr)
        return 0;

    memcpy(buf_main, buf, size);

    ((size_t*)buf_main)[bm_size/sizeof(size_t) - 1] += size;

    hash_t hsh = (hash_t)(Keys[size % KEYS_NUM]);

    for (size_t byte_i = 0; byte_i < mid_size; ++byte_i)
    {
        char b1 = *((char*)buf_main + byte_i);
        char b2 = *((char*)buf_main + byte_i + mid_size);

        char p1 = b1;
        char p2 = b2;

        bit_rotate(&p1, 1, 1 + byte_i);
        bit_rotate(&p2, 1, 1 - byte_i);

        int q1 = b2 ^ p1 ^ Keys[ byte_i      % KEYS_NUM] + b1;
        int q2 = b1 ^ p2 ^ Keys[(byte_i + 1) % KEYS_NUM] + b2;

        b1 = *((char*)buf_main - 1 - byte_i + bm_size);
        b2 = *((char*)buf_main - 1 - byte_i + mid_size);

        hsh = hsh * (q1*b2 + q2*b1) + hsh ^ (q1*b1 + q2*b2) + q1 + q2 + b1 + b2;

        bit_rotate(&hsh, sizeof(hsh), 3);
    }
    free(buf_main);

    return hsh;
}

//------------------------------------------------------------------------------
