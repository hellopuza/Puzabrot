/*------------------------------------------------------------------------------
    * File:        StringLib.h                                                 *
    * Description: String functions library                                    *
    * Created:     6 nov 2020                                                  *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#ifndef STRINGLIB_H_INCLUDED
#define STRINGLIB_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS


#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
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


//==============================================================================
/*------------------------------------------------------------------------------
                   StringLib errors                                            *
*///----------------------------------------------------------------------------
//==============================================================================


enum StringErrors
{
    STR_NOT_OK = -1                                                    ,
    STR_OK = 0                                                         ,
    STR_NO_MEMORY                                                      ,

    STR_NO_LINES                                                       ,
    STR_NO_SYMB                                                        ,
    STR_NULL_INPUT_BINCODE_FILENAME                                    ,
    STR_NULL_INPUT_BINCODE_PTR                                         ,
    STR_NULL_INPUT_BINCODE_SIZE                                        ,
    STR_NULL_INPUT_TEXT_FILE_NAME                                      ,
    STR_NULL_INPUT_TEXT_LINES_NUM                                      ,
    STR_NULL_INPUT_TEXT_LINES_LEN                                      ,
    STR_NULL_INPUT_TEXT_PTR                                            ,
    STR_BINCODE_DESTRUCTED                                             ,
    STR_BINCODE_NOT_CONSTRUCTED                                        ,
    STR_TEXT_DESTRUCTED                                                ,
    STR_TEXT_NOT_CONSTRUCTED                                           ,
};

static const char* str_errstr[] =
{
    "ERROR"                                                            ,
    "OK"                                                               ,
    "Failed to allocate memory"                                        ,

    "There are no lines with letters in text!"                         ,
    "The file has no any symbols!"                                     ,
    "The input value of the BinCode filename turned out to be zero"    ,
    "The input value of the BinCode pointer turned out to be zero"     ,
    "The input value of the BinCode size turned out to be zero"        ,
    "The input value of the Text file pointer turned out to be zero"   ,
    "The input value of lines Text number turned out to be zero"       ,
    "The input value of lines Text length turned out to be zero"       ,
    "The input value of the Text pointer turned out to be zero"        ,
    "BinCode has already destructed"                                   ,
    "BinCode did not constructed, operation is impossible"             ,
    "Text has already destructed"                                      ,
    "Text did not constructed, operation is impossible"                ,
};

static const char* STRING_LOGNAME = "string.log";

#define STR_ASSERTOK(cond, err)  if (cond)                                                                \
                                 {                                                                        \
                                   StrPrintError(STRING_LOGNAME, __FILE__, __LINE__, __FUNC_NAME__, err); \
                                   exit(err);                                                             \
                                 } //


//==============================================================================
/*------------------------------------------------------------------------------
                   StringLib constants and types                               *
*///----------------------------------------------------------------------------
//==============================================================================


struct Line
{
    char*  str = nullptr;
    size_t len = 0;
};

class Text
{
    int state_;

public:

   char*  text_  = nullptr;
   size_t size_  = 0;
   
   size_t num_   = 0;
   Line*  lines_ = nullptr;

//------------------------------------------------------------------------------
/*! @brief   Text constructor.
 */

    Text ();

//------------------------------------------------------------------------------
/*! @brief   Text constructor from file.
 *
 *  @param   filename    Name of the text file
 */

    Text (const char* filename);

//------------------------------------------------------------------------------
/*! @brief   Text constructor with number of lines and their lengths.
 *
 *  @param   lines_num   Number of lines
 *  @param   line_len    Lengths of lines
 */

    Text (size_t lines_num, size_t line_len);

//------------------------------------------------------------------------------
/*! @brief   Text copy constructor (deleted).
 *
 *  @param   obj         Source text
 */

    Text (const Text& obj);

    Text& operator = (const Text& obj); // deleted

//------------------------------------------------------------------------------
/*! @brief   Text destructor.
 */

   ~Text ();

//------------------------------------------------------------------------------
/*! @brief   Increase the number of text structure lines by 2 times.
 * 
 *  @param   line_len    Length of each line
 * 
 *  @return  error code
 */

    int Expand (size_t line_len);

//------------------------------------------------------------------------------
};


class BinCode
{
    int state_;

public:

    char*  data_ = nullptr;
    size_t size_ = 0;
    size_t ptr_  = 0;

//------------------------------------------------------------------------------
/*! @brief   BinCode constructor.
 */

    BinCode ();

//------------------------------------------------------------------------------
/*! @brief   BinCode constructor with size.
 *
 *  @param   size        Size of the data
 */

    BinCode (size_t size);

//------------------------------------------------------------------------------
/*! @brief   BinCode constructor from file.
 *
 *  @param   filename    Name of the input file
 */

    BinCode (const char* filename);

//------------------------------------------------------------------------------
/*! @brief   BinCode copy constructor (deleted).
 *
 *  @param   obj         Source BinCode
 */

    BinCode (const BinCode& obj);

    BinCode& operator = (const BinCode& obj); // deleted

//------------------------------------------------------------------------------
/*! @brief   BinCode destructor.
 */

   ~BinCode ();

//------------------------------------------------------------------------------
/*! @brief   Increase the binary code data size by 2 times.
 * 
 *  @return  error code
 */

int Expand ();

//------------------------------------------------------------------------------
};



