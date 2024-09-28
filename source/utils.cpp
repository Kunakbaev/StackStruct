#ifndef UTILS_FILE
#define UTILS_FILE

#include "../LoggerLib/include/logLib.hpp"

#define FREE(x)         \
do {                    \
    free(x);            \
    (x) = NULL;         \
} while(0)

#endif
