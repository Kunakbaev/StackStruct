#include <random>
#include <inttypes.h>

#include "../include/randomLib.hpp"

//std::mt19937_64 rnd2(std::chrono::steady_clock::now().time_since_epoch().count());
// easier for debug with constant seed
// TODO: return dynamic seed
std::mt19937_64 rnd(228);

// WARNING: be carefull here, if HASH_DATA_TYPE is too small, this will be 0
const HASH_DATA_TYPE BASE_NUMBER_FOR_HASHES = getRandomUint64tNumber() >> 32; // ???

HASH_DATA_TYPE getRandomUint64tNumber() {
    return rnd();
}

Errors fillSequenceOfBytesWithRandomValues(void* elementVoidPtr, size_t memToFill) {
    uint8_t* element = (uint8_t*)(elementVoidPtr);

    IF_ARG_NULL_RETURN(element);

    for (size_t byteInd = 0; byteInd < memToFill; ++byteInd) {
        element[byteInd] = (uint8_t)getRandomUint64tNumber();
    }

    return STATUS_OK;
}

// maybe should add module by some random number, it will be safer, but slower
Errors addNumToHash(HASH_DATA_TYPE* hash, const HASH_DATA_TYPE number) {
    IF_ARG_NULL_RETURN(hash);

    *hash *= BASE_NUMBER_FOR_HASHES;
    *hash += number + 1;

    return STATUS_OK;
}

Errors getHashOfSequenceOfBytes(const void* elementVoidPtr, size_t sizeOfSeqMemory, HASH_DATA_TYPE* hash) {
    const uint8_t* element = (const uint8_t*)(elementVoidPtr);

    // ASK: is this ok?
    IF_ARG_NULL_RETURN(hash);
    if (element == NULL) {
        *hash = 0;
        return STATUS_OK;
    }

    IF_ARG_NULL_RETURN(element);

    // LOG_DEBUG_VARS(element, element[0], sizeOfSeqMemory);

    Errors error = STATUS_OK;
    for (size_t byteInd = 0; byteInd < sizeOfSeqMemory; ++byteInd) {
        uint8_t byte = element[byteInd];
        error = addNumToHash(hash, (HASH_DATA_TYPE)(byte));
        IF_ERR_RETURN(error);
    }
    // LOG_DEBUG_VARS(*hash);

    return STATUS_OK;
}
