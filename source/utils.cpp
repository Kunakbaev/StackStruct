#ifndef UTILS_FILE
#define UTILS_FILE

#include "../LoggerLib/include/logLib.hpp"

// returns how many bytes are before this particular field of structure
#define OFFSET_OF_FIELD_I_COPIED_IT_FROM_WIKIPEDIA(structure, fieldName)   \
    ((size_t)&(((structure*)NULL)->fieldName))

#define FREE(x)         \
do {                    \
    free(x);            \
    (x) = NULL;         \
} while(0)

#endif
