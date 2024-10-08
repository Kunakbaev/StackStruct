#include <random>
#include <chrono>

#include "../include/memorySafeArray.hpp"

// just in case it will colide with same define from stack lib
#define RETURN_IF_ARR_INVALID(array)                            \
    do {                                                        \
        Errors errorTmp = isSafeArrayValid(array);              \
        IF_ERR_RETURN(errorTmp);                                \
    } while (0)

const size_t MAX_ARRAY_SIZE      = 1 << 13;
const size_t MAX_ARRAY_ELEM_SIZE = 32;

const size_t LOG_BUFFER_SIZE     = 200;

// KOLYA: cppreference: extern vs static

// these are consts
// ASK: how to make them really const?
uint8_t FRONT_CANARY[SIZE_OF_CANARY];
uint8_t  BACK_CANARY[SIZE_OF_CANARY];


// TODO: singleton
// uint8_t* GetFrontCanary()
// uint8_t* GetBackCanary();
static Errors initFrontAndBackCanaries() {
//     static uint8_t FRONT_CANARY[SIZE_OF_CANARY];
//     static uint8_t  BACK_CANARY[SIZE_OF_CANARY];
//     static bool is_init = false;
//
//     if (is_init) {
//         return FRONT_CANARY;
//     }

    Errors error = STATUS_OK;
    error = fillSequenceOfBytesWithRandomValues(FRONT_CANARY, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);

    error = fillSequenceOfBytesWithRandomValues(BACK_CANARY,  SIZE_OF_CANARY);
    IF_ERR_RETURN(error);

    //is_init = true;

    return STATUS_OK;
}

static Errors myMemCopy(void* destVoidPtr, const void* sourceVoidPtr, size_t memToCopy) {
    uint8_t*       dest   =       (uint8_t*)(  destVoidPtr);
    const uint8_t* source = (const uint8_t*)(sourceVoidPtr);

    IF_ARG_NULL_RETURN(dest);
    IF_ARG_NULL_RETURN(source);

    memcpy(dest, source, memToCopy);

    return STATUS_OK;
}

// setting front and back canaries
static Errors setCanariesForSafeArray(const SafeArray* array) {
    Errors error = STATUS_OK;

    // TODO: don't do this if new size is same as previous
    error = myMemCopy(array->array, FRONT_CANARY, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);

    size_t backCanaryIndex = SIZE_OF_CANARY + array->elementSize * array->arraySize;
    error = myMemCopy(array->array + backCanaryIndex, BACK_CANARY, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);

    return STATUS_OK;
}


//  -----------------------------       FUNCTIONS FOR HASHES        ----------------------------------

static Errors getHashOfArray(const SafeArray* array, hash_data_type* arrayHash) {
    IF_ARG_NULL_RETURN(array);
    IF_ARG_NULL_RETURN(arrayHash);

    // ASK: I have a function that returns hash of sequence of bytes, but
    // I don't want it to take hash into consideration
    // I don't want to make array argument NOT const and another option was to put
    // structHash field to the end of structure, but OFFSET_OF_FIELD was ver interesting
    // and also that's a new information for me
    *arrayHash = 0;
    //LOG_DEBUG_VARS(array, &array->structHash);
    GET_HASH_OF_STRUCT(array, structHash, arrayHash);
    //LOG_DEBUG_VARS(*arrayHash);

    hash_data_type hashOfData = 0;
    size_t memOfData = array->arraySize * array->elementSize + 2 * SIZE_OF_CANARY;
    Errors error = getHashOfSequenceOfBytes(array->array, memOfData, &hashOfData);
    IF_ERR_RETURN(error);
    *arrayHash *= hashOfData;
    //LOG_DEBUG_VARS("after : ", *arrayHash);

    return STATUS_OK;
}

// TODO: when change array by just one index, we can update structureHash without running for everytime
// ASK: maybe it's better to just put this function's body in ifdef, than to write everywhere same ifdef
static Errors recalculateHashOfArray(SafeArray* array) {
    IF_ARG_NULL_RETURN(array);

    Errors err = getHashOfArray(array, &array->structHash);
    IF_ERR_RETURN(err);

    return STATUS_OK;
}

//  ----------------------------------      CONSTRUCT SAFE ARRAY        --------------------------------------