//------------------------------------------------------------------------------
/*! @brief   Get name of a file from command line.
 *
 *  @param   argc        Number of command line arguments
 *  @param   argv        Arguments array
 *
 *  @return  name of the file, else argv[0]
 */

char* GetFileName (int argc, char** argv);

//------------------------------------------------------------------------------
/*! @brief   Get true name of a file (without path to the file and type).
 *
 *  @param   filename    name of the file
 *
 *  @return  true name of the file
 */

char* GetTrueFileName (char* filename);

//------------------------------------------------------------------------------
/*! @brief   Get a size of the file.
 *
 *  @param   fp          Pointer to the file
 *
 *  @return  size of file
 */

size_t CountSize (FILE* fp);

//------------------------------------------------------------------------------
/*! @brief   Get text of the file.
 *
 *  @param   fp          Pointer to the file
 *  @param   len         Length of the text
 *
 *  @return  pointer to text
 */

char* GetText (FILE* fp, size_t len);

//------------------------------------------------------------------------------
/*! @brief   Get number of lines in the text.
 *
 *  @param   text        C string contains text
 *  @param   len         Length of the text
 *
 *  @return  number of lines in the text
 */

size_t GetLineNum (char* text, size_t len);

//------------------------------------------------------------------------------
/*! @brief   Get pointers to start of lines and their lengths.
 *
 *  @param   text        C string contains text
 *  @param   num         Number of lines
 *
 *  @return  array of lines
 */

Line* GetLine (char* text, size_t num);

//------------------------------------------------------------------------------
/*! @brief   Get number of words in string.
 *
 *  @param   line        Pointer to the line structure
 *
 *  @return  number of words
 */

size_t GetWordsNum (Line line);

//------------------------------------------------------------------------------
/*! @brief   Counting characters in string.
 *
 *  @param   str         C string
 *  @param   c           Character to be counted
 *
 *  @return  number of characters
 */

size_t chrcnt (char* str, char c);

//------------------------------------------------------------------------------
/*! @brief   Delete spaces and other non-visible characters in string.
 *
 *  @param   str         C string
 */

void del_spaces (char* str);

//------------------------------------------------------------------------------
/*! @brief   Convert each character to uppercase in string.
 *
 *  @param   str         C string
 */

void str_touppper(char* str);

//------------------------------------------------------------------------------
/*! @brief   Convert each character to lowercase in string.
 *
 *  @param   str         C string
 */

void str_tolower(char* str);

//------------------------------------------------------------------------------
/*! @brief   Compare two lines from left alphabetically using standart strcmp.
 *
 *  @param   p1          Pointer to the first line
 *  @param   p2          Pointer to the second line
 *
 *  @return  positive integer if first line bigger then second
 *  @return  0 if first line the same as second
 *  @return  negative integer if first line smaller then second
 */

int CompareLines (const void *p1, const void *p2);

//------------------------------------------------------------------------------
/*! @brief   Compare two lines from left alphabetically.
 *
 *  @param   p1          Pointer to the first line
 *  @param   p2          Pointer to the second line
 *
 *  @return  positive integer if first line bigger then second
 *  @return  0 if first line the same as second
 *  @return  negative integer if first line smaller then second
 */

int CompareFromLeft (const void *p1, const void *p2);

//------------------------------------------------------------------------------
/*! @brief   Compare two lines from right alphabetically.
 *
 *  @param   p1          Pointer to the first line
 *  @param   p2          Pointer to the second line
 *
 *  @return  positive integer if first line bigger then second
 *  @return  0 if first line the same as second
 *  @return  negative integer if first line smaller then second
 */

int CompareFromRight (const void *p1, const void *p2);

//------------------------------------------------------------------------------
/*! @brief   Copmare two strings by letters.
 *
 *  @param   line1       First line
 *  @param   line2       Second line
 *  @param   dir         Direction of comparing (+1 - compare from left, -1 - compare from right)
 *
 *  @return  positive integer if first line bigger then second
 *  @return  0 if first line the same as second
 *  @return  negative integer if first line smaller then second
 */

int StrCompare (Line line1, Line line2, int dir);

//------------------------------------------------------------------------------
/*! @brief   Write lines to the file.
 *
 *  @param   lines       Array of lines
 *  @param   num         Number of lines
 *  @param   filename    Name of the file
 */

void Write (Line* Lines, size_t num, const char* filename);

//------------------------------------------------------------------------------
/*! @brief   Write text to the file.
 *
 *  @param   text        C string
 *  @param   len         Length of the text
 *  @param   filename    Name of the file
 */

void Print (char* text, size_t len, const char* filename);

//------------------------------------------------------------------------------
/*! @brief   Check that char is letter.
 *
 *  @param   c           Character to be checked
 *
 *  @return  1 if c is letter
 *  @return  0 if c is not letter
 */

int isAlpha (const unsigned char c);

//------------------------------------------------------------------------------
/*! @brief   Prints an error wih description to the console and to the log file.
 * 
 *  @param   logname     Name of the log file
 *  @param   file        Name of the program file
 *  @param   line        Number of line with an error
 *  @param   function    Name of the function with an error
 *  @param   err         Error code
 */

void StrPrintError (const char* logname, const char* file, int line, const char* function, int err);

//------------------------------------------------------------------------------

#endif // STRINGLIB_H_INCLUDED
