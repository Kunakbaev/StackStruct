#include "../include/errorsHandler.hpp"

const char* getErrorMessage(Errors error) {
    switch (error) {
        //  -------------------   GENERAL ERRORS    ---------------------------
        case STATUS_OK:
            return "No erros, everything is valid.\n";
        case ERROR_INVALID_ARGUMENT:
            return "Error: invalid argument (possibly set to NULL).\n";
        case ERROR_MEMORY_ALLOCATION_ERROR:
            return "Error: couldn't allocate memory.\n";
        case ERROR_MEMORY_REALLOCATION_ERROR:
            return "Error: couldn't reallocate memory.\n";

        //  -------------------   STACK ERRORS    ---------------------------
        case ERROR_STACK_INVALID_FIELD_VALUES:
            return "Error: some of stack fields are invalid.\n";
        case ERROR_STACK_INCORRECT_NUM_OF_ELEMS:
            return "incorrect index of stack (usually happens during push or pop).\n";
        case ERROR_STACK_NEW_CAPACITY_TOO_BIG:
            return "Error: new capacity is bigger than max capacity.\n";
        case ERROR_STACK_INCORRECT_CAP_KOEF:
            return "Error: incorrect capacity change koefficient.\n";
        case ERROR_STACK_ELEM_SIZE_TOO_SMALL:
            return "Error: size of stack element is too small (possibly set to 0).\n";
        case ERROR_STACK_ELEM_SIZE_TOO_BIG:
            return "Error: size of stack element is too big.\n";
        case ERROR_STACK_MEMORY_HASH_CHECK_FAILED:
            return "Error: hash of stack structure memory is not correct, stack smash or other error has occured.\n";
        case ERROR_STACK_CANARY_PROTECTION_FAILED:
            return "Error: canary protection failed with stack struct.\n";

        case ERROR_ARRAY_INVALID_FIELD_VALUES:
            return "Error: some of array fields are invalid.\n";
        case ERROR_ARRAY_BAD_INDEX:
            return "Error: index of element is < 0 or >= size of array.\n";
        case ERROR_ARRAY_MEMORY_HASH_CHECK_FAILED:
            return "Error: hash of array structure memory is not correct, stack smash or other error has occured.\n";
        case ERROR_ARRAY_CANARY_PROTECTION_FAILED:
            return "Error: canary protection failed with array struct.\n";
        case ERROR_ARRAY_ELEMENT_SIZE_IS_TOO_BIG:
            return "Error: size of array's element is too big.\n";

        default:
            return "Unknown error.\n";
    }
}
