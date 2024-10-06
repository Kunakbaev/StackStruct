#ifndef MEMORY_SAFE_ARRAY
#define MEMORY_SAFE_ARRAY

#include <inttypes.h>

#include "errorsHandler.hpp"
#include "../include/randomLib.hpp"

#define IS_CANARY_PROTECTION_ON
#define IS_HASH_MEMORY_CHECK_DEFINE

// TODO: add ifndef canary protection, then size_of_canary = 0
// FIXME: does it work? seems so
#ifdef IS_CANARY_PROTECTION_ON
const size_t SIZE_OF_CANARY = 8;
#else
const size_t SIZE_OF_CANARY = 0;
#endif

// someone can accidentically put very big size
static_assert(SIZE_OF_CANARY < 32);
static_assert((SIZE_OF_CANARY & 7) == 0); // faster to work when everything is alligned (SIZE_OF_CANARY % 8 == 0)

// TODO: dynamic array for canaries??
struct SafeArray {
    uint8_t frontCanary[SIZE_OF_CANARY];

    size_t arraySize; // actual number of elements
    // (also we have 2 additional elements -> canary in the front, and in the back
    uint8_t* array;
    hash_data_type structHash;
    size_t  elementSize;

    uint8_t backCanary[SIZE_OF_CANARY];
};

// TODO: add function to dump (log) array
Errors constructSafeArray(size_t arraySize, size_t elementSize, SafeArray* array);
Errors setValueToSafeArrayElement(SafeArray* array, size_t elementIndex, const void* element);
Errors getValueFromSafeArrayElement(const SafeArray* array, size_t elementIndex, void* element);
Errors resizeSafeArray(SafeArray* array, size_t newSize);
Errors dumpArrayLog(const SafeArray* array);
Errors isSafeArrayValid(const SafeArray* array);
Errors  destructSafeArray(SafeArray* array);

#endif
