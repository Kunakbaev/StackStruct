#ifndef STACK_LIB
#define STACK_LIB

#include <inttypes.h>

#include "errorsHandler.hpp"
#include "memorySafeArray.hpp"
#include "../include/randomLib.hpp"

// FIXME: canary size divisible by 8
#define IS_HASH_MEMORY_CHECK_DEFINE
#define IS_CANARY_PROTECTION_ON

// enum StackError {
//     ERROR_STACK_INVALID_FIELD_VALUES        = 4,                  // some of stack fields are invalid
//     ERROR_STACK_INCORRECT_NUM_OF_ELEMS      = 5,                  // incorrect index of stack (usually happens during push or pop)
//     ERROR_STACK_NEW_CAPACITY_TOO_BIG        = 6,                  // new capacity is bigger than max capacity
//     ERROR_STACK_INCORRECT_CAP_KOEF          = 7,                  // incorrect capacity change koefficient
//     ERROR_STACK_MEMORY_HASH_CHECK_FAILED    = 8,                  // hashes are not equal
//     ERROR_STACK_ELEM_SIZE_TOO_SMALL         = 9,                  // size of stack element is too small (possibly set to 0)
//     ERROR_STACK_ELEM_SIZE_TOO_BIG           = 10,                 // size of stack element is too big
// };

struct Stack {
    hash_data_type frontCanary;

    hash_data_type structHash;
    size_t numberOfElements;
    SafeArray array;

    hash_data_type backCanary;
};

Errors constructStack(Stack* stack, size_t initialCapacity, size_t stackElemSize);
Errors pushElementToStack(Stack* stack, const void* element);
Errors popElementToStack(Stack* stack, void* element);
Errors isStackValid(const Stack* stack);
Errors dumpStackLog(const Stack* stack);
Errors destructStack(Stack* stack);

#endif
