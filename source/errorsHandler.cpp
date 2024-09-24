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
        case ERROR_FSTAT_BAD_STATUS:
            return "Error: fstat bad status.\n";

        //  -------------------   STACK ERRORS    ---------------------------
        case ERROR_STACK_INVALID_FIELD_VALUES:
            return "Error: some of stack fields are invalid.\n";
        case ERROR_STACK_INCORRECT_NUM_OF_ELEMS:
            return "incorrect index of stack (usually happens during push or pop).\n";
        default:
            return "Unknown error.\n";
    }
}
