#ifndef STACK_LIB
#define STACK_LIB

#include <inttypes.h>

#include "errorsHandler.hpp"

// #ifdef HASH_MEMORY_CHECK_DEFINE
// #define IS_HASH_MEMORY_CHECK_DEFINE
// #endif

#define IS_HASH_MEMORY_CHECK_DEFINE

typedef long double StackElement;

struct Stack {
    uint64_t structHash;
    int numberOfElements;
    int stackCapacity;
    StackElement* array;
};

Errors constructStack(Stack* stack, int initialCapacity);
Errors pushElementToStack(Stack* stack, const StackElement element);
Errors popElementToStack(Stack* stack, StackElement* element);
Errors isStackIsValid(Stack* stack, bool* isValid);
Errors dumpStackLog(Stack* stack);
Errors destructStack(Stack* stack);

#endif
