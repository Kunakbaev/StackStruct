#include <iostream>
#include "../LoggerLib/include/logLib.hpp"
#include "../include/stackLib.hpp"

int main() {
    setLoggingLevel(DEBUG);

    Stack stack = {};
    Errors error = STATUS_OK;
    error = constructStack(&stack, 0);
    IF_ERR_RETURN(error);

    for (int _ = 0; _ < 5; ++_) {
        error = pushElementToStack(&stack, _ + 1.4321);
        LOG_DEBUG_VARS(_);
        IF_ERR_RETURN(error);
    }

    error = dumpStackLog(&stack);
    IF_ERR_RETURN(error);

    long double stackPopElem = -1;
    for (int _ = 0; _ < 5; ++_) {
        popElementToStack(&stack, &stackPopElem);
        LOG_DEBUG_VARS(stackPopElem);
    }

    error = dumpStackLog(&stack);
    IF_ERR_RETURN(error);

    return 0;
}
