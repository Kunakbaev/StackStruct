#ifndef RANDOM_LIB
#define RANDOM_LIB

#include "errorsHandler.hpp"

typedef uint64_t HASH_DATA_TYPE;

HASH_DATA_TYPE getRandomUint64tNumber();
Errors addNumToHash(HASH_DATA_TYPE* hash, const HASH_DATA_TYPE number);
Errors fillSequenceOfBytesWithRandomValues(void* elementVoidPtr, size_t memToFill);
Errors getHashOfSequenceOfBytes(const void* elementVoidPtr, size_t sizeOfSeqMemory, HASH_DATA_TYPE* hash);

#endif
