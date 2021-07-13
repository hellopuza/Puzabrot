/*------------------------------------------------------------------------------
    * File:        Stack.ipp                                                   *
    * Description: Implementations of stack functions.                         *
    * Created:     1 dec 2020                                                  *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

template <typename TYPE>
Stack<TYPE>::Stack () : errCode_ (STACK_NOT_CONSTRUCTED) { }

//------------------------------------------------------------------------------

template <typename TYPE>
Stack<TYPE>::Stack (char* stack_name, size_t capacity) :
    data_     (),
    size_cur_ (0),
    capacity_ (capacity),
    name_     (stack_name),
    id_       (stack_id++),
    errCode_  (STACK_OK)
{
    STACK_ASSERTOK((capacity > MAX_CAPACITY),   STACK_WRONG_INPUT_CAPACITY_VALUE_BIG);
    STACK_ASSERTOK((capacity == 0),             STACK_WRONG_INPUT_CAPACITY_VALUE_NIL);
    STACK_ASSERTOK((stack_name == nullptr),     STACK_WRONG_INPUT_STACK_NAME);
    
    try
    {
        data_ = new TYPE[capacity_];
    }
    catch (std::bad_alloc& err)
    {
        STACK_ASSERTOK(STACK_NO_MEMORY, STACK_NO_MEMORY);
    }

    fillPoison();

#ifdef HASH_PROTECT
    datahash_  = hash(data_, capacity_ * sizeof(TYPE));
    stackhash_ = hash(this, SizeForHash());
#endif // HASH_PROTECT

    STACK_CHECK;

    DUMP_PRINT{ Dump(__FUNC_NAME__); }
}

//------------------------------------------------------------------------------

template <typename TYPE>
Stack<TYPE>::Stack (const Stack& obj) :
    size_cur_ (obj.size_cur_),
    capacity_ (obj.capacity_),
    id_       (stack_id++),
    errCode_  (STACK_OK)
{
    STACK_ASSERTOK((capacity_ > MAX_CAPACITY),  STACK_WRONG_INPUT_CAPACITY_VALUE_BIG);
    STACK_ASSERTOK((capacity_ == 0),            STACK_WRONG_INPUT_CAPACITY_VALUE_NIL);

    try
    {
        data_ = new TYPE[capacity_];
    }
    catch (std::bad_alloc& err)
    {
        STACK_ASSERTOK(STACK_NO_MEMORY, STACK_NO_MEMORY);
    }

    for (int i = 0; i < capacity_; ++i) copyType(data_[i], obj.data_[i]);

#ifdef HASH_PROTECT
    datahash_  = hash(data_, capacity_ * sizeof(TYPE));
    stackhash_ = hash(this, SizeForHash());
#endif // HASH_PROTECT

    STACK_CHECK;

    DUMP_PRINT{ Dump(__FUNC_NAME__); }
}

//------------------------------------------------------------------------------

template <typename TYPE>
Stack<TYPE>& Stack<TYPE>::operator = (const Stack& obj)
{
    STACK_ASSERTOK((obj.capacity_ > MAX_CAPACITY), STACK_WRONG_INPUT_CAPACITY_VALUE_BIG);
    STACK_ASSERTOK((obj.capacity_ == 0),           STACK_WRONG_INPUT_CAPACITY_VALUE_NIL);

    size_cur_ = obj.size_cur_;
    capacity_ = obj.capacity_;
    errCode_  = STACK_OK;

    try
    {
        data_ = new TYPE[capacity_];
    }
    catch (std::bad_alloc& err)
    {
        STACK_ASSERTOK(STACK_NO_MEMORY, STACK_NO_MEMORY);
    }

    for (int i = 0; i < capacity_; ++i) copyType(data_[i], obj.data_[i]);

#ifdef HASH_PROTECT
    datahash_  = hash(data_, capacity_ * sizeof(TYPE));
    stackhash_ = hash(this, SizeForHash());
#endif // HASH_PROTECT

    STACK_CHECK;

    DUMP_PRINT{ Dump(__FUNC_NAME__); }

    return *this;
}

//------------------------------------------------------------------------------

template <typename TYPE>
Stack<TYPE>::~Stack ()
{
    if (errCode_ == STACK_NOT_CONSTRUCTED) return;

    DUMP_PRINT{ Dump (__FUNC_NAME__); }

    if (errCode_ != STACK_DESTRUCTED)
    {
        size_cur_ = 0;

        fillPoison();

        delete [] data_;
        data_  = nullptr;

        capacity_ = 0;

        #ifdef HASH_PROTECT
            datahash_  = 0;
            stackhash_ = 0;
        #endif // HASH_PROTECT

        errCode_ = STACK_DESTRUCTED;
    }
    else
    {
        STACK_ASSERTOK(STACK_DESTRUCTOR_REPEATED, STACK_DESTRUCTOR_REPEATED);
    }
}

//------------------------------------------------------------------------------

template <typename TYPE>
int Stack<TYPE>::Push (TYPE value)
{
    STACK_CHECK;

    if (size_cur_ == capacity_ - 1) Expand();

    data_[size_cur_++] = value;

#ifdef HASH_PROTECT
    datahash_  = hash(data_, capacity_ * sizeof(TYPE));
    stackhash_ = hash(this, SizeForHash());
#endif // HASH_PROTECT

    STACK_CHECK;

    DUMP_PRINT{ Dump (__FUNC_NAME__); }

    return STACK_OK;
}

//------------------------------------------------------------------------------

template <typename TYPE>
TYPE Stack<TYPE>::Pop ()
{
    STACK_CHECK;

    if (size_cur_ == 0) errCode_ = STACK_EMPTY_STACK;

    if (errCode_ == STACK_EMPTY_STACK)
    {
        DUMP_PRINT{ Dump (__FUNC_NAME__); }

        #ifdef HASH_PROTECT
            datahash_  = hash(data_, capacity_ * sizeof(TYPE));
            stackhash_ = hash(this, SizeForHash());
        #endif // HASH_PROTECT

        return POISON<TYPE>;
    }

    TYPE value = data_[--size_cur_];

    data_[size_cur_] = POISON<TYPE>;

#ifdef HASH_PROTECT
    datahash_  = hash(data_, capacity_ * sizeof(TYPE));
    stackhash_ = hash(this, SizeForHash());
#endif // HASH_PROTECT

    STACK_CHECK;

    DUMP_PRINT{ Dump (__FUNC_NAME__); }

    return value;
}

//------------------------------------------------------------------------------

template <typename TYPE>
void Stack<TYPE>::Clean ()
{
    STACK_CHECK;

    size_cur_ = 0;
    fillPoison();
    delete [] data_;

    capacity_ = DEFAULT_STACK_CAPACITY;

    try
    {
        data_ = new TYPE[capacity_];
    }
    catch (std::bad_alloc& err)
    {
        STACK_ASSERTOK(STACK_NO_MEMORY, STACK_NO_MEMORY);
    }

    fillPoison();

#ifdef HASH_PROTECT
    datahash_  = hash(data_, capacity_ * sizeof(TYPE));
    stackhash_ = hash(this, SizeForHash());
#endif // HASH_PROTECT

    STACK_CHECK;

    DUMP_PRINT{ Dump (__FUNC_NAME__); }
}

//------------------------------------------------------------------------------

template <typename TYPE>
size_t Stack<TYPE>::getSize () const
{
    return size_cur_;
}

//------------------------------------------------------------------------------

template <typename TYPE>
const char* Stack<TYPE>::getName () const
{
    return name_;
}

//------------------------------------------------------------------------------

template <typename TYPE>
void Stack<TYPE>::setName (char* name)
{
    name_ = name;
}

//------------------------------------------------------------------------------

template <typename TYPE>
TYPE& Stack<TYPE>::operator [] (size_t n)
{
    STACK_ASSERTOK((n >= capacity_), STACK_MEM_ACCESS_VIOLATION);

    return data_[n];
}

//------------------------------------------------------------------------------

template <typename TYPE>
const TYPE& Stack<TYPE>::operator [] (size_t n) const
{
    STACK_ASSERTOK((n >= capacity_), STACK_MEM_ACCESS_VIOLATION);
    
    return data_[n];
}

//------------------------------------------------------------------------------

template <typename TYPE>
void Stack<TYPE>::fillPoison ()
{
    assert(this     != nullptr);
    assert(data_    != nullptr);
    assert(size_cur_ < capacity_);

    for (int i = size_cur_; i < capacity_; ++i)
    {
        data_[i] = POISON<TYPE>;
    }
}

//------------------------------------------------------------------------------

template <typename TYPE>
int Stack<TYPE>::Expand ()
{
    assert(this != nullptr);

    capacity_ *= 2;

    TYPE* temp = nullptr;
    try
    {
        temp = new TYPE[capacity_];
    }
    catch (std::bad_alloc& err)
    {
        STACK_ASSERTOK(STACK_NO_MEMORY, STACK_NO_MEMORY);
    }

    memcpy(temp, (char*)data_, capacity_ * sizeof(TYPE) / 2);

    delete [] data_;
    data_ = temp;

    fillPoison();

    return STACK_OK;
}

//------------------------------------------------------------------------------

template <typename TYPE>
int Stack<TYPE>::Dump (const char* funcname, const char* logfile)
{
    const size_t linelen = 80;
    char divline[linelen + 1] = "********************************************************************************";

    FILE* fp = stdout;
    if (funcname != nullptr)
    {
        fp = fopen(logfile, "a");
        if (fp == nullptr)
            return STACK_NOT_OK;

        if (funcname != nullptr)
            fprintf(fp, "This dump was called from a function \"%s\"\n", funcname);

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(fp, "TIME: %d-%02d-%02d %02d:%02d:%02d\n\n",
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec);
    }

    if ((errCode_ == STACK_NOT_CONSTRUCTED)      ||
        (errCode_ == STACK_DESTRUCTED)           ||
        (errCode_ == STACK_NULL_DATA_PTR)        ||
        (errCode_ == STACK_SIZE_BIGGER_CAPACITY) ||
        (errCode_ == STACK_CAPACITY_WRONG_VALUE)   )
    {
        fprintf(fp, "\nStack (ERROR) [" PRINT_PTR "] \"%s\" id (%d)\n", this, name_, id_);
        ErrorPrint(fp);

        fprintf(fp, "%s\n", divline);
        if (fp != stdout) fclose(fp);

        return STACK_OK;
    }

    char* StkState = (char*)stk_errstr[STACK_OK + 1];

    if (errCode_) ErrorPrint(fp);

    fprintf(fp, "\nStack (%s) [" PRINT_PTR "] \"%s\", id (%d)\n", StkState, this, name_, id_);

    fprintf(fp, "\t{\n");

    fprintf(fp, "\tType of data is %s\n\n", PRINT_TYPE<TYPE>);

    fprintf(fp, "\tCapacity           = %lu\n",   capacity_);
    fprintf(fp, "\tCurrent size       = %lu\n\n", size_cur_);

#ifdef HASH_PROTECT
    fprintf(fp, "\tStack hash         = " HASH_PRINT_FORMAT "\n",   stackhash_);
    fprintf(fp, "\tData hash          = " HASH_PRINT_FORMAT "\n\n", datahash_);

    if ((errCode_ != STACK_OK) && (errCode_ != STACK_EMPTY_STACK) && (errCode_ != STACK_NO_MEMORY))
    {
        fprintf(fp, "\tTrue stack hash    = " HASH_PRINT_FORMAT "\n",   hash(this, SizeForHash()));
        fprintf(fp, "\tTrue data hash     = " HASH_PRINT_FORMAT "\n\n", hash(data_, capacity_ * sizeof(TYPE)));
    }
#endif // HASH_PROTECT

    fprintf(fp, "\tData [" PRINT_PTR "]\n", data_);

    fprintf(fp, "\t\t{\n");

    for (int i = 0; i < capacity_; i++)
    {
        char ispois = isPOISON(data_[i]);

        fprintf(fp, "\t\t%s[%d]: [", (ispois) ? " ": "*", i);
        TypePrint(fp, data_[i]);
        fprintf(fp, "]%s\n", (ispois) ? " (POISON)": "");
    }

    fprintf(fp, "\t\t}\n");

    fprintf(fp, "\t}\n");

    fprintf(fp, "%s\n", divline);
    if (fp != stdout) fclose(fp);

    return STACK_OK;
}

//------------------------------------------------------------------------------

template <typename TYPE>
int Stack<TYPE>::Check ()
{
    if (this == nullptr)
    {
        return STACK_NULL_STACK_PTR;
    }

    else if (errCode_ == STACK_NOT_CONSTRUCTED)
    {
        return STACK_NOT_CONSTRUCTED;
    }

    else if (errCode_ == STACK_DESTRUCTED)
    {
        return STACK_DESTRUCTED;
    }

#ifdef HASH_PROTECT
    else if (stackhash_ != hash(this, SizeForHash()))
    {
        errCode_ = STACK_INCORRECT_HASH;
    }
#endif // HASH_PROTECT

    else if (data_ == nullptr)
    {
        errCode_ = STACK_NULL_DATA_PTR;
    }

    else if (size_cur_ > capacity_)
    {
        errCode_ = STACK_SIZE_BIGGER_CAPACITY;
    }

    else if ((capacity_ == 0) || (capacity_ > MAX_CAPACITY))
    {
        errCode_ = STACK_CAPACITY_WRONG_VALUE;
    }

    else if (! isPOISON(data_[size_cur_]))
    {
        errCode_ = STACK_WRONG_CUR_SIZE;
    }

#ifdef HASH_PROTECT
    else if (datahash_ != hash(data_, capacity_ * sizeof(TYPE)))
    {
        errCode_ = STACK_INCORRECT_HASH;
    }
#endif // HASH_PROTECT

    else
    {
        errCode_ = STACK_OK;
    }

    return errCode_;
}

//------------------------------------------------------------------------------

template <typename TYPE>
void Stack<TYPE>::ErrorPrint (FILE* fp)
{
    assert(fp != nullptr);

    if (this == nullptr)
    {
        CONSOLE_PRINT{ printf("%s\n", stk_errstr[STACK_NULL_STACK_PTR + 1]); }
    }

    else if (errCode_ != STACK_OK)
    {
        CONSOLE_PRINT{ printf("%s\n", stk_errstr[errCode_ + 1]); }

        if (fp != stdout) fprintf(fp, "\n%s\n", stk_errstr[errCode_ + 1]);
    }
}

//------------------------------------------------------------------------------

static void printError (const char* logname, const char* file, int line, const char* function, int err)
{
    assert(function != nullptr);
    assert(logname  != nullptr);
    assert(file     != nullptr);

    FILE* log = fopen(logname, "a");
    assert(log != nullptr);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log, "TIME: %d-%02d-%02d %02d:%02d:%02d\n\n",
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec);

    fprintf(log, "ERROR: file %s  line %d  function %s\n\n", file, line, function);
    fprintf(log, "%s\n", stk_errstr[err + 1]);

    printf("ERROR: file %s  line %d  function %s\n", file, line, function);
    printf("%s\n\n", stk_errstr[err + 1]);

    fprintf(log, "********************************************************************************\n");

    fclose(log);
}

//------------------------------------------------------------------------------

#ifdef HASH_PROTECT

template <typename TYPE>
size_t Stack<TYPE>::SizeForHash ()
{
    assert(this != nullptr);

    size_t size = 0;

    size += sizeof(name_);
    size += sizeof(capacity_);
    size += sizeof(size_cur_);
    size += sizeof(data_);
    size += sizeof(id_);

    return size;
}

#endif // HASH_PROTECT

//------------------------------------------------------------------------------
