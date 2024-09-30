#include <iostream>
#include <math.h>

#include "../LoggerLib/include/logLib.hpp"

#define HASH_MEMORY_CHECK_DEFINE 1
#define IS_CANARY_PROTECTION_ON

#include "../include/stackLib.hpp"

int main() {
    setLoggingLevel(DEBUG);

    Stack stack = {};
    Errors error = STATUS_OK;

    // if stack parametre is void*, then user doesn't know about fields of Stack struct???
    error = constructStack(&stack, 0, 4);
    IF_ERR_RETURN(error);

    uint8_t* ptr = (uint8_t*)&stack;
    //*ptr = 19290;
    *(ptr + 1) = 102;

    dumpStackLog(&stack);
    LOG_DEBUG("ok");

    for (int i = 0; i < 5; ++i) {
        LOG_DEBUG("biba");
        //dumpStackLog(&stack);
        int number = (i + 1) * 10;
        // FIXME: if & is not added, error occurs
        error = pushElementToStack(&stack, (const void*)&number);
        LOG_DEBUG_VARS(i, number, error);
        IF_ERR_RETURN(error);

        //stack.numberOfElements = 0;
        // stack.stackCapacity = 0;
        // stack.array = NULL;
        dumpStackLog(&stack);
    }

    LOG_DEBUG("------------ popping elements --------------");
    // return 0;
    // stack.numberOfElements = 3;
    // stack.array[1] = 2;

    // error = dumpStackLog(&stack);
    // IF_ERR_RETURN(error);

    int stackPopElem = -1;
    for (int _ = 0; _ < 5; ++_) {
        popElementToStack(&stack, &stackPopElem);
        LOG_DEBUG_VARS(stackPopElem);
    }

    error = dumpStackLog(&stack);
    IF_ERR_RETURN(error);

    error = destructStack(&stack);
    IF_ERR_RETURN(error);

    return 0;
}
