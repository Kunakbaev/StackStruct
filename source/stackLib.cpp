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

// const uint64_t RAND_NUMBER_FOR_HASHES        = rnd();
const uint64_t BASE_NUMBER_FOR_HASHES        = rnd() >> 32; // ???

const uint64_t FRONT_CANARY                  = rnd();
const uint64_t  BACK_CANARY                  = rnd();

constexpr const size_t MAX_STACK_ELEM_SIZE   = 32;
constexpr const size_t MIN_STACK_CAPACITY    = 8;
constexpr const size_t MAX_STACK_CAPACITY    = 1 << 10;
constexpr const double REALLOC_SIZE_KOEF     = 2.0;
constexpr const double MIN_REALLOC_SIZE_KOEF = 1.5;

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
    err = addNumToHash(stackHash, (uint64_t*)&stack->array.arraySize);
    IF_ERR_RETURN(err);
    err = addNumToHash(stackHash, (uint64_t*)&stack->array.array); // address of a pointer
    IF_ERR_RETURN(err);
    err = addNumToHash(stackHash, (uint64_t*)&stack->array.structHash);
    IF_ERR_RETURN(err);

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

    stack->frontCanary      = FRONT_CANARY;
    stack->backCanary       = BACK_CANARY;
    stack->numberOfElements = 0;
    Errors error = constructSafeArray(initialCapacity, stackElemSize, &stack->array);
    IF_ERR_RETURN(error);

    error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    // just in case
    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

//  -----------------------------       MEMORY MANAGMENT        ----------------------------------

static double sq(double x) {
    return x * x;
}

static Errors reallocateMemoryForStackIfNeeded(Stack* stack, int newCapacity) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(REALLOC_SIZE_KOEF >= MIN_REALLOC_SIZE_KOEF,
                       ERROR_STACK_INCORRECT_CAP_KOEF);

    if (newCapacity < MIN_STACK_CAPACITY)
        newCapacity = MIN_STACK_CAPACITY;

    IF_NOT_COND_RETURN(newCapacity <= MAX_STACK_CAPACITY,
                       ERROR_STACK_NEW_CAPACITY_TOO_BIG);

    Errors error = resizeSafeArray(&stack->array, newCapacity);
    IF_ERR_RETURN(error);
    return error;
}

static Errors reallocateStackArrIfNeeded(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(REALLOC_SIZE_KOEF >= MIN_REALLOC_SIZE_KOEF,
                       ERROR_STACK_INCORRECT_CAP_KOEF);

    // there are too many unused elements in stack
    int stackCapacity = stack->array.arraySize;
    int newCapacityLess = roundl(stackCapacity / sq(REALLOC_SIZE_KOEF));
    // we need more elements, so we will make capacity of stack multiplied by some constant
    int newCapacityMore = roundl(stackCapacity * REALLOC_SIZE_KOEF);

    Errors error = STATUS_OK;
    if (stack->numberOfElements <  newCapacityLess) {
        error = reallocateMemoryForStackIfNeeded(stack, newCapacityLess);
        IF_ERR_RETURN(error);
    }
    if (stack->numberOfElements == stackCapacity) {
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

    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(element);
    RETURN_IF_INVALID(stack);

    Errors error = reallocateStackArrIfNeeded(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    IF_NOT_COND_RETURN(stack->numberOfElements + 1 <= stack->array.arraySize,
                       ERROR_STACK_INCORRECT_NUM_OF_ELEMS);

    error = setValueToSafeArrayElement(&stack->array, stack->numberOfElements, elementVoidPtr);
    IF_ERR_RETURN(error);
    ++stack->numberOfElements;

    error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

Errors popElementToStack(Stack* stack, void* elementVoidPtr) {
    uint8_t* element = (uint8_t*)elementVoidPtr;

    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(element);
    RETURN_IF_INVALID(stack);

    Errors error = reallocateStackArrIfNeeded(stack);
    IF_ERR_RETURN(error);

    IF_NOT_COND_RETURN(stack->numberOfElements >= 1,
                       ERROR_STACK_INCORRECT_NUM_OF_ELEMS);

    error = getValueFromSafeArrayElement(&stack->array, stack->numberOfElements - 1, elementVoidPtr);
    IF_ERR_RETURN(error);
    // ASK: should I clean elem after pop and how to do it better?
    //uint8_t zeroBytes[ = {};
    // error = setValueToSafeArrayElement(&stack->array, stack->numberOfElements - 1, elementVoidPtr);
    // IF_ERR_RETURN(error);

    --stack->numberOfElements;

    error = recalculateHashOfStack(stack);
    IF_ERR_RETURN(error);

    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

//  -----------------------------       CHECK IF STACK IS VALID        ----------------------------------

// ASK: or is it better to right how it's done in memorySafeArray's same function
Errors isStackValid(const Stack* stack, bool* isValid) {
    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(isValid);

    *isValid  = true;

#ifdef IS_CANARY_PROTECTION_ON
    // stack smash attack (or just error) from one of the ends of struct
    if (stack->frontCanary != FRONT_CANARY ||
        stack->backCanary  !=  BACK_CANARY) {
        LOG_ERROR("Error: canary protection failed.\n");
        *isValid = false;
        return STATUS_OK;
    }
#endif

    *isValid &= stack->array.elementSize   <  MAX_STACK_ELEM_SIZE;
    *isValid &= stack->array.elementSize   >  0;
    *isValid &= stack->numberOfElements    >= 0;
    *isValid &= stack->numberOfElements    <= stack->array.arraySize;
    *isValid &= stack->array.array != NULL || stack->numberOfElements == 0;
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

Errors dumpStackLog(const Stack* stack) {
    IF_ARG_NULL_RETURN(stack);

    RETURN_IF_INVALID(stack);

    LOG_DEBUG("--------------------------------------");
    LOG_DEBUG("Stack:");
    LOG_DEBUG_VARS(stack->numberOfElements);
    LOG_DEBUG_VARS(stack->array.arraySize);
    LOG_DEBUG_VARS(stack->structHash);

    // TODO: dump array
    Errors error = dumpArrayLog(&stack->array);
    IF_ERR_RETURN(error);
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
    Errors error = destructSafeArray(&stack->array);
    IF_ERR_RETURN(error);

    stack->numberOfElements = 0;
    stack->structHash       = 0;

    return STATUS_OK;
}
