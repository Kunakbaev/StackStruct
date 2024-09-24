#ifndef STACK_LIB
#define STACK_LIB

#include "errorsHandler.hpp"

typedef long double StackElement;

struct Stack {
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
