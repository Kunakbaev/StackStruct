#include <random>
#include <chrono>

#include "../include/memorySafeArray.hpp"

// TODO: change errors to arrayErrors
// just in case it will colide with same define from stack lib
#define RETURN_IF_ARR_INVALID(array)                            \
    do {                                                        \
        bool isValid = false;                                   \
        Errors errorTmp = isSafeArrayValid(array, &isValid);    \
        IF_ERR_RETURN(errorTmp);                                \
        if (!isValid) {                                         \
            IF_ERR_RETURN(ERROR_STACK_INVALID_FIELD_VALUES);    \
            /*assert(false);                                 */ \
        }                                                       \
    } while (0)

const size_t MAX_ARRAY_SIZE = 1 << 13;

//std::mt19937_64 rnd2(std::chrono::steady_clock::now().time_since_epoch().count());
std::mt19937_64 rnd2(228);

// these are consts
uint8_t FRONT_CANARY[SIZE_OF_CANARY];
uint8_t BACK_CANARY[SIZE_OF_CANARY];

constexpr void initFrontAndBackCanaries() {
    for (size_t i = 0; i < SIZE_OF_CANARY; ++i) {
        FRONT_CANARY[i] = rnd2();
        BACK_CANARY[i]  = rnd2();
    }
}

Errors myMemCopy(void* destVoidPtr, const void* sourceVoidPtr, size_t memToCopy) {
    uint8_t* dest   = (uint8_t*)(destVoidPtr);
    uint8_t* source = (uint8_t*)(sourceVoidPtr);

    IF_ARG_NULL_RETURN(dest);
    IF_ARG_NULL_RETURN(source);

    //memcpy(dest, source, memToCopy);
    for (size_t byteInd = 0; byteInd < memToCopy; ++byteInd) {
        dest[byteInd] = source[byteInd];
    }

    return STATUS_OK;
}

Errors setCanariesForSafeArray(const SafeArray* array) {
    // setting front and back canaries
    Errors error = STATUS_OK;

    // TODO: don't do this if new size is same as previous
    //if (array->arraySize == 0) {
    error = myMemCopy(array->array, FRONT_CANARY, array->elementSize);
    IF_ERR_RETURN(error);

    //}

    size_t backCanaryIndex = SIZE_OF_CANARY + array->elementSize * array->arraySize;
    error = myMemCopy(array->array + backCanaryIndex, BACK_CANARY, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);

    return STATUS_OK;
}

// this function allocates memory only if arraySize > 0
Errors constructSafeArray(size_t arraySize, size_t elementSize, SafeArray* array) {
    IF_ARG_NULL_RETURN(array);

    *array = {};
    array->elementSize = elementSize;
    array->array       = NULL;

    initFrontAndBackCanaries();
    if (arraySize > 0) {
        Errors error = resizeSafeArray(array, arraySize);
        IF_ERR_RETURN(error);
        LOG_DEBUG("ok");
    }
    array->arraySize = arraySize;

    return STATUS_OK;
}

Errors resizeSafeArray(SafeArray* array, size_t newSize) {
    LOG_DEBUG_VARS(array);
    IF_ARG_NULL_RETURN(array);
    IF_NOT_COND_RETURN(newSize < MAX_ARRAY_SIZE,
                       ERROR_STACK_NEW_CAPACITY_TOO_BIG);
    RETURN_IF_ARR_INVALID(array);

    // nothing to do
    if (newSize == array->arraySize)
        return STATUS_OK;

    size_t newNumOfBytes = newSize * array->elementSize + 2 * SIZE_OF_CANARY;
    void* tmpPtr = realloc(array->array, newNumOfBytes);
    IF_NOT_COND_RETURN(tmpPtr, ERROR_MEMORY_REALLOCATION_ERROR);
    array->array = (uint8_t*)tmpPtr;

    size_t oldSize = array->arraySize;
    array->arraySize = newSize;

    LOG_DEBUG_VARS(oldSize, newSize, newNumOfBytes, array->array);

    if (newSize > oldSize) {
        size_t deltaBytes = (newSize - oldSize) * array->elementSize;
        memset(array->array + oldSize + SIZE_OF_CANARY * 2, 0, deltaBytes);
    }

#ifdef IS_CANARY_PROTECTION_ON
    Errors error = setCanariesForSafeArray(array);
    IF_ERR_RETURN(error);
#endif

    return STATUS_OK;
}

