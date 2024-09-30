#include <math.h>
#include <random>
#include <chrono>

#include "../include/stackLib.hpp"

#define RETURN_IF_INVALID(stack)                                \
    do {                                                        \
        bool isValid = false;                                   \
        Errors errorTmp = isStackValid(stack, &isValid);        \
        IF_ERR_RETURN(errorTmp);                                \
        if (!isValid) {                                         \
            IF_ERR_RETURN(ERROR_STACK_INVALID_FIELD_VALUES);    \
            /*assert(false);                                 */ \
        }                                                       \
    } while (0)


// beware strings of Tue a Morse
std::mt19937_64 rnd(std::chrono::steady_clock::now().time_since_epoch().count());

const uint64_t RAND_NUMBER_FOR_HASHES        = rnd();
const uint64_t BASE_NUMBER_FOR_HASHES        = rnd() >> 32; // ???

const uint64_t FRONT_CANARY                  = rnd();
const uint64_t  BACK_CANARY                  = rnd();

constexpr const size_t MAX_STACK_ELEM_SIZE   = 32;
constexpr const size_t MIN_STACK_CAPACITY    = 8;
constexpr const size_t MAX_STACK_CAPACITY    = 1 << 10;
constexpr const double REALLOC_SIZE_KOEF     = 2.0;
constexpr const double MIN_REALLOC_SIZE_KOEF = 1.5;

const size_t LOG_BUFFER_SIZE                 = 200;
char* bufferToOutputStackElems               = "";
char* buffForByte                            = "";

static_assert(MIN_STACK_CAPACITY    > 0);
static_assert(MIN_REALLOC_SIZE_KOEF > 1);
static_assert(REALLOC_SIZE_KOEF     > MIN_REALLOC_SIZE_KOEF);

//  -----------------------------       FUNCTIONS FOR HASHES        ----------------------------------

static Errors addNumToHash(uint64_t* stackHash, const uint64_t* number) {
    IF_ARG_NULL_RETURN(stackHash);
    IF_ARG_NULL_RETURN(number);

    *stackHash *= BASE_NUMBER_FOR_HASHES;
    *stackHash += *number + 1;

    return STATUS_OK;
}

static Errors getHashOfStack(const Stack* stack, uint64_t* stackHash) {
    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(stackHash);

    *stackHash = 0;
    Errors err = STATUS_OK;
    err = addNumToHash(stackHash, (uint64_t*)&stack->numberOfElements);
    IF_ERR_RETURN(err);
    addNumToHash(stackHash,       (uint64_t*)&stack->stackCapacity);
    IF_ERR_RETURN(err);
    addNumToHash(stackHash,       (uint64_t*)&stack->array); // address of a pointer
    IF_ERR_RETURN(err);

    // for (size_t elemInd = 0; elemInd < stack->numberOfElements; ++elemInd) {
    //     err = addNumToHash(stackHash, (uint64_t*)&stack->array[elemInd]);
    //     IF_ERR_RETURN(err);
    // }
    for (size_t byteInd = 0; byteInd < stack->numberOfElements * stack->elementSize; ++byteInd) {
        err = addNumToHash(stackHash, (uint64_t*)&stack->array[byteInd]);
        IF_ERR_RETURN(err);
    }

    return STATUS_OK;
}

static Errors recalculateHashOfStack(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);
    // RETURN_IF_INVALID(stack);

    Errors err = getHashOfStack(stack, &stack->structHash);
    IF_ERR_RETURN(err);

    return STATUS_OK;
}

//  -----------------------------       STACK CONSTRUCTOR        ----------------------------------

Errors constructStack(Stack* stack, int initialCapacity, size_t stackElemSize) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(initialCapacity >= 0, ERROR_INVALID_ARGUMENT);
    IF_NOT_COND_RETURN(stackElemSize   >  0, ERROR_STACK_ELEM_SIZE_TOO_SMALL);
    LOG_DEBUG_VARS(stackElemSize);
    IF_NOT_COND_RETURN(stackElemSize   <= MAX_STACK_ELEM_SIZE,
                       ERROR_STACK_ELEM_SIZE_TOO_BIG);
    IF_NOT_COND_RETURN(initialCapacity <  MAX_STACK_CAPACITY,
                       ERROR_INVALID_ARGUMENT);

    stack->frontCanary = FRONT_CANARY;
    stack->numberOfElements = 0;
    stack->stackCapacity    = initialCapacity;
    stack->array            = NULL;
    stack->elementSize      = stackElemSize;
    stack->backCanary  = BACK_CANARY;

    if (initialCapacity > 0) {
        stack->array = (uint8_t*)calloc(initialCapacity, sizeof(uint8_t));
        IF_NOT_COND_RETURN(stack->array != NULL,
                           ERROR_MEMORY_ALLOCATION_ERROR);
    }

    bufferToOutputStackElems = (char*)calloc(LOG_BUFFER_SIZE, sizeof(char));
    IF_NOT_COND_RETURN(bufferToOutputStackElems != NULL,
                       ERROR_MEMORY_ALLOCATION_ERROR);
    buffForByte = (char*)calloc(4, sizeof(char*));
    IF_NOT_COND_RETURN(buffForByte != NULL,
                       ERROR_MEMORY_ALLOCATION_ERROR);

    Errors error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    // just in case
    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

