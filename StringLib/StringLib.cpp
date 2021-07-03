/*------------------------------------------------------------------------------
    * File:        StringLib.cpp                                               *
    * Description: Implementations of string functions                         *
    * Created:     6 nov 2020                                                  *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#include "StringLib.h"

//------------------------------------------------------------------------------

Text::Text () : state_ (STR_TEXT_NOT_CONSTRUCTED) {}

//------------------------------------------------------------------------------

Text::Text (const char* filename) :
    state_ (STR_OK)
{
    STR_ASSERTOK((filename == nullptr), STR_NULL_INPUT_TEXT_FILE_NAME);

    FILE* fp = nullptr;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("\n ERROR. Input file \"%s\" is not found\n", filename);

        return;
    }

    size_ = CountSize(fp);
    STR_ASSERTOK((size_ == 0), STR_NO_SYMB);

    text_ = GetText(fp, size_);
    STR_ASSERTOK((text_ == nullptr), STR_NO_MEMORY);

    num_ = GetLineNum(text_, size_);
    STR_ASSERTOK((num_ == 0), STR_NO_LINES);

    lines_ = GetLine(text_, num_);
    STR_ASSERTOK((lines_ == nullptr), STR_NO_MEMORY);

    fclose(fp);
}

//------------------------------------------------------------------------------

Text::Text (size_t lines_num, size_t line_len) :
    state_ (STR_OK)
{
    STR_ASSERTOK((lines_num == 0), STR_NULL_INPUT_TEXT_LINES_NUM);
    STR_ASSERTOK((line_len == 0), STR_NULL_INPUT_TEXT_LINES_LEN);

    num_ = lines_num;
    lines_ = (Line*)calloc(num_ + 2, sizeof(Line));
    STR_ASSERTOK((lines_ == nullptr) , STR_NO_MEMORY);

    for (int i = 0; i < num_; ++i)
    {
        lines_[i].len = line_len;
        lines_[i].str = (char*)calloc(line_len, 1);
        STR_ASSERTOK((lines_[i].str == nullptr) , STR_NO_MEMORY);
    }
}

//------------------------------------------------------------------------------

Text::~Text ()
{
    STR_ASSERTOK((this == nullptr), STR_NULL_INPUT_TEXT_PTR);

    if ((state_ != STR_TEXT_DESTRUCTED) && (state_ != STR_TEXT_NOT_CONSTRUCTED))
    {
        if (num_ != 0)
        {
            assert(lines_ != nullptr);
            free(lines_);
            lines_ = nullptr;
            num_   = 0;
        }

        if (size_ != 0)
        {
            assert(text_ != nullptr);
            free(text_);
            text_ = nullptr;
            size_ = 0;
        }

        state_ = STR_TEXT_DESTRUCTED;
    }
}

//------------------------------------------------------------------------------

int Text::Expand (size_t line_len)
{
    STR_ASSERTOK((this == nullptr), STR_NULL_INPUT_TEXT_PTR);
    STR_ASSERTOK(state_, state_);

    num_ *= 2;

    void* temp = calloc(num_ + 2, sizeof(Line));
    if (temp == nullptr)
        return STR_NO_MEMORY;

    void* oldtemp = lines_;

    memcpy(temp, lines_, num_ * sizeof(Line) / 2);
    free(oldtemp);

    lines_ = (Line*)temp;

    for (int i = num_ / 2; i < num_; ++i)
    {
        lines_[i].len = line_len;
        lines_[i].str = (char*)calloc(line_len, 1);
        STR_ASSERTOK((lines_[i].str == nullptr) , STR_NO_MEMORY);
    }

    return STR_OK;
}

//------------------------------------------------------------------------------

BinCode::BinCode () : state_ (STR_BINCODE_NOT_CONSTRUCTED) {}

//------------------------------------------------------------------------------

BinCode::BinCode (size_t size) :
    state_ (STR_OK)
{
    STR_ASSERTOK((this == nullptr), STR_NULL_INPUT_BINCODE_PTR);
    STR_ASSERTOK((size == 0),       STR_NULL_INPUT_BINCODE_SIZE);

    data_ = (char*)calloc(size + 2, 1);
    STR_ASSERTOK((data_ == nullptr) , STR_NO_MEMORY);

    ptr_ = 0;
    size_ = size;
}

//------------------------------------------------------------------------------

BinCode::BinCode (const char* filename) :
    state_ (STR_OK)
{
    STR_ASSERTOK((this == nullptr),     STR_NULL_INPUT_BINCODE_PTR);
    STR_ASSERTOK((filename == nullptr), STR_NULL_INPUT_BINCODE_FILENAME);

    FILE* fp = nullptr;
    if ((fp = fopen(filename, "rb")) == NULL)
    {
        printf("\n ERROR. Input file \"%s\" is not found\n", filename);

        return;
    }

    size_ = CountSize(fp);
    STR_ASSERTOK((size_ == 0) , STR_NO_MEMORY);

    data_ = GetText(fp, size_);
    STR_ASSERTOK((data_ == nullptr) , STR_NO_MEMORY);

    fclose(fp);

    ptr_ = 0;
}

//------------------------------------------------------------------------------

BinCode::~BinCode ()
{
    STR_ASSERTOK((this == nullptr), STR_NULL_INPUT_BINCODE_PTR);

    if ((state_ != STR_BINCODE_DESTRUCTED) && (state_ != STR_BINCODE_NOT_CONSTRUCTED))
    {
        if (size_ != 0)
        {
            free(data_);
            ptr_  = 0;
            size_ = 0;
        }

        state_ = STR_BINCODE_DESTRUCTED;
    }
}

//------------------------------------------------------------------------------

int BinCode::Expand ()
{
    STR_ASSERTOK((this == nullptr), STR_NULL_INPUT_BINCODE_PTR);
    STR_ASSERTOK(state_, state_);

    size_ *= 2;

    void* temp = calloc(size_ + 2, 1);
    if (temp == nullptr)
        return STR_NO_MEMORY;

    void* oldtemp = data_;
    memcpy(temp, data_, size_ / 2);
    free(oldtemp);

    data_ = (char*)temp;

    return STR_OK;
}

//------------------------------------------------------------------------------

char* GetFileName (int argc, char** argv)
{
    assert(argc);
    assert(argv != nullptr);

    if (argc > 1)
    {
        return argv[1];
    }

    return argv[0];
}

//------------------------------------------------------------------------------

char* GetTrueFileName (char* filename)
{
    assert(filename != nullptr);

    int ptr_end = strlen(filename) - 1;

    for (int i = ptr_end; i > -1; --i)
    {
        if (filename[i] == '.')
            filename[i] = '\0';
        else
        if ((filename[i] == '/') ||
            (filename[i] == '\\'))
            return filename + i + 1;
    }

    return filename;
}

//------------------------------------------------------------------------------

size_t CountSize (FILE* fp)
{
    assert(fp != nullptr);

    struct stat prop;
#ifdef _MSC_VER
    fstat(_fileno(fp), &prop);
#else
    fstat(fileno(fp), &prop);
#endif

    return prop.st_size;
}

//------------------------------------------------------------------------------

char* GetText (FILE* fp, size_t len)
{
    assert(fp != nullptr);
    assert(len);

    char* text = (char*)calloc(len + 2, 1);
    if (text == nullptr)
        return nullptr;

    int err = fread(text, 1, len, fp);

    return text;
}

//------------------------------------------------------------------------------

size_t GetLineNum (char* text, size_t len)
{
    assert(text != nullptr);
    assert(len);

    char* start = text;

    size_t num = 0;

    while (text - start <= len)
    {
        ++num;

        text = strchr(text, '\n') + 1;
        if (text == (char*)1)
            break;
    }

    return num;
}

//------------------------------------------------------------------------------

Line* GetLine (char* text, size_t num)
{
    assert(text != nullptr);
    assert(num);

    Line* Lines = (Line*)calloc(num + 2, sizeof(Line));
    if (Lines == nullptr)
        return nullptr;

    Line* temp1 = Lines;

    while (num-- > 0)
    {
        while (isspace(*text) && (*text != '\n'))
            ++text;

        char* start = text;
        text = strchr(text, '\n');

        if (text != 0) *text = '\0';

        temp1->str = (char*)start;
        temp1->len = strlen(start);

        ++temp1;
        ++text;
    }

    return Lines;
}

//------------------------------------------------------------------------------

size_t GetWordsNum (Line line)
{
    assert(line.str != nullptr);

    int num = 0;
    char f  = 0;
    for (int i = 0; i <= line.len; ++i)
    {
        char c = *(line.str + i);

        if (isgraph(c))
            f = 1;
        else
            if ((f == 1) && (isspace(c) || (c == '\0')))
            {
                f = 0;
                ++num;
            }
    }

    return num;
}

//------------------------------------------------------------------------------

size_t chrcnt (char* str, char c)
{
    assert(str != nullptr);

    size_t count = 0;

    str = strchr(str, c);
    while (str != NULL)
    {
        ++count;
        str = strchr(str + 1, c);
        if (str == 0)
            break;
    }

    return count;
}

//------------------------------------------------------------------------------

void del_spaces (char* str)
{
    char* to_write = str;
    char* to_check = str;

    while (*to_check != '\0')
    {
        if (not isspace(*to_check))
        {
            *to_write = *to_check;
            ++to_write;
        }

        ++to_check;
    }

    *to_write = '\0';
}

//------------------------------------------------------------------------------

void str_touppper(char* str)
{
    while (*str != '\0')
    {
        *str = toupper(*str);
        ++str;
    }
}

//------------------------------------------------------------------------------

void str_tolower(char* str)
{
    while (*str != '\0')
    {
        *str = tolower(*str);
        ++str;
    }
}

//------------------------------------------------------------------------------

int CompareLines (const void* p1, const void* p2)
{
    assert(p1 != nullptr);
    assert(p2 != nullptr);
    assert(p1 != p2);

    return strcmp(((Line*)p1)->str, ((Line*)p2)->str);
}

//------------------------------------------------------------------------------

int CompareFromLeft (const void* p1, const void* p2)
{
    assert(p1 != nullptr);
    assert(p2 != nullptr);
    assert(p1 != p2);

    return StrCompare(*(Line*)p1, *(Line*)p2, 1);
}

//------------------------------------------------------------------------------

int CompareFromRight (const void* p1, const void* p2)
{
    assert(p1 != nullptr);
    assert(p2 != nullptr);
    assert(p1 != p2);

    return StrCompare(*(Line*)p1, *(Line*)p2, -1);
}

//------------------------------------------------------------------------------

int StrCompare (Line line1, Line line2, int dir)
{
    assert((dir == 1) || (dir == -1));

    int i1 = 0;
    int i2 = 0;

    if (dir == -1)
    {
        i1 = line1.len - 1;
        i2 = line2.len - 1;
    }

    while ((line1.str[i1] != '\0') && (line2.str[i2] != '\0'))
    {
        if (not(isAlpha(line1.str[i1])))
        {
            i1 += dir;
            continue;
        }

        if (not(isAlpha(line2.str[i2])))
        {
            i2 += dir;
            continue;
        }

        if ((unsigned char)line1.str[i1] == (unsigned char)line2.str[i2])
        {
            i1 += dir;
            i2 += dir;
            continue;
        }

        else return ((unsigned char)line1.str[i1] - (unsigned char)line2.str[i2]);
    }

    if (dir == 1)
        return ((unsigned char)line1.str[i1] - (unsigned char)line2.str[i2]);
    else
        return ((unsigned char)line1.str[i2] - (unsigned char)line2.str[i1]);
}

//------------------------------------------------------------------------------

int isAlpha (const unsigned char c)
{
    return (   ((unsigned char)'a' <= c) && (c <= (unsigned char)'z')
            || ((unsigned char)'A' <= c) && (c <= (unsigned char)'Z')
            || ((unsigned char)'а' <= c) && (c <= (unsigned char)'я')
            || ((unsigned char)'А' <= c) && (c <= (unsigned char)'Я'));
}

//------------------------------------------------------------------------------

void Write (Line* lines, size_t num, const char* filename)
{
    assert(lines != nullptr);
    assert(num);
    assert(filename);
    printf("file: [%s]\n", filename);

    FILE* fp = fopen(filename, "w");

    for (int i = 0; i < num; ++i)
        fprintf(fp, "%s\n", lines[i].str);

    fclose(fp);
}

//------------------------------------------------------------------------------

void Print (char* text, size_t len, const char* filename)
{
    assert(text != nullptr);
    assert(len);
    assert(filename);

    FILE* fp = fopen(filename, "w");

    for (int i = 0; i < len; ++i)
        fputc(text[i], fp);

    fclose(fp);
}

//------------------------------------------------------------------------------

void StrPrintError (const char* logname, const char* file, int line, const char* function, int err)
{
    assert(function != nullptr);
    assert(logname != nullptr);
    assert(file != nullptr);

    FILE* log = fopen(logname, "a");
    assert(log != nullptr);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(log, "###############################################################################\n");
    fprintf(log, "TIME: %d-%02d-%02d %02d:%02d:%02d\n\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(log, "ERROR: file %s  line %d  function %s\n\n", file, line, function);
    fprintf(log, "%s\n", str_errstr[err + 1]);

    printf (     "ERROR: file %s  line %d  function %s\n",   file, line, function);
    printf (     "%s\n\n", str_errstr[err + 1]);

    fclose(log);
}

//------------------------------------------------------------------------------
