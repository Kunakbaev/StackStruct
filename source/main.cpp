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
    error = testSafeArray();
    IF_ERR_RETURN(error);

    // error = testStack();
    // IF_ERR_RETURN(error);

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

    resizeSafeArray(&array, 3);

    int num = 10;
    error = setValueToSafeArrayElement(&array, 2, &num);
    IF_ERR_RETURN(error);
    LOG_DEBUG_VARS("bruh");

    resizeSafeArray(&array, 1);
    resizeSafeArray(&array, 0);

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
    error = constructStack(&stack, 1, 8);
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
        error = pushElementToStack(&stack, &number);
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

    size_t stackPopElem = -1;
    for (int i = 0; i < 5; ++i) {
        error = popElementToStack(&stack, &stackPopElem);
        IF_ERR_RETURN(error);
        LOG_DEBUG_VARS(stackPopElem);
        //printf("i : %d\n", i);
        LOG_INFO("stackPopElement: %zu, i : %d", stackPopElem, i);

        //dumpStackLog(&stack);
    }

    error = dumpStackLog(&stack);
    IF_ERR_RETURN(error);

    error = destructStack(&stack);
    IF_ERR_RETURN(error);

    return STATUS_OK;
}
