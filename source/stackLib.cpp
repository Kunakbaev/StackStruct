#include <math.h>

#include "../include/stackLib.hpp"

#define RETURN_IF_INVALID(stack)                                \
    do {                                                        \
        bool isValid = false;                                   \
        Errors errorTmp = isStackIsValid(stack, &isValid);      \
        LOG_AND_RETURN(errorTmp);                               \
        if (!isValid)                                           \
            LOG_AND_RETURN(ERROR_STACK_INVALID_FIELD_VALUES);   \
    } while (0)

const size_t MIN_STACK_CAPACITY    = 8;
const size_t MAX_STACK_CAPACITY    = 1 << 10;
const double REALLOC_SIZE_KOEF     = 2.0;
const double MIN_REALLOC_SIZE_KOEF = 1.5;

Errors constructStack(Stack* stack, int initialCapacity) {
    CHECK_ARGUMENT_FOR_NULL(stack);
    CHECK_ARG_FOR_CONDITION(initialCapacity >= 0, ERROR_INVALID_ARGUMENT);
    CHECK_ARG_FOR_CONDITION(initialCapacity < MAX_STACK_CAPACITY,
                            ERROR_INVALID_ARGUMENT);

    stack->numberOfElements = 0;
    stack->stackCapacity    = initialCapacity;
    stack->array            = NULL;

    if (initialCapacity > 0) {
        stack->array = (StackElement*)calloc(initialCapacity, sizeof(StackElement));
        CHECK_ARG_FOR_CONDITION(stack->array != NULL,
                                ERROR_MEMORY_ALLOCATION_ERROR);
    }

    return STATUS_OK;
}

static Errors myRecalloc(void* ptr, size_t newNumOfBytes) {
    CHECK_ARGUMENT_FOR_NULL(ptr);

    void* tmpPtr = realloc(ptr, newNumOfBytes);
    CHECK_ARG_FOR_CONDITION(tmpPtr, ERROR_MEMORY_REALLOCATION_ERROR);
    ptr = tmpPtr;

    // TODO:
    // memset(ptr, 0, newNumOfBytes);

    return STATUS_OK;
}

static int sq(int x) {
    return x * x;
}

Errors reallocateStackArrIfNeeded(Stack* stack) {
    CHECK_ARGUMENT_FOR_NULL(stack);
    CHECK_ARG_FOR_CONDITION(REALLOC_SIZE_KOEF > MIN_REALLOC_SIZE_KOEF,
                            ERROR_STACK_INCORRECT_CAP_KOEF);

    Errors error = STATUS_OK;
    // we need more elements, so we will make capacity of stack multiplied by some constant
    if (stack->numberOfElements == stack->stackCapacity) {
        int newCapacity = roundl(stack->stackCapacity * REALLOC_SIZE_KOEF);
        CHECK_ARG_FOR_CONDITION(newCapacity <= MAX_STACK_CAPACITY,
                                ERROR_STACK_NEW_CAPACITY_TOO_BIG);

        error = myRecalloc(stack->array, newCapacity * sizeof(StackElement));
        LOG_AND_RETURN(error);
        return STATUS_OK;
    }

    int newCapacity = roundl(stack->stackCapacity / REALLOC_SIZE_KOEF);
    if (stack->numberOfElements <= newCapacity) {
        error = myRecalloc(stack->array, newCapacity * sizeof(StackElement));
        LOG_AND_RETURN(error);
    }

    return STATUS_OK;
}

Errors pushElementToStack(Stack* stack, const StackElement element) {
    CHECK_ARGUMENT_FOR_NULL(stack);
    CHECK_ARGUMENT_FOR_NULL(element);

    // FIXME: copypaste
    RETURN_IF_INVALID(stack);

    CHECK_ARG_FOR_CONDITION(stack->numberOfElements + 1 <= stack->stackCapacity,
                            ERROR_STACK_INCORRECT_NUM_OF_ELEMS);

    // FIXME: copy is bad, maybe make stack of pointers to elements
    stack->array[stack->numberOfElements] = element;
    ++stack->numberOfElements;
    Errors error = reallocateStackArrIfNeeded(stack);
    LOG_AND_RETURN(error);

    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

Errors popElementToStack(Stack* stack, StackElement* element) {
    CHECK_ARGUMENT_FOR_NULL(stack);
    CHECK_ARGUMENT_FOR_NULL(element);

    RETURN_IF_INVALID(stack);

    CHECK_ARG_FOR_CONDITION(stack->numberOfElements >= 1,
                            ERROR_STACK_INCORRECT_NUM_OF_ELEMS);
    *element = stack->array[stack->numberOfElements - 1];
    --stack->numberOfElements;

    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}

Errors isStackIsValid(Stack* stack, bool* isValid) {
    CHECK_ARGUMENT_FOR_NULL(stack);
    CHECK_ARGUMENT_FOR_NULL(isValid);

    // TODO:
    *isValid  = true;
    *isValid &= stack->numberOfElements >= 0;
    *isValid &= stack->numberOfElements <= stack->stackCapacity;
    *isValid &= stack->array            != NULL;

    return STATUS_OK;
}

Errors dumpStackLog(Stack* stack) {
    CHECK_ARGUMENT_FOR_NULL(stack);

    RETURN_IF_INVALID(stack);

    LOG_DEBUG("--------------------------------------");
    LOG_DEBUG("Stack:");
    LOG_DEBUG_VARS(stack->numberOfElements);
    LOG_DEBUG_VARS(stack->stackCapacity);
    LOG_DEBUG("elements:");
    for (size_t elemIndex = 0; elemIndex < stack->numberOfElements; ++elemIndex) {
        LOG_DEBUG_VARS(elemIndex, stack->array[elemIndex]);
    }

    // just in case, maybe too paranoid
    RETURN_IF_INVALID(stack);

    return STATUS_OK;
}
