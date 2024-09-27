#include <iostream>
#include <math.h>

#include "../LoggerLib/include/logLib.hpp"

#define HASH_MEMORY_CHECK_DEFINE 1

#include "../include/stackLib.hpp"

#define HASH_MEMORY_CHECK_DEFINE 1

int main() {
    setLoggingLevel(DEBUG);

    Stack stack = {};
    Errors error = STATUS_OK;

    // if stack parametre is void*, then user doesn't know about fields of Stack struct???
    error = constructStack(&stack, 0);
    IF_ERR_RETURN(error);

    dumpStackLog(&stack);
    LOG_DEBUG("ok");

    for (int _ = 0; _ < 5; ++_) {
        LOG_DEBUG("biba");
        //dumpStackLog(&stack);
        error = pushElementToStack(&stack, _ + 1.4321);
        LOG_DEBUG_VARS(_, error);
        IF_ERR_RETURN(error);

        //stack.numberOfElements = 0;
        // stack.stackCapacity = 0;
        // stack.array = NULL;
        dumpStackLog(&stack);
    }

    LOG_DEBUG("------------ popping elements --------------");
   // return 0;
    //stack.numberOfElements = 3;
    //stack.array[1] = 2;

    // error = dumpStackLog(&stack);
    // IF_ERR_RETURN(error);

    long double stackPopElem = -1;
    for (int _ = 0; _ < 5; ++_) {
        popElementToStack(&stack, &stackPopElem);
        LOG_DEBUG_VARS(stackPopElem);
    }

    error = dumpStackLog(&stack);
    IF_ERR_RETURN(error);

    return 0;
}
