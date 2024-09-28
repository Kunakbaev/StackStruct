#ifndef STACK_LIB
#define STACK_LIB

#include <inttypes.h>

#include "errorsHandler.hpp"

// #ifdef HASH_MEMORY_CHECK_DEFINE
// #define IS_HASH_MEMORY_CHECK_DEFINE
// #endif

#define IS_HASH_MEMORY_CHECK_DEFINE

struct Stack {
    uint64_t structHash;
    int numberOfElements;
    int stackCapacity;
    uint8_t* array;
    size_t elementSize;
};

Errors constructStack(Stack* stack, int initialCapacity, size_t stackElemSize);
Errors pushElementToStack(Stack* stack, const void* element);
Errors popElementToStack(Stack* stack, void* element);
Errors isStackValid(Stack* stack, bool* isValid);
Errors dumpStackLog(Stack* stack);
Errors destructStack(Stack* stack);

#endif
