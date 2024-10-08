#include <iostream>
#include <math.h>

// remove warnings via #pragma GCC diagnostics ignored "WARN TO BE IGNORED format non literal"
#include "../LoggerLib/include/logLib.hpp"

// doesn't work
#define IS_CANARY_PROTECTION_ON
#define IS_HASH_MEMORY_CHECK_DEFINE

#include "../include/stackLib.hpp"
#include "../include/memorySafeArray.hpp"

Errors testSafeArray();
Errors testStack();

int main() {
    setLoggingLevel(DEBUG);

    Errors error = STATUS_OK;
    // error = testSafeArray();
    // IF_ERR_RETURN(error);

    error = testStack();
    IF_ERR_RETURN(error);

    return 0;
}



// -----------------------------------     TEST SAFE ARRAY      -----------------------------

// очень смешное название, пока нифига не сейф
Errors testSafeArray() {
    SafeArray array = {};
    Errors error = STATUS_OK;

    error = constructSafeArray(10, 4, &array);
    IF_ERR_RETURN(error);

    // uint8_t* ptr = (uint8_t*)(&array);
    // *(ptr + 6) = 1832;
    LOG_DEBUG("--------------------------------------");

    int num = 10;
    error = setValueToSafeArrayElement(&array, 0, &num);
    IF_ERR_RETURN(error);
    LOG_DEBUG_VARS("bruh");

    error = dumpArrayLog(&array);
    IF_ERR_RETURN(error);

    int elem = -1;
    error = getValueFromSafeArrayElement(&array, 0, &elem);
    IF_ERR_RETURN(error);
    LOG_DEBUG_VARS(elem);

    return STATUS_OK;
}

// -----------------------------------     TEST STACK      -----------------------------

Errors testStack() {
    Stack stack = {};
    Errors error = STATUS_OK;

    // if stack parametre is void*, then user doesn't know about fields of Stack struct???
    error = constructStack(&stack, 15, 4);
    IF_ERR_RETURN(error);
    //LOG_DEBUG("main");

    // uint8_t* ptr = (uint8_t*)stack.array.array;
    // *(ptr) = 190;
    //*(ptr + 1) = 102;

    // LOG_DEBUG("be");
    // dumpStackLog(&stack);
    // //LOG_DEBUG("ok");

    for (int i = 0; i < 9; ++i) {
        //LOG_DEBUG("biba");
        //dumpStackLog(&stack);
        int number = (i + 1) * 10;
        // FIXME: if & is not added, error occurs
        error = pushElementToStack(&stack, (const void*)&number);
        //LOG_DEBUG_VARS(i, number, error);
        IF_ERR_RETURN(error);

        // stack.numberOfElements = 0;
        // stack.stackCapacity = 0;
        // stack.array = NULL;
        //dumpStackLog(&stack);
        LOG_DEBUG_VARS(stack.array.arraySize, stack.numberOfElements);
        uint8_t* ptr = (uint8_t*)stack.array.array;

        dumpStackLog(&stack);
        //*ptr = 19290;
        //*(ptr + 5) = 102;
    }

    uint8_t* ptr = (uint8_t*)stack.array.array;
    // *ptr = 19290;
    // *(ptr + 3) = 102;

    LOG_DEBUG("------------ popping elements --------------");
    // return STATUS_OK;
    // stack.numberOfElements = 3;
    // stack.array[1] = 2;

    // error = dumpStackLog(&stack);
    // IF_ERR_RETURN(error);

    int stackPopElem = -1;
    for (int _ = 0; _ < 5; ++_) {
        popElementToStack(&stack, &stackPopElem);
        LOG_DEBUG_VARS(stackPopElem);
        printf("elem : %d\n", stackPopElem);

        //dumpStackLog(&stack);
    }

    error = dumpStackLog(&stack);
    IF_ERR_RETURN(error);

    error = destructStack(&stack);
    IF_ERR_RETURN(error);

    return STATUS_OK;
}