// this function allocates memory only if arraySize > 0
Errors constructSafeArray(size_t arraySize, size_t elementSize, SafeArray* array) {
    IF_ARG_NULL_RETURN(array);

    *array             = {};
    array->elementSize = elementSize;
    array->array       = NULL;

    Errors error = initFrontAndBackCanaries();
    IF_ERR_RETURN(error);

// KOLYA: COPYPASTE!!!
#ifdef IS_CANARY_PROTECTION_ON
    error = myMemCopy(array->frontCanary, FRONT_CANARY, SIZE_OF_CANARY);
    IF_ERR_RETURN(error);
    error = myMemCopy(array->backCanary,  BACK_CANARY,  SIZE_OF_CANARY);
    IF_ERR_RETURN(error);
#endif

    if (arraySize > 0) {
        error = resizeSafeArray(array, arraySize);
        IF_ERR_RETURN(error);
    }
    array->arraySize = arraySize;

#ifdef IS_HASH_MEMORY_CHECK_DEFINE
    error = recalculateHashOfArray(array);
    IF_ERR_RETURN(error);
#endif

    LOG_DEBUG_VARS("ok");
    RETURN_IF_ARR_INVALID(array);

    return STATUS_OK;
}

Errors resizeSafeArray(SafeArray* array, size_t newSize) {
    LOG_DEBUG_VARS("resizing array", array, array->arraySize, newSize);
    IF_ARG_NULL_RETURN(array);
    IF_NOT_COND_RETURN(newSize < MAX_ARRAY_SIZE,
                       ERROR_ARRAY_NEW_ARRAY_SIZE_IS_TOO_BIG);
    RETURN_IF_ARR_INVALID(array);

    // nothing to do
    if (newSize == array->arraySize)
        return STATUS_OK;

    // KOLYA: to recalloc
    size_t oldSize = array->arraySize;
    size_t newNumOfBytes = newSize * array->elementSize + 2 * SIZE_OF_CANARY;
    size_t deltaSize  = (newSize < oldSize ? oldSize - newSize : newSize - oldSize); // still cringe?
    size_t deltaBytes = deltaSize * array->elementSize;

    LOG_DEBUG_VARS(array->arraySize, newSize);
    LOG_DEBUG_VARS(newNumOfBytes, array->elementSize);

    if (oldSize > newSize) { // just in case, old memory will be clean (filled with zeros)
        memset(array->array + newSize * array->elementSize + SIZE_OF_CANARY, 0, deltaBytes);
    }

    uint8_t* tmpPtr = (uint8_t*)realloc(array->array, newNumOfBytes);
    LOG_DEBUG("ok");
    IF_NOT_COND_RETURN(tmpPtr, ERROR_MEMORY_REALLOCATION_ERROR);
    LOG_DEBUG("ok");
    array->array = tmpPtr;

    array->arraySize = newSize;

    // cleaning newly allocated memory
    // ASK: maybe it's better to clear even if we are resizing to a smaller size

    // KOLYA: ssize_t
    LOG_DEBUG_VARS(newSize, oldSize, deltaSize, deltaBytes);
    LOG_DEBUG_VARS(newNumOfBytes, array->elementSize);
    LOG_DEBUG_VARS(oldSize * array->elementSize + SIZE_OF_CANARY);

    // ASK: is this ok?
    if (oldSize < newSize) {
        memset(array->array + oldSize * array->elementSize + SIZE_OF_CANARY, 0, deltaBytes);
    }

    Errors error = STATUS_OK;
#ifdef IS_CANARY_PROTECTION_ON
    error = setCanariesForSafeArray(array);
    LOG_DEBUG("set canaries fro array -----------------");
    IF_ERR_RETURN(error);
#endif

#ifdef IS_HASH_MEMORY_CHECK_DEFINE
    error = recalculateHashOfArray(array);
    IF_ERR_RETURN(error);
#endif

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
                       ERROR_ARRAY_BAD_INDEX);

    size_t arrInd = SIZE_OF_CANARY + elementIndex * array->elementSize;
    // FIXME: arrInd < size
    Errors error = myMemCopy(array->array + arrInd, elementVoidPtr, array->elementSize);

#ifdef IS_HASH_MEMORY_CHECK_DEFINE
    error = recalculateHashOfArray(array);
    IF_ERR_RETURN(error);
#endif

    RETURN_IF_ARR_INVALID(array);

    return error;
}

