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

const size_t MAX_ARRAY_SIZE                = 1 << 13;
constexpr const size_t MAX_ARRAY_ELEM_SIZE = 32;

const size_t LOG_BUFFER_SIZE               = 200;
char* bufferToOutputStackElems             = "";
char* buffForByte                          = "";

//std::mt19937_64 rnd2(std::chrono::steady_clock::now().time_since_epoch().count());
// easier for debug with constant seed
// TODO: return dynamic seed
std::mt19937_64 rnd2(228);

const uint64_t BASE_NUMBER_FOR_HASHES        = rnd2() >> 32; // ???

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

    memcpy(dest, source, memToCopy);
    // for (size_t byteInd = 0; byteInd < memToCopy; ++byteInd) {
    //     dest[byteInd] = source[byteInd];
    // }

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


//  -----------------------------       FUNCTIONS FOR HASHES        ----------------------------------

static Errors addNumToHash(uint64_t* stackHash, const uint64_t* number) {
    IF_ARG_NULL_RETURN(stackHash);
    IF_ARG_NULL_RETURN(number);

    *stackHash *= BASE_NUMBER_FOR_HASHES;
    *stackHash += *number + 1;

    return STATUS_OK;
}

static Errors getHashOfArray(const SafeArray* array, uint64_t* arrayHash) {
    IF_ARG_NULL_RETURN(array);
    IF_ARG_NULL_RETURN(arrayHash);

    *arrayHash = 0;
    Errors err = STATUS_OK;
    err = addNumToHash(arrayHash, (uint64_t*)&array->elementSize);
    IF_ERR_RETURN(err);
    addNumToHash(arrayHash,       (uint64_t*)&array->arraySize);
    IF_ERR_RETURN(err);
    addNumToHash(arrayHash,       (uint64_t*)&array->array); // address of a pointer
    IF_ERR_RETURN(err);

    for (size_t byteInd = 0; byteInd < array->arraySize * array->elementSize; ++byteInd) {
        // IF_NOT_COND_RETURN(byteInd < stack->array.arraySize,
        //                    ERROR_ARRAY_BAD_INDEX);
        err = addNumToHash(arrayHash, (uint64_t*)&array->array[byteInd]);
        IF_ERR_RETURN(err);
    }

    return STATUS_OK;
}

static Errors recalculateHashOfArray(SafeArray* array) {
    IF_ARG_NULL_RETURN(array);
    // RETURN_IF_INVALID(stack);

    Errors err = getHashOfArray(array, &array->structHash);
    IF_ERR_RETURN(err);

    return STATUS_OK;
}

//  ----------------------------------      CONSTRUCT SAFE ARRAY        --------------------------------------

// this function allocates memory only if arraySize > 0
Errors constructSafeArray(size_t arraySize, size_t elementSize, SafeArray* array) {
    IF_ARG_NULL_RETURN(array);

    *array = {};
    array->elementSize = elementSize;
    array->array       = NULL;

    initFrontAndBackCanaries();

    Errors error = myMemCopy(array->frontCanary, FRONT_CANARY, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);
    error = myMemCopy(array->backCanary,  BACK_CANARY, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);

    if (arraySize > 0) {
        error = resizeSafeArray(array, arraySize);
        IF_ERR_RETURN(error);
    }
    array->arraySize = arraySize;

    bufferToOutputStackElems = (char*)calloc(LOG_BUFFER_SIZE, sizeof(char));
    IF_NOT_COND_RETURN(bufferToOutputStackElems != NULL,
                       ERROR_MEMORY_ALLOCATION_ERROR);
    buffForByte = (char*)calloc(4, sizeof(char*));
    IF_NOT_COND_RETURN(buffForByte != NULL,
                       ERROR_MEMORY_ALLOCATION_ERROR);

    error = recalculateHashOfArray(array);
    IF_ERR_RETURN(error);
    RETURN_IF_ARR_INVALID(array);

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
        memset(array->array + oldSize + SIZE_OF_CANARY, 0, deltaBytes);
    }

    Errors error = STATUS_OK;
#ifdef IS_CANARY_PROTECTION_ON
    error = setCanariesForSafeArray(array);
    IF_ERR_RETURN(error);
#endif

    error = recalculateHashOfArray(array);
    IF_ERR_RETURN(error);
    RETURN_IF_ARR_INVALID(array);

    return STATUS_OK;
}

Errors setValueToSafeArrayElement(SafeArray* array, size_t elementIndex, const void* elementVoidPtr) {
    IF_ARG_NULL_RETURN(array);
    IF_ARG_NULL_RETURN(elementVoidPtr);
    IF_ARG_NULL_RETURN(array->array);
    RETURN_IF_ARR_INVALID(array);

    // FIXME: change to array errors
    IF_NOT_COND_RETURN(elementIndex < array->arraySize,
                       ERROR_STACK_INCORRECT_NUM_OF_ELEMS);

    int arrInd = SIZE_OF_CANARY + elementIndex * array->elementSize;
    Errors error = myMemCopy(array->array + arrInd, elementVoidPtr, array->elementSize);

    error = recalculateHashOfArray(array);
    IF_ERR_RETURN(error);
    RETURN_IF_ARR_INVALID(array);

    return error;
}

