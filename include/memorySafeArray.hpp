#ifndef MEMORY_SAFE_ARRAY
#define MEMORY_SAFE_ARRAY

#include <inttypes.h>

#include "errorsHandler.hpp"

#define IS_CANARY_PROTECTION_ON

// TODO: add ifndef canary protection, then size_of_canary = 0
const size_t SIZE_OF_CANARY = 4;

static_assert(SIZE_OF_CANARY < 32);

struct SafeArray {
    size_t   arraySize; // actual number of elements
    // (also we have 2 additional elements -> canary in the front, and in the back
    uint8_t* array;
    size_t   elementSize;
};

// TODO: add function to dump (log) array
Errors constructSafeArray(size_t arraySize, size_t elementSize, SafeArray* array);
Errors setValueToSafeArrayElement(SafeArray* array, size_t elementIndex, const void* element);
Errors getValueFromSafeArrayElement(const SafeArray* array, size_t elementIndex, void* element);
Errors resizeSafeArray(SafeArray* array, size_t newSize);
Errors isSafeArrayValid(const SafeArray* array, bool* isValid);
Errors  destructSafeArray(SafeArray* array);

#endif