Errors getValueFromSafeArrayElement(const SafeArray* array, size_t elementIndex, void* elementVoidPtr) {
    IF_ARG_NULL_RETURN(array);
    IF_ARG_NULL_RETURN(array->array);
    IF_ARG_NULL_RETURN(elementVoidPtr);
    RETURN_IF_ARR_INVALID(array);

    IF_NOT_COND_RETURN(elementIndex < array->arraySize,
                       ERROR_ARRAY_BAD_INDEX);

    size_t arrInd = SIZE_OF_CANARY + elementIndex * array->elementSize;
    Errors error = myMemCopy(elementVoidPtr, array->array + arrInd, array->elementSize);
    IF_ERR_RETURN(error);
    RETURN_IF_ARR_INVALID(array);

    return error;
}

//  -----------------------------       SAFE ARRAY LOGGING        ----------------------------------

static Errors logArrayElement(const uint8_t* element, size_t elementSize) {
    IF_ARG_NULL_RETURN(element);
    IF_NOT_COND_RETURN(elementSize < MAX_ARRAY_ELEM_SIZE,
                       ERROR_ARRAY_SIZE_IS_TOO_BIG);

    // FIXME: move to function, make static
    char bufferToOutputStackElems[LOG_BUFFER_SIZE] = {};
    char buffForByte[4]                            = {};
    memset(bufferToOutputStackElems, 0, LOG_BUFFER_SIZE);
    memset(bufferToOutputStackElems, 0, 4);

    for (size_t byteInd = 0; byteInd < elementSize; ++byteInd) {
        uint8_t elem = *(element + byteInd);
        // FIXME: output
        // KOLYA: SAFETY!!!
        snprintf(buffForByte, 4, "%zx ", (size_t)elem); // sNNNNprintf
        strncat(bufferToOutputStackElems, buffForByte, LOG_BUFFER_SIZE); // strNNNNcat
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
        Errors error = logArrayElement(array->array + arrInd, array->elementSize);
        IF_ERR_RETURN(error);
    }
    LOG_DEBUG("--------------------------------------");

    // just in case, maybe too paranoid
    RETURN_IF_ARR_INVALID(array);

    return STATUS_OK;
}

Errors isSafeArrayValid(const SafeArray* array) {
    IF_ARG_NULL_RETURN(array);
    IF_NOT_COND_RETURN(array->arraySize == 0 || array->array != NULL,
                       ERROR_ARRAY_SIZE_EMPTY_ARRAY_NOT_ZERO_SIZE);
    IF_NOT_COND_RETURN(array->arraySize <= MAX_ARRAY_SIZE,
                       ERROR_ARRAY_SIZE_IS_TOO_BIG);
    IF_NOT_COND_RETURN(array->elementSize < MAX_ARRAY_ELEM_SIZE,
                       ERROR_ARRAY_ELEMENT_SIZE_IS_TOO_BIG);
    if (array->array == NULL)
        return STATUS_OK;

#ifdef IS_CANARY_PROTECTION_ON
    // stack smash attack (or just error) from one of the ends of struct
    size_t backCanaryIndex = SIZE_OF_CANARY + array->elementSize * array->arraySize;
    int cmpResultFront       = memcmp(array->array,                    FRONT_CANARY, SIZE_OF_CANARY);
    int cmpResultBack        = memcmp(array->array + backCanaryIndex,  BACK_CANARY,  SIZE_OF_CANARY);

    int cmpResultFrontStruct = memcmp(array->frontCanary,              FRONT_CANARY, SIZE_OF_CANARY);
    int cmpResultBackStruct  = memcmp(array->backCanary,               BACK_CANARY,  SIZE_OF_CANARY);
    //LOG_DEBUG_VARS(array->frontCanary[5], FRONT_CANARY[5]);
    // LOG_DEBUG_VARS(array->array[5], FRONT_CANARY[5]);
    // LOG_DEBUG_VARS(cmpResultFront, cmpResultBack);
    // LOG_DEBUG_VARS(cmpResultFrontStruct, cmpResultBackStruct);
    IF_NOT_COND_RETURN(cmpResultFront       == 0 && cmpResultBack       == 0 &&
                       cmpResultFrontStruct == 0 && cmpResultBackStruct == 0,
                       ERROR_ARRAY_CANARY_PROTECTION_FAILED);
#endif

#ifdef IS_HASH_MEMORY_CHECK_DEFINE
    hash_data_type correctArrayHash = 0;
    Errors err = getHashOfArray(array, &correctArrayHash);
    IF_ERR_RETURN(err);

    //LOG_DEBUG_VARS(correctArrayHash, array->structHash);
    IF_ERR_RETURN(err);
    IF_NOT_COND_RETURN(correctArrayHash == array->structHash,
                       ERROR_ARRAY_MEMORY_HASH_CHECK_FAILED);
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
