#ifndef RANDOM_LIB
#define RANDOM_LIB

#include "errorsHandler.hpp"

typedef uint64_t hash_data_type;

hash_data_type getRandomUint64tNumber();
Errors addNumToHash(hash_data_type* hash, const hash_data_type number);
Errors fillSequenceOfBytesWithRandomValues(void* elementVoidPtr, size_t memToFill);
Errors getHashOfSequenceOfBytes(const void* elementVoidPtr, size_t sizeOfSeqMemory, hash_data_type* hash);
Errors getHashOfStruct(const void* structObjVoidPtr, size_t sizeOfStruct, size_t sizeOfHash,
                       size_t bytesBeforeHash, hash_data_type* hash);

#define GET_HASH_OF_STRUCT(structObj, hashFieldName, hash)                                                  \
do {                                                                                                        \
    IF_ARG_NULL_RETURN(structObj);                                                                          \
    size_t sizeOfStruct    = sizeof(*structObj);                                                            \
    size_t sizeOfHash      = sizeof(structObj->hashFieldName);                                              \
    size_t bytesBeforeHash = (size_t)((const uint8_t*)(&structObj->hashFieldName) - (const uint8_t*)structObj);   \
    Errors error           = getHashOfStruct(structObj, sizeOfStruct, sizeOfHash, bytesBeforeHash, hash);   \
    IF_ERR_RETURN(error);                                                                                   \
} while (0);

#endif