Errors setValueToSafeArrayElement(SafeArray* array, size_t elementIndex, const void* elementVoidPtr) {
    uint8_t* element = (uint8_t*)elementVoidPtr;

    IF_ARG_NULL_RETURN(array);
    IF_ARG_NULL_RETURN(element);
    IF_ARG_NULL_RETURN(array->array);
    RETURN_IF_ARR_INVALID(array);

    // FIXME: change to array errors
    IF_NOT_COND_RETURN(elementIndex < array->arraySize,
                       ERROR_STACK_INCORRECT_NUM_OF_ELEMS);

    int arrInd = SIZE_OF_CANARY + elementIndex * array->elementSize;
    Errors error = myMemCopy(array->array + arrInd, elementVoidPtr, array->elementSize);

    return error;
}

Errors getValueFromSafeArrayElement(const SafeArray* array, size_t elementIndex, void* elementVoidPtr) {
    uint8_t* element = (uint8_t*)(elementVoidPtr);

    IF_ARG_NULL_RETURN(array);
    IF_ARG_NULL_RETURN(array->array);
    IF_ARG_NULL_RETURN(element);
    RETURN_IF_ARR_INVALID(array);

//     for (size_t byteInd = 0; byteInd < array->elementSize; ++byteInd) {
//         int arrInd = SIZE_OF_CANARY + elementIndex * array->elementSize + byteInd;
//         //IF_NOT_COND_RETURN(arrInd < array->ele
//         //LOG_DEBUG_VARS(arrInd, *element, stack->array[arrInd]);
//         *(element + byteInd) = array->array[arrInd];
//     }

    int arrInd = SIZE_OF_CANARY + elementIndex * array->elementSize;
    Errors error = myMemCopy(elementVoidPtr, array->array + arrInd, array->elementSize);
    IF_ERR_RETURN(error);

    return error;
}

Errors isSafeArrayValid(const SafeArray* array, bool* isValid) {
    IF_ARG_NULL_RETURN(array);

    *isValid = true;
    *isValid &= array->arraySize == 0 || array->array != NULL;
    *isValid &= array->arraySize <  MAX_ARRAY_SIZE;
    if (!*isValid)
        return STATUS_OK;
    if (array->array == NULL)
        return STATUS_OK;

#ifdef IS_CANARY_PROTECTION_ON
    // TODO: change to dynamic memory
    uint8_t canaryFront[SIZE_OF_CANARY] = {};
    uint8_t canaryBack[SIZE_OF_CANARY]  = {};

    Errors error = myMemCopy(canaryFront, array->array, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);
    size_t backCanaryIndex = SIZE_OF_CANARY + array->elementSize * array->arraySize;
    error = myMemCopy(canaryBack, array->array + backCanaryIndex, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);

    LOG_DEBUG_VARS(canaryFront[0], canaryBack[0]);
    LOG_DEBUG_VARS(FRONT_CANARY[0], BACK_CANARY[0]);

    int cmpResultFront = memcmp(canaryFront, FRONT_CANARY, SIZE_OF_CANARY);
    int cmpResultBack  = memcmp(canaryBack,  BACK_CANARY,  SIZE_OF_CANARY);
    if (cmpResultFront != 0 || cmpResultBack != 0) {
        LOG_ERROR("Error: canary protection failed.\n");
        *isValid = false;
        return STATUS_OK;
    }
#endif

    return STATUS_OK;
}

Errors destructSafeArray(SafeArray* array) {
    IF_ARG_NULL_RETURN(array);
    IF_ARG_NULL_RETURN(array->array);
    RETURN_IF_ARR_INVALID(array);

    FREE(array->array);
    array->arraySize   = 0;
    array->elementSize = 0;

    return STATUS_OK;
}
