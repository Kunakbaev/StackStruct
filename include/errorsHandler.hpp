#ifndef ERRORS_HANDLER
#define ERRORS_HANDLER

#include "../source/utils.cpp"

// log and return happens only if error realy occured (error != STATUS_OK)
#define IF_ERR_RETURN(error)                                        \
    do {                                                            \
        if (error != STATUS_OK) {                                   \
            LOG_ERROR(getErrorMessage(error));                      \
            assert(error != STATUS_OK);                             \
            return error;                                           \
        }                                                           \
    } while(0)

#define IF_NOT_COND_RETURN(condition, error)                   \
    do {                                                            \
        if (!(condition)) {                                         \
            LOG_ERROR(getErrorMessage(error));                      \
            assert(condition);                                      \
            return error;                                           \
        }                                                           \
    } while(0)

#define IF_ARG_NULL_RETURN(arg)                                \
    do {                                                            \
        if (arg == NULL) {                                          \
            LOG_ERROR(getErrorMessage(ERROR_INVALID_ARGUMENT));     \
            assert(arg != NULL);                                    \
            return ERROR_INVALID_ARGUMENT;                          \
        }                                                           \
    } while (0)

enum Errors {
    //  --------------------------      GENERAL ERRORS          -----------------------------
    STATUS_OK                               = 0,                  // no error, everything is valid
    ERROR_INVALID_ARGUMENT                  = 1,                  // usually when argument is set to NULL
    ERROR_MEMORY_ALLOCATION_ERROR           = 2,                  // usually when memory allocation by calloc fails
    ERROR_MEMORY_REALLOCATION_ERROR         = 3,                  // couldn't reallocate memory
    // STACK_ERROR                             = 4,                  // error occured in stack

    ERROR_STACK_INVALID_FIELD_VALUES        = 4,                  // some of stack fields are invalid
    ERROR_STACK_INCORRECT_NUM_OF_ELEMS      = 5,                  // incorrect index of stack (usually happens during push or pop)
    ERROR_STACK_NEW_CAPACITY_TOO_BIG        = 6,                  // new capacity is bigger than max capacity
    ERROR_STACK_INCORRECT_CAP_KOEF          = 7,                  // incorrect capacity change koefficient
    ERROR_STACK_MEMORY_HASH_CHECK_FAILED    = 8,                  // hash of structure memory is not correct, stack smash or other error has occured
    ERROR_STACK_ELEM_SIZE_TOO_SMALL         = 9,                  // size of stack element is too small (possibly set to 0)
    ERROR_STACK_ELEM_SIZE_TOO_BIG           = 10,                 // size of stack element is too big

    ERROR_ARRAY_BAD_INDEX                   = 11,                 // index of element is < 0 or >= size of array
};

const char* getErrorMessage(Errors error);

#endif
