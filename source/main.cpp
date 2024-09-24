#include <iostream>
#include "../LoggerLib/include/logLib.hpp"
#include "../include/stackLib.hpp"

int main() {
    setLoggingLevel(DEBUG);

    Stack stack = {};
    Errors error = STATUS_OK;
    error = constructStack(&stack, 23);
    LOG_AND_RETURN(error);

    error = pushElementToStack(&stack, 10);
    LOG_AND_RETURN(error);

    error = pushElementToStack(&stack, 20);
    LOG_AND_RETURN(error);

    error = pushElementToStack(&stack, 30);
    LOG_AND_RETURN(error);

    error = dumpStackLog(&stack);
    LOG_AND_RETURN(error);

    int stackPopElem = -1;
    popElementToStack(&stack, &stackPopElem);
    LOG_DEBUG_VARS(stackPopElem);

    popElementToStack(&stack, &stackPopElem);
    LOG_DEBUG_VARS(stackPopElem);

    popElementToStack(&stack, &stackPopElem);
    LOG_DEBUG_VARS(stackPopElem);

    return 0;
}