//  -----------------------------       MEMORY MANAGMENT        ----------------------------------

static Errors myRecalloc(void** ptr, size_t newNumOfBytes) {
    IF_ARG_NULL_RETURN(ptr);

    // LOG_DEBUG_VARS("Reallocating", newNumOfBytes);
    void* tmpPtr = realloc(*ptr, newNumOfBytes);
    IF_NOT_COND_RETURN(tmpPtr, ERROR_MEMORY_REALLOCATION_ERROR);
    *ptr = tmpPtr;
    // LOG_DEBUG_VARS(tmpPtr, ptr);

    // TODO:
    // memset(ptr, 0, newNumOfBytes);

    return STATUS_OK;
}

static double sq(double x) {
    return x * x;
}

static Errors reallocateMemoryForStackIfNeeded(Stack* stack, int newCapacity) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(REALLOC_SIZE_KOEF >= MIN_REALLOC_SIZE_KOEF,
                       ERROR_STACK_INCORRECT_CAP_KOEF);

    if (newCapacity < MIN_STACK_CAPACITY)
        newCapacity = MIN_STACK_CAPACITY;

    if (newCapacity == stack->stackCapacity)
        return STATUS_OK;

    IF_NOT_COND_RETURN(newCapacity <= MAX_STACK_CAPACITY,
                       ERROR_STACK_NEW_CAPACITY_TOO_BIG);

    Errors error = myRecalloc((void**)&stack->array,
                              newCapacity * stack->elementSize);
    stack->stackCapacity = newCapacity;
    IF_ERR_RETURN(error);
    return error;
}

static Errors reallocateStackArrIfNeeded(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(REALLOC_SIZE_KOEF >= MIN_REALLOC_SIZE_KOEF,
                       ERROR_STACK_INCORRECT_CAP_KOEF);

    // there are too many unused elements in stack
    int newCapacityLess = roundl(stack->stackCapacity / sq(REALLOC_SIZE_KOEF));
    // we need more elements, so we will make capacity of stack multiplied by some constant
    int newCapacityMore = roundl(stack->stackCapacity * REALLOC_SIZE_KOEF);

    Errors error = STATUS_OK;
    if (stack->numberOfElements <  newCapacityLess) {
        error = reallocateMemoryForStackIfNeeded(stack, newCapacityLess);
        IF_ERR_RETURN(error);
    }
    if (stack->numberOfElements == stack->stackCapacity) {
        error = reallocateMemoryForStackIfNeeded(stack, newCapacityMore);
        IF_ERR_RETURN(error);
    }

    error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    return STATUS_OK;
}

//  -----------------------------       TWO MAIN FUNCTIONS        ----------------------------------

