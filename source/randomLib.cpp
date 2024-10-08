#include <random>
#include <inttypes.h>
#include <chrono>

#include "../include/randomLib.hpp"

//std::mt19937_64 rnd2(std::chrono::steady_clock::now().time_since_epoch().count());
// easier for debug with constant seed
// TODO: return dynamic seed
std::mt19937_64 rnd((size_t)std::chrono::steady_clock::now().time_since_epoch().count());

// WARNING: be carefull here, if hash_data_type is too small, this will be 0
const hash_data_type BASE_NUMBER_FOR_HASHES = getRandomUint64tNumber() >> 32; // ???

hash_data_type getRandomUint64tNumber() {
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
Errors addNumToHash(hash_data_type* hash, const hash_data_type number) {
    IF_ARG_NULL_RETURN(hash);

    *hash *= BASE_NUMBER_FOR_HASHES;
    *hash += number + 1;

    return STATUS_OK;
}

Errors getHashOfSequenceOfBytes(const void* elementVoidPtr, size_t sizeOfSeqMemory, hash_data_type* hash) {
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
        error = addNumToHash(hash, (hash_data_type)(byte));
        IF_ERR_RETURN(error);
    }
    // LOG_DEBUG_VARS(*hash);

    return STATUS_OK;
}

Errors getHashOfStruct(const void* structObjVoidPtr, size_t sizeOfStruct, size_t sizeOfHash,
                       size_t bytesBeforeHash, hash_data_type* hash) {
    const uint8_t* structObj = (const uint8_t*)structObjVoidPtr;

    IF_ARG_NULL_RETURN(structObj);
    IF_ARG_NULL_RETURN(hash);
    // ASK: I have a function that returns hash of sequence of bytes, but
    // I don't want it to take hash into consideration
    // I don't want to make stack argument NOT const and another option was to put
    // structHash field to the end of structure, but OFFSET_OF_FIELD was ver interesting
    // and also that's a new information for me
    *hash = 0;

    //LOG_DEBUG_VARS(sizeOfHash, sizeOfStruct, bytesBeforeHash, hash);

    hash_data_type firstHalf  = 0;
    hash_data_type secondHalf = 0;
    Errors error = STATUS_OK;
    error = getHashOfSequenceOfBytes(structObj, bytesBeforeHash, &firstHalf);
    IF_ERR_RETURN(error);
    error = getHashOfSequenceOfBytes(structObj + bytesBeforeHash + sizeOfHash,
                                     sizeOfStruct - bytesBeforeHash - sizeOfHash, &secondHalf);
    IF_ERR_RETURN(error);
    //LOG_DEBUG_VARS(firstHalf, secondHalf);
    *hash = firstHalf * secondHalf; // multipilication with overflow
    //*stackHash = firstHalf ^ secondHalf;

    return STATUS_OK;
}
