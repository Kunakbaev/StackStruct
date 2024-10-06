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

#define IF_NOT_COND_RETURN(condition, error)                        \
    do {                                                            \
        bool tmpCondition = (condition);                            \
        if (!tmpCondition) {                                        \
            LOG_ERROR(getErrorMessage(error));                      \
            assert(tmpCondition);                                   \
            return error;                                           \
        }                                                           \
    } while(0)

// ASK: should I create tmp variable for this case too?
#define IF_ARG_NULL_RETURN(arg)                                     \
    do {                                                            \
        if (arg == NULL) {                                          \
            LOG_ERROR(getErrorMessage(ERROR_INVALID_ARGUMENT));     \
            assert(arg != NULL);                                    \
            return ERROR_INVALID_ARGUMENT;                          \
        }                                                           \
    } while (0)

// ASK: how to rewrite this enum, so errors are distributed among modules
enum Errors {
    //  --------------------------      GENERAL ERRORS          -----------------------------
    STATUS_OK                                   = 0,                  // no error, everything is valid
    ERROR_INVALID_ARGUMENT                      = 1,                  // usually when argument is set to NULL
    ERROR_MEMORY_ALLOCATION_ERROR               = 2,                  // usually when memory allocation by calloc fails
    ERROR_MEMORY_REALLOCATION_ERROR             = 3,                  // couldn't reallocate memory
    // STACK_ERROR                              = 4,                  // error occured in stack

    ERROR_STACK_INVALID_FIELD_VALUES            = 4,                  // some of stack fields are invalid
    ERROR_STACK_INCORRECT_NUM_OF_ELEMS          = 5,                  // incorrect index of stack (usually happens during push or pop)
    ERROR_STACK_NEW_CAPACITY_TOO_BIG            = 6,                  // new capacity is bigger than max capacity
    ERROR_STACK_INCORRECT_CAP_KOEF              = 7,                  // incorrect capacity change koefficient
    ERROR_STACK_ELEM_SIZE_TOO_SMALL             = 9,                  // size of stack element is too small (possibly set to 0)
    ERROR_STACK_ELEM_SIZE_TOO_BIG               = 10,                 // size of stack element is too big
    ERROR_STACK_MEMORY_HASH_CHECK_FAILED        = 11,                 // hash of structure memory is not correct, stack smash or other error has occured
    ERROR_STACK_CANARY_PROTECTION_FAILED        = 12,                 // canary protection fail
    ERROR_STACK_ARRAY_SIZE_IS_TOO_BIG           = 13,                 // size of stack array is too big
    ERROR_STACK_INVALID_NUM_OF_ELEMS            = 14,                 // number of elements in stack is either too small (<0) or too big(>capacity)

    ERROR_ARRAY_INVALID_FIELD_VALUES            = 15,                 // some of array fields are invalid
    ERROR_ARRAY_BAD_INDEX                       = 16,                 // index of element is < 0 or >= size of array
    ERROR_ARRAY_MEMORY_HASH_CHECK_FAILED        = 17,                 // hash of structure memory is not correct
    ERROR_ARRAY_CANARY_PROTECTION_FAILED        = 18,                 // canary protection fail
    ERROR_ARRAY_SIZE_EMPTY_ARRAY_NOT_ZERO_SIZE  = 19,                 // array is NULL, but it's size is greater than zero
    ERROR_ARRAY_SIZE_IS_TOO_BIG                 = 20,                 // array size is too big
    ERROR_ARRAY_NEW_ARRAY_SIZE_IS_TOO_BIG       = 21,                 // new size for array is too big
    ERROR_ARRAY_ELEMENT_SIZE_IS_TOO_BIG         = 22,                 // size of array's element is too big
};

const char* getErrorMessage(Errors error);

#endif