Errors pushElementToStack(Stack* stack, const void* elementVoidPtr) {
    uint8_t* element = (uint8_t*)elementVoidPtr;
    // LOG_DEBUG_VARS(element);

    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(element);
    //IF_ARG_NULL_RETURN(*element);
    RETURN_IF_INVALID(stack);

    Errors error = reallocateStackArrIfNeeded(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    IF_NOT_COND_RETURN(stack->numberOfElements + 1 <= stack->stackCapacity,
                       ERROR_STACK_INCORRECT_NUM_OF_ELEMS);

    // FIXME: copy is bad, maybe make stack of pointers to elements
    for (size_t byteInd = 0; byteInd < stack->elementSize; ++byteInd) {
        //LOG_DEBUG_VARS(byteInd, *(element + byteInd));
        int arrInd = stack->numberOfElements * stack->elementSize + byteInd;
        stack->array[arrInd] = *(element + byteInd);
    }
    ++stack->numberOfElements;

    error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

Errors popElementToStack(Stack* stack, void* elementVoidPtr) {
    uint8_t* element = (uint8_t*)elementVoidPtr;
    // LOG_DEBUG_VARS(element);

    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(element);

    RETURN_IF_INVALID(stack);
    IF_NOT_COND_RETURN(stack->array != NULL,
                       ERROR_STACK_INVALID_FIELD_VALUES);

    Errors error = reallocateStackArrIfNeeded(stack);
    IF_ERR_RETURN(error);

    IF_NOT_COND_RETURN(stack->numberOfElements >= 1,
                       ERROR_STACK_INCORRECT_NUM_OF_ELEMS);

    for (size_t byteInd = 0; byteInd < stack->elementSize; ++byteInd) {
        int arrInd = (stack->numberOfElements - 1) * stack->elementSize + byteInd;
        //LOG_DEBUG_VARS(arrInd, *element, stack->array[arrInd]);
        *(element + byteInd) = stack->array[arrInd];
    }
    --stack->numberOfElements;

    error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

//  -----------------------------       CHECK IF STACK IS VALID        ----------------------------------

Errors isStackValid(const Stack* stack, bool* isValid) {
    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(isValid);

    // TODO:
    *isValid  = true;

#ifdef IS_CANARY_PROTECTION_ON
    // stack smash attack (or just error) from one of the ends of struct
    if (stack->frontCanary != FRONT_CANARY ||
        stack->backCanary  !=  BACK_CANARY) {
        LOG_ERROR("Error: canary protection failed.\n");
        return STATUS_OK;
    }
#endif

    *isValid &= stack->elementSize      <  MAX_STACK_ELEM_SIZE;
    *isValid &= stack->elementSize      >  0;
    *isValid &= stack->numberOfElements >= 0;
    *isValid &= stack->numberOfElements <= stack->stackCapacity;
    *isValid &= stack->array != NULL    || stack->numberOfElements == 0;
    if (!(*isValid))
        return STATUS_OK;

#ifdef IS_HASH_MEMORY_CHECK_DEFINE
    uint64_t correctHashStack = 0;
    Errors err = getHashOfStack(stack, &correctHashStack);
    IF_ERR_RETURN(err);

    // LOG_DEBUG_VARS(correctHashStack, stack->structHash);
    IF_ERR_RETURN(err);
    IF_NOT_COND_RETURN(correctHashStack == stack->structHash,
                       ERROR_STACK_MEMORY_HASH_CHECK_FAILED);
#endif

    return STATUS_OK;
}

//  -----------------------------       STACK LOGGING        ----------------------------------

Errors logStackElement(const uint8_t* element, size_t elementSize) {
    IF_ARG_NULL_RETURN(element);
    IF_NOT_COND_RETURN(elementSize < MAX_STACK_ELEM_SIZE,
                       ERROR_STACK_ELEM_SIZE_TOO_BIG);

    memset(bufferToOutputStackElems, 0, LOG_BUFFER_SIZE);
    for (size_t byteInd = 0; byteInd < elementSize; ++byteInd) {
        uint8_t elem = *(element + byteInd);
        //LOG_DEBUG_VARS(buffForByte);
        sprintf(buffForByte, "%p ", elem);
        strcat(bufferToOutputStackElems, buffForByte);
        //LOG_DEBUG_VARS(byteInd, elem);
    }
    LOG_DEBUG(bufferToOutputStackElems);

    return STATUS_OK;
}

Errors dumpStackLog(const Stack* stack) {
    IF_ARG_NULL_RETURN(stack);

    RETURN_IF_INVALID(stack);

    LOG_DEBUG("--------------------------------------");
    LOG_DEBUG("Stack:");
    LOG_DEBUG_VARS(stack->numberOfElements);
    LOG_DEBUG_VARS(stack->stackCapacity);
    LOG_DEBUG_VARS(stack->structHash);
    LOG_DEBUG_VARS(stack->array);
    LOG_DEBUG("elements:");
    for (size_t elemIndex = 0; elemIndex < stack->numberOfElements; ++elemIndex) {
        size_t arrInd = elemIndex * stack->elementSize;
        // FIXME: somehow output bytes
        assert(stack->elementSize == 4);

        //LOG_DEBUG_VARS(elemIndex, (int)stack->array[arrInd]);
        LOG_DEBUG_VARS(elemIndex);

        Errors error = logStackElement(stack->array + arrInd, stack->elementSize);
        IF_ERR_RETURN(error);
    }
    LOG_DEBUG("--------------------------------------");

    // just in case, maybe too paranoid
    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}



//  -----------------------------       STACK DESTRUCTOR        ----------------------------------

Errors destructStack(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);

    RETURN_IF_INVALID(stack);

    LOG_DEBUG("destrucing stack");
    FREE(stack->array);
    stack->numberOfElements = 0;
    stack->stackCapacity    = 0;
    stack->structHash       = 0;
    FREE(bufferToOutputStackElems);
    FREE(buffForByte);

    return STATUS_OK;
}
