#include <math.h>
#include <random>
#include <chrono>

#include "../include/stackLib.hpp"

#define RETURN_IF_INVALID(stack)                                \
    do {                                                        \
        Errors errorTmp = isStackValid(stack);                  \
        IF_ERR_RETURN(errorTmp);                                \
    } while (0)

const HASH_DATA_TYPE BASE_NUMBER_FOR_HASHES        = getRandomUint64tNumber() >> 32; // ???

const HASH_DATA_TYPE FRONT_CANARY                  = getRandomUint64tNumber();
const HASH_DATA_TYPE  BACK_CANARY                  = getRandomUint64tNumber();

constexpr const size_t MAX_STACK_ELEM_SIZE   = 32;
constexpr const size_t MIN_STACK_CAPACITY    = 8;
constexpr const size_t MAX_STACK_CAPACITY    = 1 << 10;
constexpr const double REALLOC_SIZE_KOEF     = 2.0;
constexpr const double MIN_REALLOC_SIZE_KOEF = 1.5;

static_assert(MIN_STACK_CAPACITY    > 0);
static_assert(MIN_REALLOC_SIZE_KOEF > 1);
static_assert(REALLOC_SIZE_KOEF     > MIN_REALLOC_SIZE_KOEF);

//  -----------------------------       FUNCTIONS FOR HASHES        ----------------------------------

static Errors getHashOfStack(const Stack* stack, HASH_DATA_TYPE* stackHash) {
    IF_ARG_NULL_RETURN(stack);
    IF_ARG_NULL_RETURN(stackHash);

    // ASK: I have a function that returns hash of sequence of bytes, but
    // I don't want it to take hash into consideration
    // I don't want to make stack argument NOT const and another option was to put
    // structHash field to the end of structure, but OFFSET_OF_FIELD was ver interesting
    // and also that's a new information for me
    *stackHash = 0;

    size_t bytesBeforeHash = OFFSET_OF_FIELD_I_COPIED_IT_FROM_WIKIPEDIA(Stack, structHash);
    LOG_DEBUG_VARS(bytesBeforeHash);

    HASH_DATA_TYPE firstHalf  = 0;
    HASH_DATA_TYPE secondHalf = 0;
    Errors error = STATUS_OK;
    error = getHashOfSequenceOfBytes(stack, bytesBeforeHash, &firstHalf);
    IF_ERR_RETURN(error);
    error = getHashOfSequenceOfBytes(stack + bytesBeforeHash + sizeof(stack->structHash),
                                     bytesBeforeHash, &secondHalf);
    IF_ERR_RETURN(error);
    LOG_DEBUG_VARS(firstHalf, secondHalf);
    *stackHash = firstHalf * secondHalf; // multipilication with overflow
    //*stackHash = firstHalf ^ secondHalf;

    return STATUS_OK;
}

static Errors recalculateHashOfStack(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);

    Errors err = getHashOfStack(stack, &stack->structHash);
    IF_ERR_RETURN(err);

    // ASK: should I check that array hash is good here too?
    // Errors err = getHashOfArray();

    return STATUS_OK;
}

//  -----------------------------       STACK CONSTRUCTOR        ----------------------------------

Errors constructStack(Stack* stack, size_t initialCapacity, size_t stackElemSize) {
    IF_ARG_NULL_RETURN(stack);
    //IF_NOT_COND_RETURN(initialCapacity >= 0, ERROR_INVALID_ARGUMENT);
    IF_NOT_COND_RETURN(stackElemSize   >  0, ERROR_STACK_ELEM_SIZE_TOO_SMALL);
    LOG_DEBUG_VARS(stackElemSize);
    IF_NOT_COND_RETURN(stackElemSize   <= MAX_STACK_ELEM_SIZE,
                       ERROR_STACK_ELEM_SIZE_TOO_BIG);
    IF_NOT_COND_RETURN(initialCapacity <  MAX_STACK_CAPACITY,
                       ERROR_INVALID_ARGUMENT);

    LOG_INFO("constructing stack");

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

static Errors reallocateMemoryForStackIfNeeded(Stack* stack, size_t newCapacity) {
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

static double squareNumber(double x) {
    return x * x;
}

static Errors reallocateStackArrIfNeeded(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);
    IF_NOT_COND_RETURN(REALLOC_SIZE_KOEF >= MIN_REALLOC_SIZE_KOEF,
                       ERROR_STACK_INCORRECT_CAP_KOEF);

    // there are too many unused elements in stack
    // FIXME: rename sq
    size_t stackCapacity = stack->array.arraySize;
    size_t newCapacityLess = (size_t)roundl((long double)stackCapacity / squareNumber(REALLOC_SIZE_KOEF));
    // we need more elements, so we will make capacity of stack multiplied by some constant
    size_t newCapacityMore = (size_t)roundl((long double)stackCapacity * REALLOC_SIZE_KOEF);

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
    const uint8_t* element = (const uint8_t*)elementVoidPtr;

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

Errors isStackValid(const Stack* stack) {
    IF_ARG_NULL_RETURN(stack);

#ifdef IS_CANARY_PROTECTION_ON
    // stack smash attack (or just error) from one of the ends of struct
    IF_NOT_COND_RETURN(stack->frontCanary == FRONT_CANARY &&
                       stack->backCanary  ==  BACK_CANARY,
                       ERROR_STACK_CANARY_PROTECTION_FAILED);
#endif

    LOG_DEBUG_VARS(stack->array.arraySize, stack->numberOfElements);
    IF_NOT_COND_RETURN(stack->array.elementSize   <  MAX_STACK_ELEM_SIZE,
                       ERROR_STACK_ARRAY_SIZE_IS_TOO_BIG);
    IF_NOT_COND_RETURN(stack->array.elementSize > 0,
                       ERROR_STACK_ELEM_SIZE_TOO_SMALL);
    IF_NOT_COND_RETURN(stack->array.elementSize < MAX_STACK_ELEM_SIZE,
                       ERROR_STACK_ELEM_SIZE_TOO_BIG);
    IF_NOT_COND_RETURN(stack->numberOfElements  <= stack->array.arraySize,
                       ERROR_STACK_INVALID_NUM_OF_ELEMS);
    IF_NOT_COND_RETURN(stack->array.arraySize   == 0 || stack->array.array != NULL,
                       ERROR_ARRAY_SIZE_EMPTY_ARRAY_NOT_ZERO_SIZE);
    Errors error = isSafeArrayValid(&stack->array);
    IF_ERR_RETURN(error);

#ifdef IS_HASH_MEMORY_CHECK_DEFINE
    HASH_DATA_TYPE correctHashStack = 0;
    error = getHashOfStack(stack, &correctHashStack);
    IF_ERR_RETURN(error);

    // LOG_DEBUG_VARS(correctHashStack, stack->structHash);
    IF_ERR_RETURN(error);
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

    Errors error = dumpArrayLog(&stack->array);
    IF_ERR_RETURN(error);

    // just in case, maybe too paranoid
    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}



//  -----------------------------       STACK DESTRUCTOR        ----------------------------------

Errors destructStack(Stack* stack) {
    IF_ARG_NULL_RETURN(stack);

    RETURN_IF_INVALID(stack);

    LOG_INFO("destructing stack");
    Errors error = destructSafeArray(&stack->array);
    IF_ERR_RETURN(error);

    stack->numberOfElements = 0;
    stack->structHash       = 0;

    return STATUS_OK;
}
