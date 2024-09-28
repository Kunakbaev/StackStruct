#include <math.h>
#include <random>
#include <chrono>

#include "../include/stackLib.hpp"

#define RETURN_IF_INVALID(stack)                                \
    do {                                                        \
        bool isValid = false;                                   \
        Errors errorTmp = isStackIsValid(stack, &isValid);      \
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

constexpr const size_t MIN_STACK_CAPACITY    = 8;
constexpr const size_t MAX_STACK_CAPACITY    = 1 << 10;
constexpr const double REALLOC_SIZE_KOEF     = 2.0;
constexpr const double MIN_REALLOC_SIZE_KOEF = 1.5;

static_assert(MIN_STACK_CAPACITY    > 0);
static_assert(MIN_REALLOC_SIZE_KOEF > 1);
static_assert(REALLOC_SIZE_KOEF     > MIN_REALLOC_SIZE_KOEF);

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

Errors constructStack(Stack* stack, int initialCapacity, size_t stackElemSize) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(initialCapacity >= 0, ERROR_INVALID_ARGUMENT);
    IF_NOT_COND_RETURN(initialCapacity < MAX_STACK_CAPACITY,
                       ERROR_INVALID_ARGUMENT);

    stack->numberOfElements = 0;
    stack->stackCapacity    = initialCapacity;
    stack->array            = NULL;
    stack->elementSize      = stackElemSize;

    if (initialCapacity > 0) {
        stack->array = (uint8_t*)calloc(initialCapacity, sizeof(uint8_t));
        IF_NOT_COND_RETURN(stack->array != NULL,
                           ERROR_MEMORY_ALLOCATION_ERROR);
    }

    Errors error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    return STATUS_OK;
}

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

static Errors reallocateMoreMemoryForStackIfNeeded(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(REALLOC_SIZE_KOEF > MIN_REALLOC_SIZE_KOEF,
                       ERROR_STACK_INCORRECT_CAP_KOEF);

    // we need more elements, so we will make capacity of stack multiplied by some constant
    int newCapacity = roundl(stack->stackCapacity * REALLOC_SIZE_KOEF);
    if (newCapacity < MIN_STACK_CAPACITY)
        newCapacity = MIN_STACK_CAPACITY;
    if (stack->numberOfElements < stack->stackCapacity ||
            newCapacity == stack->stackCapacity)
        return STATUS_OK; // this happens when array is still empty, but already has some capacity
    IF_NOT_COND_RETURN(newCapacity <= MAX_STACK_CAPACITY,
                       ERROR_STACK_NEW_CAPACITY_TOO_BIG);

    stack->stackCapacity = newCapacity;
    Errors error = myRecalloc((void**)&stack->array,
                              newCapacity * stack->elementSize);
    IF_ERR_RETURN(error);
    return STATUS_OK;
}

static Errors reallocateLessMemoryForStackIfNeeded(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(REALLOC_SIZE_KOEF > MIN_REALLOC_SIZE_KOEF,
                       ERROR_STACK_INCORRECT_CAP_KOEF);

    // there are too many unused elements in stack
    int newCapacity = roundl(stack->stackCapacity / sq(REALLOC_SIZE_KOEF));
    if (newCapacity < MIN_STACK_CAPACITY)
        newCapacity = MIN_STACK_CAPACITY;

    if (stack->numberOfElements > newCapacity ||
            newCapacity == stack->stackCapacity)
        return STATUS_OK;

    Errors error = myRecalloc((void**)&stack->array,
                              newCapacity * stack->elementSize);
    stack->stackCapacity = newCapacity;
    IF_ERR_RETURN(error);
    return error;
}

static Errors reallocateStackArrIfNeeded(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(REALLOC_SIZE_KOEF > MIN_REALLOC_SIZE_KOEF,
                       ERROR_STACK_INCORRECT_CAP_KOEF);

    Errors error = reallocateMoreMemoryForStackIfNeeded(stack);
    IF_ERR_RETURN(error);

    error        = reallocateLessMemoryForStackIfNeeded(stack);
    IF_ERR_RETURN(error);

    error        = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    return STATUS_OK;
}

Errors pushElementToStack(Stack* stack, const void* elementVoidPtr) {
    uint8_t* element = (uint8_t*)elementVoidPtr;
    LOG_DEBUG_VARS(element);

    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(element);
    IF_ARG_NULL_RETURN(*element);
    LOG_DEBUG("ok");
    RETURN_IF_INVALID(stack);

    Errors error = reallocateStackArrIfNeeded(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    IF_NOT_COND_RETURN(stack->numberOfElements + 1 <= stack->stackCapacity,
                       ERROR_STACK_INCORRECT_NUM_OF_ELEMS);

    // FIXME: copy is bad, maybe make stack of pointers to elements
    // FIXME: does it work?
    for (size_t byteInd = 0; byteInd < stack->elementSize; ++byteInd) {
        LOG_DEBUG_VARS(byteInd, *(element + byteInd));
        int arrInd = stack->numberOfElements * stack->elementSize + byteInd;
        stack->array[arrInd] = *(element + byteInd);
    }
    LOG_DEBUG("ok");
    ++stack->numberOfElements;

    error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

Errors popElementToStack(Stack* stack, void* elementVoidPtr) {
    uint8_t* element = (uint8_t*)elementVoidPtr;
    LOG_DEBUG_VARS(element);

    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(element);

    RETURN_IF_INVALID(stack);
    IF_NOT_COND_RETURN(stack->array != NULL,
                       ERROR_STACK_INVALID_FIELD_VALUES);

    Errors error = reallocateStackArrIfNeeded(stack);
    IF_ERR_RETURN(error);

    IF_NOT_COND_RETURN(stack->numberOfElements >= 1,
                       ERROR_STACK_INCORRECT_NUM_OF_ELEMS);
    //*element = stack->array[stack->numberOfElements - 1];

    for (size_t byteInd = 0; byteInd < stack->elementSize; ++byteInd) {
        int arrInd = (stack->numberOfElements - 1) * stack->elementSize + byteInd;
        LOG_DEBUG_VARS(arrInd, *element, stack->array[arrInd]);
        *(element + byteInd) = stack->array[arrInd];
    }
    --stack->numberOfElements;

    error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

Errors isStackIsValid(Stack* stack, bool* isValid) {
    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(isValid);

    // TODO:
    *isValid  = true;
    *isValid &= stack->numberOfElements >= 0;
    *isValid &= stack->numberOfElements <= stack->stackCapacity;
    *isValid &= stack->array != NULL    || stack->numberOfElements == 0;
    if (!(*isValid))
        return STATUS_OK;

#ifdef IS_HASH_MEMORY_CHECK_DEFINE
    uint64_t correctHashStack = 0;
    Errors err = getHashOfStack(stack, &correctHashStack);
    IF_ERR_RETURN(err);

    LOG_DEBUG_VARS(correctHashStack, stack->structHash);
    IF_ERR_RETURN(err);
    IF_NOT_COND_RETURN(correctHashStack == stack->structHash,
                       ERROR_STACK_MEMORY_HASH_CHECK_FAILED);
#endif

    return STATUS_OK;
}

Errors dumpStackLog(Stack* stack) {
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
        LOG_DEBUG_VARS(elemIndex, (int)stack->array[arrInd]);
    }
    LOG_DEBUG("--------------------------------------");

    // just in case, maybe too paranoid
    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

Errors destructStack(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);

    RETURN_IF_INVALID(stack);

    FREE(stack->array);
    stack->numberOfElements = 0;
    stack->stackCapacity    = 0;
    stack->structHash       = 0;

    return STATUS_OK;
}