Errors getValueFromSafeArrayElement(const SafeArray* array, size_t elementIndex, void* elementVoidPtr) {
    IF_ARG_NULL_RETURN(array);
    IF_ARG_NULL_RETURN(array->array);
    IF_ARG_NULL_RETURN(elementVoidPtr);
    RETURN_IF_ARR_INVALID(array);

    int arrInd = SIZE_OF_CANARY + elementIndex * array->elementSize;
    Errors error = myMemCopy(elementVoidPtr, array->array + arrInd, array->elementSize);
    IF_ERR_RETURN(error);
    RETURN_IF_ARR_INVALID(array);

    return error;
}

//  -----------------------------       SAFE ARRAY LOGGING        ----------------------------------

static Errors logStackElement(const uint8_t* element, size_t elementSize) {
    IF_ARG_NULL_RETURN(element);
    IF_NOT_COND_RETURN(elementSize < MAX_ARRAY_ELEM_SIZE,
                       ERROR_STACK_ELEM_SIZE_TOO_BIG);

    memset(bufferToOutputStackElems, 0, LOG_BUFFER_SIZE);
    for (size_t byteInd = 0; byteInd < elementSize; ++byteInd) {
        uint8_t elem = *(element + byteInd);
        sprintf(buffForByte, "%p ", elem);
        strcat(bufferToOutputStackElems, buffForByte);
    }
    LOG_DEBUG(bufferToOutputStackElems);

    return STATUS_OK;
}

Errors dumpArrayLog(const SafeArray* array) {
    IF_ARG_NULL_RETURN(array);
    RETURN_IF_ARR_INVALID(array);

    LOG_DEBUG("--------------------------------------");
    LOG_DEBUG("Array:");
    LOG_DEBUG_VARS(array);
    LOG_DEBUG_VARS(array->arraySize);
    LOG_DEBUG_VARS(array->elementSize);
    // LOG_DEBUG_VARS(array->array[4], array->array[5], array->array[6]);
    LOG_DEBUG("elements:");
    for (size_t elemIndex = 0; elemIndex < array->arraySize; ++elemIndex) {
        size_t arrInd = SIZE_OF_CANARY + elemIndex * array->elementSize;
        LOG_DEBUG_VARS(elemIndex);
        Errors error = logStackElement(array->array + arrInd, array->elementSize);
        IF_ERR_RETURN(error);
    }
    LOG_DEBUG("--------------------------------------");

    // just in case, maybe too paranoid
    RETURN_IF_ARR_INVALID(array);

    return STATUS_OK;
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
    // stack smash attack (or just error) from one of the ends of struct
    size_t backCanaryIndex = SIZE_OF_CANARY + array->elementSize * array->arraySize;
    // TODO: I can simply pass pointer to begining of canary
    int cmpResultFront       = memcmp(array->array,                    FRONT_CANARY, SIZE_OF_CANARY);
    int cmpResultBack        = memcmp(array->array + backCanaryIndex,  BACK_CANARY,  SIZE_OF_CANARY);

    int cmpResultFrontStruct = memcmp(array->frontCanary, FRONT_CANARY, SIZE_OF_CANARY);
    int cmpResultBackStruct  = memcmp(array->backCanary,  BACK_CANARY,  SIZE_OF_CANARY);
    LOG_DEBUG_VARS(array->frontCanary[0], FRONT_CANARY);
    LOG_DEBUG_VARS(cmpResultFront, cmpResultBack);
    LOG_DEBUG_VARS(cmpResultFrontStruct, cmpResultBackStruct);
    if (cmpResultFront != 0 || cmpResultBack != 0 ||
            cmpResultFrontStruct != 0 || cmpResultBackStruct != 0) {
        LOG_ERROR("Error: canary protection failed.\n");
        *isValid = false;
        return STATUS_OK;
    }
#endif

#ifdef IS_HASH_MEMORY_CHECK_DEFINE
    uint64_t correctArrayHash = 0;
    Errors err = getHashOfArray(array, &correctArrayHash);
    IF_ERR_RETURN(err);

    LOG_DEBUG_VARS(correctArrayHash, array->structHash);
    IF_ERR_RETURN(err);
    IF_NOT_COND_RETURN(correctArrayHash == array->structHash,
                       ERROR_STACK_MEMORY_HASH_CHECK_FAILED);
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

    FREE(bufferToOutputStackElems);
    FREE(buffForByte);

    return STATUS_OK;
}
